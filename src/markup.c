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

gchar *x_label = NULL;
gchar *y_label = NULL;
gchar *z_label = NULL;

#define DEFAULT_BG_RED		0.3
#define DEFAULT_BG_GREEN	0.5
#define DEFAULT_BG_BLUE		0.7
#define DEFAULT_BG_ALPHA	1.0

GdkRGBA bg_colour = {DEFAULT_BG_RED, DEFAULT_BG_GREEN,
		     DEFAULT_BG_BLUE, DEFAULT_BG_ALPHA};

void
markup_dialogue (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialogue =
    gtk_dialog_new_with_buttons (_ ("Appearance and Background Colour"),
				 GTK_WINDOW (window),
				 GTK_DIALOG_DESTROY_WITH_PARENT,
                                 "_OK", GTK_RESPONSE_ACCEPT,
                                 "_Cancel", GTK_RESPONSE_CANCEL,
                                 NULL);
  gtk_window_set_position (GTK_WINDOW (dialogue), GTK_WIN_POS_MOUSE);
  gtk_dialog_set_default_response (GTK_DIALOG (dialogue),
                                   GTK_RESPONSE_ACCEPT);

  GtkWidget *outer_vbox = gtk_dialog_get_content_area (GTK_DIALOG (dialogue));
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_pack_start (GTK_BOX (outer_vbox), GTK_WIDGET (hbox), TRUE, TRUE, 8);

  GtkWidget *grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (grid), TRUE, TRUE, 8);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 8);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 8);
  gtk_grid_set_row_homogeneous (GTK_GRID (grid), FALSE);
  gtk_grid_set_column_homogeneous (GTK_GRID (grid), FALSE);

  gint row = 0;
  gint col = 0;
  
  GtkWidget *x_lbl_label = gtk_label_new (_ ("X Label"));
  gtk_grid_attach (GTK_GRID (grid), x_lbl_label, col++, row, 1, 1);
  GtkWidget *x_lbl_entry = gtk_entry_new ();
  if (x_label) gtk_entry_set_text (GTK_ENTRY (x_lbl_entry), x_label);
  gtk_grid_attach (GTK_GRID (grid), x_lbl_entry, col++, row, 1, 1);
  gtk_entry_set_placeholder_text (GTK_ENTRY (x_lbl_entry),  _ ("X Label"));

  row++;
  col = 0;
  
  GtkWidget *y_lbl_label = gtk_label_new (_ ("Y Label"));
  gtk_grid_attach (GTK_GRID (grid), y_lbl_label, col++, row, 1, 1);
  GtkWidget *y_lbl_entry = gtk_entry_new ();
  if (y_label) gtk_entry_set_text (GTK_ENTRY (y_lbl_entry), y_label);
  gtk_grid_attach (GTK_GRID (grid), y_lbl_entry, col++, row, 1, 1);
  gtk_entry_set_placeholder_text (GTK_ENTRY (y_lbl_entry),  _ ("Y Label"));

  row++;
  col = 0;
  
  GtkWidget *z_lbl_label = gtk_label_new (_ ("Z Label"));
  gtk_grid_attach (GTK_GRID (grid), z_lbl_label, col++, row, 1, 1);
  GtkWidget *z_lbl_entry = gtk_entry_new ();
  if (z_label) gtk_entry_set_text (GTK_ENTRY (z_lbl_entry), z_label);
  gtk_grid_attach (GTK_GRID (grid), z_lbl_entry, col++, row, 1, 1);
  gtk_entry_set_placeholder_text (GTK_ENTRY (z_lbl_entry),  _ ("Z Label"));

  GtkWidget *colour_chooser =
    gtk_color_chooser_widget_new ();
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (colour_chooser),
		      TRUE, TRUE, 8);


  gtk_widget_show_all (dialogue);
  gint response = gtk_dialog_run (GTK_DIALOG (dialogue));
  
  if (response == GTK_RESPONSE_ACCEPT) {
    if (x_label) g_free (x_label);
    x_label = g_strdup (gtk_entry_get_text (GTK_ENTRY (x_lbl_entry)));
    
    if (y_label) g_free (y_label);
    y_label = g_strdup (gtk_entry_get_text (GTK_ENTRY (y_lbl_entry)));
    
    if (z_label) g_free (z_label);
    z_label = g_strdup (gtk_entry_get_text (GTK_ENTRY (z_lbl_entry)));

    gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (colour_chooser),
				&bg_colour);
    expression_activate_cb (NULL, NULL);
  }
  gtk_widget_destroy (dialogue);
}
