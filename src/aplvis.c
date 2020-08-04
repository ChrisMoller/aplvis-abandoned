/*
    This file is part of GNU APL, a free implementation of the
    ISO/IEC Standard 13751, "Programming Language APL, Extended"

    Copyright (C) 2020 Chris Moller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>

#include <alloca.h>
#include <malloc.h>
#include <math.h>
#include <values.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <plplot.h>
#include <cairo/cairo-pdf.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-svg.h>
#include <apl/libapl.h>

#include "aplvis.h"
#include "save.h"
#include "markup.h"
#include "render.h"

#define DEFAULT_WIDTH  480
#define DEFAULT_HEIGHT 320

static GtkWidget       *status;
static GFileMonitor    *gfm;
static gulong           gsc;
static char            *newfn;
static FILE            *newout;
static int              newfd;
static gboolean		menu_ready = FALSE;

gboolean		loading = FALSE;

//                                     labels
indep_s indep_x = {NULL, NULL, NULL /*, NULL*/};
indep_s indep_y = {NULL, NULL, NULL /*, NULL*/};
GtkAdjustment *gran_adj = NULL;
GtkWidget *gran_spin    = NULL;
gint             width           = DEFAULT_WIDTH;
gint             height          = DEFAULT_HEIGHT;

GtkWidget       *window          = NULL;
GtkWidget       *title;
GtkWidget       *expression;

#define DEFAULT_GRANULARITY	60
gint granularity = DEFAULT_GRANULARITY;
PLFLT *xvec = NULL;
PLFLT *yvec = NULL;
PLFLT xmin = 0.0;
PLFLT xmax = 0.0;
PLFLT ymin = 0.0;
PLFLT ymax = 0.0;
int rank =  0;
uint64_t count = 0;
GtkWidget       *da              = NULL;

//#define expvar "expvar⍙"
#define expvar "expvarλ"

static void
set_indep (indep_s *indep)
{
  glong items_read;
  glong items_written;
  const gchar *x_name = gtk_entry_get_text (GTK_ENTRY (indep->axis_name));
  if (x_name && *x_name) {
    gdouble x_adj_min =
      gtk_adjustment_get_value (GTK_ADJUSTMENT (indep->axis_min_adj));
    gdouble x_adj_max =
      gtk_adjustment_get_value (GTK_ADJUSTMENT (indep->axis_max_adj));
    if (x_adj_min ==  x_adj_max) {
      // fixme dump status
      return;
    }

    //fixme temporary

    granularity =
      gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (gran_spin));
    
    gdouble k_incr = (x_adj_max - x_adj_min) / (double)granularity;
    
#define INCR_FMT "%s ← (0%c%g) %c %g×⍳(%d)"
    char *x_incr =
      g_strdup_printf (INCR_FMT,
		       x_name,
		       ((x_adj_min < 0.0) ? '-' : '+'),
		       fabs (x_adj_min),
		       ((k_incr < 0.0) ? '-' : '+'),
		       fabs (k_incr),
		       granularity);

    gunichar *x_incr_ucs = g_utf8_to_ucs4 (x_incr,
					   (glong)strlen (x_incr),
					   &items_read,
					   &items_written,
					   NULL);
    g_free (x_incr);
    apl_exec_ucs (x_incr_ucs);
    g_free (x_incr_ucs);
  }
}

void
expression_activate_cb (GtkEntry *entry,
			gpointer  user_data)
{
  if (!menu_ready || loading) return;
  const gchar *expr = gtk_entry_get_text (GTK_ENTRY (expression));
  if (!expr || !*expr) return;
  set_indep (&indep_x);
  set_indep (&indep_y);
  char *cmd = g_strdup_printf ("%s←%s", expvar, expr);

  glong items_read;
  glong items_written;
  gunichar *cmd_ucs = g_utf8_to_ucs4 (cmd,
				      (glong)strlen (cmd),
				      &items_read,
				      &items_written,
				      NULL);
  g_free (cmd);
  apl_exec_ucs (cmd_ucs);
  g_free (cmd_ucs);
  //  g_mutex_trylock (&mutex);
  APL_value expval = get_var_value(expvar, "something");
  //  g_mutex_unlock (&mutex);
  if (expval == NULL) {
    //    gtk_label_set_text (GTK_LABEL (status),
    //			_ ("Null return from expression evaluation"));
    return;
  }
  gtk_label_set_text (GTK_LABEL (status), "");
  count = get_element_count (expval);
  int i;
  for (i = 0; i < count; i++) {
#if 1
    if (!(get_type(expval, i) & CCT_NUMERIC)) break;
#else
    if (!is_numeric (expval, i)) break;
#endif
  }
  if (i < count) {
    gtk_label_set_text (GTK_LABEL (status),
			_ ("Non-numeric result in expression"));
    return;
  }
  
  rank =  get_rank (expval);
  if (xvec) free (xvec);
  if (yvec) free (yvec);
  xmax = ymax = -MAXDOUBLE;
  xmin = ymin =  MAXDOUBLE;
  if (count > 1) {
    if (rank == 0) {
      gtk_label_set_text (GTK_LABEL (status),
			  _("Sorry, but it's hard to plot a point..."));
      return;
    }
    if (rank == 1) {
      int i;
      APL_value quad_io = get_var_value("⎕io", "something");
      int qio = get_int (quad_io, 0);
      xvec = malloc (count * sizeof(PLFLT));
      yvec = malloc (count * sizeof(PLFLT));
      for (i = 0; i < count; i++) {
	switch(get_type (expval, i)) {
	case CCT_INT:	
	  yvec[i] = ((PLFLT)get_int (expval, i));
	  break;
	case CCT_FLOAT:
	  yvec[i] = (PLFLT)get_real (expval, i);
	  break;
	}
	xvec[i] = ((PLFLT)(qio + i));
	if (xmax < xvec[i]) xmax = xvec[i];
	if (xmin > xvec[i]) xmin = xvec[i];
	if (ymax < yvec[i]) ymax = yvec[i];
	if (ymin > yvec[i]) ymin = yvec[i];
      }
    }
    else if (rank == 2) plot_rank_2 (expval);

    guint width = gtk_widget_get_allocated_width (da);
    guint height = gtk_widget_get_allocated_height (da);
    gtk_widget_queue_draw_area (da, 0, 0, width, height);
  }
  else gtk_label_set_text (GTK_LABEL (status), _ ("No data to plot."));
}

static void
go_button_cb (GtkButton *button,
	      gpointer   user_data)
{
  expression_activate_cb (NULL, NULL);
}

static void
aplvis_quit (GtkWidget *object, gpointer data)
{
  unlink (newfn);
  gtk_main_quit ();
}

static gboolean
da_key_cb (GtkWidget *widget,
           GdkEvent  *event,
           gpointer   user_data)
{
  GdkEventAny *any = (GdkEventAny *)event;
  if (any->type == GDK_KEY_PRESS) {
    GdkEventKey *key = (GdkEventKey *)event;
    if (key->keyval == GDK_KEY_q) aplvis_quit (NULL, NULL);
  }
  return GDK_EVENT_PROPAGATE;
}

static gboolean
da_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  PLPointer pbuf = NULL;
  cairo_surface_t *surface = NULL;
  guint width = gtk_widget_get_allocated_width (widget);
  guint height = gtk_widget_get_allocated_height (widget);
  pbuf = g_malloc0 (4 * width * height);
  if (pbuf) {
    plsdev ("memcairo");
    plsmema ((PLINT)width, (PLINT)height, pbuf);
    surface =
      cairo_image_surface_create_for_data (pbuf,
					   CAIRO_FORMAT_ARGB32,
					   (PLINT)width,
					   (PLINT)height,
					   4 * (PLINT)width);
    plinit();

    /* set bg */
    cairo_rectangle (cr, 0.0, 0.0, (PLFLT)width, (PLFLT)height);
    cairo_set_source_rgba (cr,
			   bg_colour.red,
			   bg_colour.green,
			   bg_colour.blue,
			   bg_colour.alpha);
    cairo_fill (cr);
    
    if (xvec) {
      plenv (xmin, xmax, ymin, ymax, 0, 0);
      pllab (x_label, y_label, gtk_entry_get_text (GTK_ENTRY (title)));
      plssym (0.0, 0.75);
      plcol0 (3);
      plline (count, xvec, yvec);
    }
  
    plend ();
  
    cairo_set_source_surface (cr, surface, 0, 0);

    cairo_paint (cr);
    g_free (pbuf);
  }
  else {		// fixme error
  }

  return GDK_EVENT_STOP;	
}


static void
build_menu (GtkWidget *vbox)
{
  GtkWidget *menubar;
  GtkWidget *menu;
  GtkWidget *item;

  menubar = gtk_menu_bar_new();

  /********* file menu ********/

  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_label (_ ("File"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);

  item = gtk_menu_item_new_with_label (_ ("Save..."));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (save_dialogue), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_label (_ ("Load..."));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (load_dialogue), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_separator_menu_item_new();
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  item = gtk_menu_item_new_with_label (_ ("Quit"));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (aplvis_quit), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

  /********* edit ********/

  menu = gtk_menu_new();
  item = gtk_menu_item_new_with_label (_ ("Preferences"));
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), menu);
  gtk_menu_shell_append (GTK_MENU_SHELL (menubar), item);

  item = gtk_menu_item_new_with_label (_ ("Markup"));
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (markup_dialogue), NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);


  /********* end of menus ********/

  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (menubar), FALSE, FALSE, 2);
}

//static GMutex mutex;

static void
monitor_changed (GFileMonitor      *monitor,
                 GFile             *file,
                 GFile             *other_file,
                 GFileMonitorEvent  event_type,
                 gpointer           user_data)
{
  static off_t offset = 0;
  g_signal_handler_block (gfm, gsc);
  struct stat statbuf;
  fstat (fileno (newout), &statbuf);
  off_t size = statbuf.st_size;
  if (size > offset) {
    gchar *bfr = g_malloc0 (16 + (size_t)(size-offset));
    int fd = open (newfn, O_RDONLY);
    lseek (fd, offset, SEEK_SET);
    read (fd, bfr, (size_t)size);
    char *p = bfr;
    for (; *p; p++) if (*p == '\n' || *p == '^') *p = ' ';
    char *msg = g_strdup_printf ("APL error: %s\n", bfr);
    gtk_label_set_text (GTK_LABEL (status), msg);
    while (*--p == ' ') *p = 0;
    g_free (bfr);
    g_free (msg);
    close (fd);
    offset = size;
  }
  
  g_signal_handler_unblock (gfm, gsc);
}

static void
spin_changed_cb (GtkSpinButton *spin_button,
		 gpointer       user_data)
{
  expression_activate_cb (NULL, NULL);
}

static gboolean
spin_output_cb (GtkSpinButton *spin_button,
	 gpointer       user_data)
{
  gboolean frc = FALSE;
#define PATTERN0 "-?pi$"
#define PATTERN1 "(-?[0-9]*(\\.[0-9]*)?)pi$"
#define PATTERN2 "(-?)pi/([0-9]*(\\.[0-9]*)?)$"
  
  static GRegex *fmt0 = NULL;
  static GRegex *fmt1 = NULL;
  static GRegex *fmt2 = NULL;

  if (!fmt0) 
    fmt0 = g_regex_new (PATTERN0,
			G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_OPTIMIZE,
			G_REGEX_MATCH_ANCHORED, NULL);
  if (!fmt1) 
    fmt1 = g_regex_new (PATTERN1,
			G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_OPTIMIZE,
			G_REGEX_MATCH_ANCHORED, NULL);
  if (!fmt2)
    fmt2 = g_regex_new (PATTERN2,
			G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_OPTIMIZE,
			G_REGEX_MATCH_ANCHORED, NULL);
  
  const gchar *str = gtk_entry_get_text (GTK_ENTRY (spin_button));

  gboolean rc;
  GMatchInfo *match_info;
  gdouble res = NAN;

  rc = g_regex_match (fmt0, str, 0, &match_info);
  if (rc) res = ('-' == *g_match_info_fetch (match_info, 0 )) ? -G_PI : G_PI;
  if (isnan (res)) {
    rc = g_regex_match (fmt1, str, 0, &match_info);
    if (rc) {
      res = G_PI * g_strtod (g_match_info_fetch (match_info, 1), NULL);
      g_match_info_free (match_info);
    }
  }
  if (isnan (res)) {
    rc = g_regex_match (fmt2, str, 0, &match_info);
    if (rc) {
      gdouble val = g_strtod (g_match_info_fetch (match_info, 2), NULL);
      if (val != 0.0) {
	res = G_PI/val;
	if ('-' == (*g_match_info_fetch (match_info, 1)))
	  res = -res;
      }
      g_match_info_free (match_info);
    }
  }
  if (!isnan (res)) {
#define RESSTR_SIZE	64
    gchar resstr[RESSTR_SIZE];
    g_snprintf (resstr, RESSTR_SIZE, "%g", res);
    gtk_entry_set_text (GTK_ENTRY (spin_button), resstr);
    frc = TRUE;
  }
  
  return frc;
}

static void
create_limit_spin (GtkWidget *grid, indep_s *indep,
		   gint *row, gint *col, const gchar *name)
{
  indep->axis_name = gtk_entry_new ();
  gtk_grid_attach (GTK_GRID (grid), indep->axis_name, (*col)++, *row, 1, 1);
  gtk_entry_set_max_length (GTK_ENTRY (indep->axis_name), 6);
  gtk_entry_set_placeholder_text (GTK_ENTRY (indep->axis_name), name);

  indep->axis_min_adj =
    gtk_adjustment_new (-1.0, 
			-MAXDOUBLE,
			MAXDOUBLE,
			0.1,		// gdouble step_increment,
			0.5,		// gdouble page_increment,
			0.5);		// gdouble page_size);
  GtkWidget *axis_x_min = gtk_spin_button_new (indep->axis_min_adj, 0.1, 4);
  g_signal_connect (axis_x_min, "output",
                    G_CALLBACK (spin_output_cb), NULL);
  g_signal_connect (axis_x_min, "value-changed",
                    G_CALLBACK (spin_changed_cb), NULL);
  gtk_grid_attach (GTK_GRID (grid), axis_x_min, (*col)++, *row, 1, 1);
  indep->axis_max_adj =
    gtk_adjustment_new (1.0, 
			-MAXDOUBLE,
			MAXDOUBLE,
			0.1,		// gdouble step_increment,
			0.5,		// gdouble page_increment,
			0.5);		// gdouble page_size);
  GtkWidget *axis_x_max = gtk_spin_button_new (indep->axis_max_adj, 0.1, 4);
  g_signal_connect (axis_x_max, "output",
                    G_CALLBACK (spin_output_cb), NULL);
  g_signal_connect (axis_x_max, "value-changed",
                    G_CALLBACK (spin_changed_cb), NULL);
  gtk_grid_attach (GTK_GRID (grid), axis_x_max, (*col)++, *row, 1, 1);
}

#if 0
#if 0
static const gchar *css_data =
"* {\n"
"  font-size: small;\n"
"}\n"
"#SpinFrame {\n"
"  background-color:green;\n"
"  margin-top:    -2px;\n"
"  margin-bottom: -2px;\n"
"}\n"
  ;
#else
const gchar *css_data =
"* #button {\n"
"  background-color:rgba (192, 0, 0, 1);\n"
"}\n"
  ;
#endif

static const gchar *css_data =" .mycolourchooser {width: 16px;}";
#endif


#if 0
static void
set_up_css ()
{
  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_data (provider,
                                   css_data,
                                   -1,
                                   NULL);
  GdkDisplay *display = gdk_display_get_default ();
  GdkScreen *screen   = gdk_display_get_default_screen (display);
  gtk_style_context_add_provider_for_screen (screen,
            GTK_STYLE_PROVIDER (provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);
}
#endif

static void
sigint_handler (int sig, siginfo_t *si, void *data)
{
  unlink (newfn);
}

int
main (int ac, char *av[])
{
  struct sigaction action;
  action.sa_sigaction = sigint_handler;
  sigemptyset (&action.sa_mask);
  sigaction (SIGINT, &action, NULL);
  sigaction (SIGQUIT, &action, NULL);
  sigaction (SIGTERM, &action, NULL);
  
  asprintf (&newfn, "/tmp/aplvis-%d-%d.log", (int)getuid (), (int)getpid ());
  newfd = memfd_create (newfn, 0);
  newout = freopen(newfn, "a+", stdout);
  stdout = newout;

  GFile *gf = g_file_new_for_path (newfn);
  gfm = g_file_monitor_file (gf,
			     G_FILE_MONITOR_NONE, // GFileMonitorFlags flags,
			     NULL,		// GCancellable *cancellable,
			     NULL);		// GError **error);
  g_file_monitor_set_rate_limit (gfm, 200);
  gsc = g_signal_connect (G_OBJECT(gfm), "changed",
			  G_CALLBACK (monitor_changed), NULL);
  
  GOptionEntry entries[] =
    {
#if 0
     { "setvar", 'v', 0, G_OPTION_ARG_STRING_ARRAY,
       &vars, "Set variable.", NULL },
#endif
     { NULL }
  };

  GOptionContext *context = g_option_context_new ("string string string...");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group (context, gtk_get_option_group (TRUE));

  init_libapl (av[0], 0);
  
  gtk_init (&ac, &av);
#if 0
  set_up_css ();
#endif

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (window, "key-press-event",
                    G_CALLBACK (da_key_cb), NULL);
  g_signal_connect (window, "destroy", G_CALLBACK (aplvis_quit), NULL);
  gtk_window_set_title (GTK_WINDOW (window), _ ("Visualiser"));

  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
  gtk_container_add (GTK_CONTAINER (window), vbox);

  build_menu (vbox);


  /******** center grid ********/
  
  GtkWidget *grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (grid), TRUE, TRUE, 2);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), FALSE);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), FALSE);


  /************* title *************/

  gint row = 0;
  gint col = 0;

  title = gtk_entry_new ();
  g_signal_connect (title, "activate",
                    G_CALLBACK (expression_activate_cb), NULL);
  gtk_grid_attach (GTK_GRID (grid), title, col, row, 2, 1);
  gtk_entry_set_placeholder_text (GTK_ENTRY (title),  _ ("Title"));

  col += 2;
  
  gran_adj = gtk_adjustment_new ((gdouble)granularity, 
				 -10.0,
				 1024,
				 1.0,	// gdouble step_increment,
				 5.0,	// gdouble page_increment,
				 10.0);	// gdouble page_size);
  gran_spin = gtk_spin_button_new (gran_adj, 1, 4);
  g_signal_connect (gran_spin, "value-changed",
                    G_CALLBACK (spin_changed_cb), NULL);
  gtk_grid_attach (GTK_GRID (grid), gran_spin, col++, row, 1, 1);

  /******* x axis ******/
  
  row += 2;
  col = 0;

  create_limit_spin (grid, &indep_x, &row, &col, _ ("X Name"));
  row += 1;
  col = 0;
  create_limit_spin (grid, &indep_y, &row, &col, _ ("Y Name"));
  row += 1;

  /******* expression *******/


  expression = gtk_entry_new ();
  g_signal_connect (expression, "activate",
                    G_CALLBACK (expression_activate_cb), NULL);
  gtk_entry_set_placeholder_text (GTK_ENTRY (expression),
				  _ ("APL expression"));
  gtk_grid_attach (GTK_GRID (grid), expression, 0, row, col, 1);

  /********** status bar ******/

  row++;

  status = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), status, 0, row, col-1, 1);

  /************* go button **********/

  GtkWidget *go_button = gtk_button_new_with_label (_ ("Go"));
  gtk_grid_attach (GTK_GRID (grid), go_button, col-1, row, 1, 1);
  g_signal_connect (go_button, "clicked",
                    G_CALLBACK (go_button_cb), NULL);
  
  /***** drawing area ****/

  row = 1;
  
  da = gtk_drawing_area_new ();
  gtk_widget_set_size_request (da, width, height);
  g_signal_connect (da, "draw",
                    G_CALLBACK (da_draw_cb), NULL);
  gtk_widget_set_hexpand (da, TRUE);
  gtk_widget_set_vexpand (da, TRUE);
  gtk_grid_attach (GTK_GRID (grid), da, 0, row, col, 1);

  /******* end grid ******/

  menu_ready = TRUE;

  if (ac > 1) {
    load_file (av[1]);
    expression_activate_cb (NULL, NULL);
  }

  gtk_widget_show_all (window);
  gtk_main ();

}
