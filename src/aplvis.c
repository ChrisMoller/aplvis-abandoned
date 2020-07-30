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



#define DEFAULT_WIDTH  480
#define DEFAULT_HEIGHT 320

static gint             width           = DEFAULT_WIDTH;
static gint             height          = DEFAULT_HEIGHT;
static GtkWidget       *window          = NULL;
static GtkWidget       *da              = NULL;
static GtkWidget       *axis_x_name;
static GtkWidget       *axis_x_label;
static GtkAdjustment   *axis_x_min_adj;
static GtkAdjustment   *axis_x_max_adj;
static GtkWidget       *axis_y_name;
static GtkWidget       *axis_y_label;
static GtkAdjustment   *axis_y_min_adj;
static GtkAdjustment   *axis_y_max_adj;
static GtkWidget       *expression;
static GtkWidget       *status;
static GFileMonitor    *gfm;
static gulong           gsc;
static char            *newfn;
static FILE            *newout;
static int              newfd;

#define DEFAULT_GRANULARITY	100
gint granularity = DEFAULT_GRANULARITY;
PLFLT *xvec= NULL;
PLFLT *yvec= NULL;
PLFLT xmin, xmax, ymin, ymax;
int rank =  0;
uint64_t count = 0;

#define expvar "expvar⍙"

static void
expression_activate_cb (GtkEntry *entry,
			gpointer  user_data)
{
  glong items_read;
  glong items_written;
  const gchar *expr = gtk_entry_get_text (GTK_ENTRY (entry));
  if (!expr || !*expr) return;

  const gchar *x_name = gtk_entry_get_text (GTK_ENTRY (axis_x_name));
  if (x_name && *x_name) {
    gdouble x_adj_min =
      gtk_adjustment_get_value (GTK_ADJUSTMENT (axis_x_min_adj));
    gdouble x_adj_max =
      gtk_adjustment_get_value (GTK_ADJUSTMENT (axis_x_max_adj));
    if (x_adj_min ==  x_adj_max) {
      // fixme dump status
      return;
    }
    gdouble k_incr = (x_adj_max - x_adj_min) / (double)granularity;
    /***
	1..4 ==> i 2 3 4 ==> 1 + 0 1 2 3 ==> 1 + ⍳(4 - 1)
	4..1 ==> 4 3 2 1 ==> 4 - 0 1 2 3 ==> 4 - ⍳(4 - 1)
	-4..-1 ==> -4 -3 -2 -1 ==> (0-4) + ⍳(4 - 1)
	-1..-4 ==> -1 -2 -3 -4 ==> (0-1) - ⍳(4 - 1)
     ***/
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
  
  char *cmd = g_strdup_printf ("%s←%s", expvar, expr);

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
    else if (rank == 2) {
      int i, j;
      //APL_value quad_io = get_var_value("⎕io", "something");
      //      int qio = get_int (quad_io, 0);
      count /= 2;
      xvec = malloc (count * sizeof(PLFLT));
      yvec = malloc (count * sizeof(PLFLT));
      for (i = 0, j = 0; j < count; i++, j++) {
	switch(get_type (expval, i)) {
	case CCT_INT:	
	  xvec[j] = ((PLFLT)get_int (expval, i));
	  break;
	case CCT_FLOAT:
	  xvec[j] = (PLFLT)get_real (expval, i);
	  break;
	}
	if (xmax < xvec[j]) xmax = xvec[j];
	if (xmin > xvec[j]) xmin = xvec[j];
      }
      for (j = 0; j < count; i++,j++) {
	switch(get_type (expval, i)) {
	case CCT_INT:	
	  yvec[j] = ((PLFLT)get_int (expval, i));
	  break;
	case CCT_FLOAT:
	  yvec[j] = (PLFLT)get_real (expval, i);
	  break;
	}
	if (ymax < yvec[j]) ymax = yvec[j];
	if (ymin > yvec[j]) ymin = yvec[j];
      }
    }

    guint width = gtk_widget_get_allocated_width (da);
    guint height = gtk_widget_get_allocated_height (da);
    gtk_widget_queue_draw_area (da, 0, 0, width, height);
  }
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
    cairo_set_source_rgba (cr, 1.0, 0.5, 0.7, 1.0);
    cairo_fill (cr);
    
    if (xvec) {
      plenv (xmin, xmax, ymin, ymax, 0, 0);
      pllab ("X", "Y", "Title");
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

  item = gtk_menu_item_new_with_label (_ ("Export..."));
#if 0
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (save_dialogue), NULL);
#endif
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

  item = gtk_menu_item_new_with_label (_ ("Environment"));
#if 0
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (preferences), NULL);
#endif
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


  gint row = 1;
  gint col = 0;


  /******* x axis ******/
  
  row += 1;
  col = 0;
  
  axis_x_name = gtk_entry_new ();
  gtk_grid_attach (GTK_GRID (grid), axis_x_name, col++, row, 1, 1);
  gtk_entry_set_max_length (GTK_ENTRY (axis_x_name), 6);
  gtk_entry_set_placeholder_text (GTK_ENTRY (axis_x_name),  _ ("X Name"));
  
  axis_x_label = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (axis_x_label), 16);
  gtk_entry_set_placeholder_text (GTK_ENTRY (axis_x_label),  _ ("X Label"));
  gtk_grid_attach (GTK_GRID (grid), axis_x_label, col++, row, 1, 1);
  axis_x_min_adj = gtk_adjustment_new (-1.0, 
				       -MAXDOUBLE,
				       MAXDOUBLE,
				       0.1,	// gdouble step_increment,
				       0.5,	// gdouble page_increment,
				       0.5);	// gdouble page_size);
  GtkWidget *axis_x_min = gtk_spin_button_new (axis_x_min_adj, 0.1, 4);
  gtk_grid_attach (GTK_GRID (grid), axis_x_min, col++, row, 1, 1);
  axis_x_max_adj = gtk_adjustment_new (1.0, 
				       -MAXDOUBLE,
				       MAXDOUBLE,
				       0.1,	// gdouble step_increment,
				       0.5,	// gdouble page_increment,
				       0.5);	// gdouble page_size);
  GtkWidget *axis_x_max = gtk_spin_button_new (axis_x_max_adj, 0.1, 4);
  gtk_grid_attach (GTK_GRID (grid), axis_x_max, col++, row, 1, 1);


  /******* y axis ******/
  
  row += 1;
  col = 0;
  
  axis_y_name = gtk_entry_new ();
  gtk_grid_attach (GTK_GRID (grid), axis_y_name, col++, row, 1, 1);
  gtk_entry_set_max_length (GTK_ENTRY (axis_y_name), 6);
  gtk_entry_set_placeholder_text (GTK_ENTRY (axis_y_name),  _ ("Y Name"));
  
  axis_y_label = gtk_entry_new ();
  gtk_entry_set_max_length (GTK_ENTRY (axis_y_label), 16);
  gtk_entry_set_placeholder_text (GTK_ENTRY (axis_y_label),  _ ("Y Label"));
  gtk_grid_attach (GTK_GRID (grid), axis_y_label, col++, row, 1, 1);
  axis_y_min_adj = gtk_adjustment_new (-1.0, 
				       -MAXDOUBLE,
				       MAXDOUBLE,
				       0.1,	// gdouble step_increment,
				       0.5,	// gdouble page_increment,
				       0.5);	// gdouble page_size);
  GtkWidget *axis_y_min = gtk_spin_button_new (axis_y_min_adj, 0.1, 4);
  gtk_grid_attach (GTK_GRID (grid), axis_y_min, col++, row, 1, 1);
  axis_y_max_adj = gtk_adjustment_new (1.0, 
				       -MAXDOUBLE,
				       MAXDOUBLE,
				       0.1,	// gdouble step_increment,
				       0.5,	// gdouble page_increment,
				       0.5);	// gdouble page_size);
  GtkWidget *axis_y_max = gtk_spin_button_new (axis_y_max_adj, 0.1, 4);
  gtk_grid_attach (GTK_GRID (grid), axis_y_max, col++, row, 1, 1);

  /******* expression *******/

  row += 1;

  expression = gtk_entry_new ();
  g_signal_connect (expression, "activate",
                    G_CALLBACK (expression_activate_cb), NULL);
  gtk_entry_set_placeholder_text (GTK_ENTRY (expression),
				  _ ("APL expression"));
  gtk_grid_attach (GTK_GRID (grid), expression, 0, row, col, 1);

  /********** status bar ******/

  row++;

  status = gtk_label_new ("");
  gtk_grid_attach (GTK_GRID (grid), status, 0, row, col, 1);

  /************* go button **********/

  GtkWidget *go_button = gtk_button_new_with_label (_ ("Go"));
  g_signal_connect (go_button, "clicked",
                    G_CALLBACK (go_button_cb), NULL);
  
  /***** drawing area ****/

  row = 0;
  
  da = gtk_drawing_area_new ();
  gtk_widget_set_size_request (da, width, height);
  g_signal_connect (da, "draw",
                    G_CALLBACK (da_draw_cb), NULL);
  gtk_widget_set_hexpand (da, TRUE);
  gtk_widget_set_vexpand (da, TRUE);
  gtk_grid_attach (GTK_GRID (grid), da, 0, row, col, 1);

  /******* end grid ******/

  gtk_widget_show_all (window);
  gtk_main ();

}
