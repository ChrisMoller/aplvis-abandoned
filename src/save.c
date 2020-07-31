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

#include "aplvis.h"
#include "xml-kwds.h"

static void
build_settings (FILE *ofile, const gchar*tag, GtkAdjustment *adj)
{
  gdouble adj_min_value =
    gtk_adjustment_get_value (GTK_ADJUSTMENT (adj));
  gdouble adj_min_lower =
    gtk_adjustment_get_lower (GTK_ADJUSTMENT (axis_x_max_adj));
  gdouble adj_min_upper =
    gtk_adjustment_get_upper (GTK_ADJUSTMENT (axis_x_max_adj));
  gdouble adj_min_page_size =
    gtk_adjustment_get_page_size (GTK_ADJUSTMENT (axis_x_max_adj));
  gdouble adj_min_page_increment =
    gtk_adjustment_get_page_increment (GTK_ADJUSTMENT (axis_x_max_adj));
  gdouble adj_min_step_increment =
    gtk_adjustment_get_step_increment (GTK_ADJUSTMENT (axis_x_max_adj));
  fprintf (ofile, "      <%s %s=\"%s\" %s=\"%g\" %s=\"%g\" %s=\"%g\" %s=\"%g\" %s=\"%g\"  %s=\"%g\"/>\n",
	   RANGE,
	   LIMIT, tag,
	   VALUE, adj_min_value,
	   LOWER, adj_min_lower,
	   UPPER, adj_min_upper,
	   PAGE_SIZE, adj_min_page_size,
	   PAGE_INCREMENT, adj_min_page_increment,
	   STEP_INCREMENT, adj_min_step_increment);
}

void
save_dialogue (GtkWidget *widget, gpointer data)
{
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (GTK_FILE_FILTER (filter), _ ("APL Vis files"));
  gtk_file_filter_add_pattern (filter, "*.vis");
  GtkWidget *dialog =
    gtk_file_chooser_dialog_new (_ ("Save vis file"),
                                 GTK_WINDOW (window),
                                 GTK_FILE_CHOOSER_ACTION_SAVE,
                                 "_OK", GTK_RESPONSE_ACCEPT,
                                 "_Cancel", GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                   GTK_RESPONSE_ACCEPT);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  gtk_widget_show_all (dialog);
  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  
  if (response == GTK_RESPONSE_ACCEPT) {
    gchar *file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    
    if (file) {
      FILE *ofile = fopen (file, "w");
      if (ofile) {
	fprintf (ofile, "<%s %s=\"%s\">\n", APLVIS, VIS_VERSION, "1.0.0");
	const gchar *titletxt  = gtk_entry_get_text (GTK_ENTRY (title));
	fprintf (ofile, "  <%1$s>%2$s</%1$s>\n", TITLE, titletxt);
	
	fprintf (ofile, "  <%s>\n", SETTINGS);
	
	const gchar *expr = gtk_entry_get_text (GTK_ENTRY (expression));
	fprintf (ofile, "    <%1$s>%2$s</%1$s>\n", EXPRESSION, expr);
	
	fprintf (ofile, "    <%s %s=\"%s\">\n", INDEPENDENT, AXIS, X_AXIS);
	const gchar *x_name  = gtk_entry_get_text (GTK_ENTRY (axis_x_name));
	const gchar *x_label = gtk_entry_get_text (GTK_ENTRY (axis_x_label));
	fprintf (ofile, "      <%1$s>%2$s</%1$s>\n", NAME, x_name);
	fprintf (ofile, "      <%1$s>%2$s</%1$s>\n", LABEL, x_label);
	build_settings (ofile, MINV, axis_x_min_adj);
	build_settings (ofile, MAXV, axis_x_max_adj);
	fprintf (ofile, "    </%s>\n", INDEPENDENT);
	
    
	fprintf (ofile, "  </%s>\n", SETTINGS);


	
	fprintf (ofile, "</%s\n", APLVIS);
	fflush (ofile);
	fclose (ofile);
      }
      g_free (file);
    }
  }
  gtk_widget_destroy (dialog);
}

static GHashTable *element_hash = NULL;
//static GQuark parse_quark = 0;

static gint
get_kwd (const gchar *str)
{
  return GPOINTER_TO_INT (g_hash_table_lookup (element_hash, str));
}

static void
name_text (GMarkupParseContext *context,
	   const gchar         *text,
	   gsize                text_len,
	   gpointer             user_data,
	   GError             **error)
{
  fprintf (stderr, "name = %s\n", text);
}

static void
label_text (GMarkupParseContext *context,
	    const gchar         *text,
	    gsize                text_len,
	    gpointer             user_data,
	    GError             **error)
{
  fprintf (stderr, "label = %s\n", text);
}

static const GMarkupParser name_parser =
  {
   NULL,				// start_element      
   NULL,				// parser.end_element 
   name_text,			// text               
   NULL,				// parser.passthrough 
   NULL				// error              
  };

static const GMarkupParser label_parser =
  {
   NULL,				// start_element      
   NULL,				// parser.end_element 
   label_text,			// text               
   NULL,				// parser.passthrough 
   NULL				// error              
  };

static void
expression_text (GMarkupParseContext *context,
		 const gchar         *text,
		 gsize                text_len,
		 gpointer             user_data,
		 GError             **error)
{
  fprintf (stderr, "expression = %s\n", text);
}

static const GMarkupParser expression_parser =
  {
   NULL,			// start_element      
   NULL,			// parser.end_element 
   expression_text,		// text               
   NULL,			// parser.passthrough 
   NULL				// error              
  };

static void
independent_start_element (GMarkupParseContext *context,
			   const gchar         *element_name,
			   const gchar        **attribute_names,
			   const gchar        **attribute_values,
			   gpointer             user_data,
			   GError             **error)
{
  switch(get_kwd (element_name)) {
  case KWD_NAME:
    g_markup_parse_context_push (context, &name_parser, NULL);
    break;
  case KWD_LABEL:
    g_markup_parse_context_push (context, &label_parser, NULL);
    break;
  case KWD_RANGE:
    {
      gchar *limit = NULL;
      gchar *value = NULL;
      gchar *lower = NULL;
      gchar *upper = NULL;
      gchar *page_size = NULL;
      gchar *page_increment = NULL;
      gchar *step_increment = NULL;
      g_markup_collect_attributes (element_name,
				   attribute_names,
				   attribute_values,
				   error,
				   G_MARKUP_COLLECT_STRING,
				   LIMIT, &limit,
				   G_MARKUP_COLLECT_STRING,
				   VALUE, &value,
				   G_MARKUP_COLLECT_STRING,
				   UPPER, &upper,
				   G_MARKUP_COLLECT_STRING,
				   LOWER, &lower,
				   G_MARKUP_COLLECT_STRING,
				   PAGE_SIZE, &page_size,
				   G_MARKUP_COLLECT_STRING,
				   PAGE_INCREMENT, &page_increment,
				   G_MARKUP_COLLECT_STRING,
				   STEP_INCREMENT, &step_increment,
				   G_MARKUP_COLLECT_INVALID);
      fprintf (stderr, "limit = %s\n", limit);
      fprintf (stderr, "value = %s\n", value);
      fprintf (stderr, "lower = %s\n", lower);
      fprintf (stderr, "upper = %s\n", upper);
      fprintf (stderr, "page_size = %s\n", page_size);
      fprintf (stderr, "page_increment = %s\n", page_increment);
      fprintf (stderr, "step_increment = %s\n", step_increment);
    }
    break;
  default:
    // fixme -- bad xml
    break;
  }
}


static void
independent_end_element (GMarkupParseContext *context,
			 const gchar *element_name,
			 gpointer      user_data,
			 GError      **error)
{
  switch(get_kwd (element_name)) {
  case KWD_NAME:
  case KWD_LABEL:
    //  case KWD_RANGE:
    g_markup_parse_context_pop (context);
    break;
  default:
    // fixme -- bad xml
    break;
  }
}

static const GMarkupParser independent_parser =
  {
   independent_start_element,	// start_element
   independent_end_element,	// parser.end_element
   NULL,			// text
   NULL,			// parser.passthrough
   NULL				// error              
  };

static void
settings_start_element (GMarkupParseContext *context,
			const gchar         *element_name,
			const gchar        **attribute_names,
			const gchar        **attribute_values,
			gpointer             user_data,
			GError             **error)
{
  switch(get_kwd (element_name)) {
  case KWD_EXPRESSION:
    g_markup_parse_context_push (context, &expression_parser, NULL);
    break;
  case KWD_INDEPENDENT:
    {
      gchar *axis = NULL;
      g_markup_collect_attributes (element_name,
				   attribute_names,
				   attribute_values,
				   error,
				   G_MARKUP_COLLECT_STRING,
				   AXIS, &axis,
				   G_MARKUP_COLLECT_INVALID);
      fprintf (stderr, "axis = %s\n", axis);
      g_markup_parse_context_push (context, &independent_parser, NULL);
    }
    break;
  default:
    // fixme -- bad xml
    break;
  }
}


static void
settings_end_element (GMarkupParseContext *context,
		     const gchar *element_name,
		     gpointer      user_data,
		     GError      **error)
{
  switch(get_kwd (element_name)) {
  case KWD_EXPRESSION:
  case KWD_INDEPENDENT:
    g_markup_parse_context_pop (context);
    break;
  default:
    // fixme -- bad xml
    break;
  }
}

static void
title_text (GMarkupParseContext *context,
	    const gchar         *text,
	    gsize                text_len,
	    gpointer             user_data,
	    GError             **error)
{
  fprintf (stderr, "title = %s\n", text);
}

static const GMarkupParser title_parser =
  {
   NULL,			// start_element      
   NULL,			// parser.end_element 
   title_text,			// text               
   NULL,			// parser.passthrough 
   NULL				// error              
  };

static const GMarkupParser settings_parser =
  {
   settings_start_element,	// start_element      
   settings_end_element,		// parser.end_element 
   NULL,				// text               
   NULL,				// parser.passthrough 
   NULL				// error              
  };

static void
aplvis_start_element (GMarkupParseContext *context,
		      const gchar         *element_name,
		      const gchar        **attribute_names,
		      const gchar        **attribute_values,
		      gpointer             user_data,
		      GError             **error)
{
  switch(get_kwd (element_name)) {
  case KWD_TITLE:
    g_markup_parse_context_push (context, &title_parser, NULL);
    break;
  case KWD_SETTINGS:
    g_markup_parse_context_push (context, &settings_parser, NULL);
    break;
  default:
    // fixme -- bad xml
    break;
  }
}


static void
aplvis_end_element (GMarkupParseContext *context,
		     const gchar *element_name,
		     gpointer      user_data,
		     GError      **error)
{
  switch(get_kwd (element_name)) {
  case KWD_SETTINGS:
  case KWD_TITLE:
    g_markup_parse_context_pop (context);
    break;
  default:
    // fixme -- bad xml
    break;
  }
}

static const GMarkupParser aplvis_parser =
  {
   aplvis_start_element,	// start_element
   aplvis_end_element,	// parser.end_element
   NULL,			// text
   NULL,			// parser.passthrough
   NULL			// error              
  };

static void
initial_start_element (GMarkupParseContext *context,
		       const gchar         *element_name,
		       const gchar        **attribute_names,
		       const gchar        **attribute_values,
		       gpointer             user_data,
		       GError             **error)
{
  switch(get_kwd (element_name)) {
  case KWD_APLVIS:
    {
      gchar *version = NULL;
      g_markup_collect_attributes (element_name,
				   attribute_names,
				   attribute_values,
				   error,
				   G_MARKUP_COLLECT_STRING,
				   VIS_VERSION, &version,
				   G_MARKUP_COLLECT_INVALID);
      fprintf (stderr, "version = %s\n", version);
      g_markup_parse_context_push (context, &aplvis_parser, NULL);
    }
    break;
  default:
    // fixme -- bad xml
    break;
  }
}


static void
initial_end_element (GMarkupParseContext *context,
		     const gchar *element_name,
		     gpointer      user_data,
		     GError      **error)
{
  switch(get_kwd (element_name)) {
  case KWD_APLVIS:
    g_markup_parse_context_pop (context);
    break;
  default:
    // fixme -- bad xml
    break;
  }
}



void
load_dialogue (GtkWidget *widget, gpointer data)
{
  GtkFileFilter *filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (GTK_FILE_FILTER (filter), _ ("APL Vis files"));
  gtk_file_filter_add_pattern (filter, "*.vis");
  GtkWidget *dialog =
    gtk_file_chooser_dialog_new (_ ("Load vis file"),
                                 GTK_WINDOW (window),
                                 GTK_FILE_CHOOSER_ACTION_OPEN,
                                 "_OK", GTK_RESPONSE_ACCEPT,
                                 "_Cancel", GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                   GTK_RESPONSE_ACCEPT);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
  gtk_widget_show_all (dialog);
  gint response = gtk_dialog_run (GTK_DIALOG (dialog));
  
  if (response == GTK_RESPONSE_ACCEPT) {
    gchar *file = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    if (file) {

      if (!element_hash) {
	element_hash = g_hash_table_new (g_str_hash,  g_str_equal);
	gint i;
	for (i = 0; i < nr_keys; i++)
	  g_hash_table_insert (element_hash, keywords[i].keyword,
			       GINT_TO_POINTER (keywords[i].keyvalue));
      }
      
      //      GError *error = NULL;
      GMarkupParser initial_parser =
	{
	 initial_start_element,
	 initial_end_element,
	 NULL,
	 NULL,
	 NULL
	};
	
      GMarkupParseContext *initial_context =
	g_markup_parse_context_new (&initial_parser,
				    G_MARKUP_TREAT_CDATA_AS_TEXT,
				    NULL,
				    NULL);
      gchar *contents = NULL;
      gsize length;
      gboolean rc = g_file_get_contents (file,
					 &contents,
					 &length,
					 NULL);
      if (rc) {
	GError *error = NULL;
	rc = g_markup_parse_context_parse (initial_context,
					   contents,
					   length,
					   &error);
	if (rc == 0) 
	  fprintf (stderr, "parsing error %s\n", error->message);
	if (contents) g_free (contents);
      }

      g_markup_parse_context_end_parse (initial_context,  NULL);
      g_markup_parse_context_free (initial_context);
    }
  }
  gtk_widget_destroy (dialog);
}
