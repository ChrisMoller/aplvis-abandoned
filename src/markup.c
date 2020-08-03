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
#include "markup.h"

gchar *x_label = NULL;
gchar *y_label = NULL;
gchar *z_label = NULL;

#define DEFAULT_BG_RED		0.3
#define DEFAULT_BG_GREEN	0.5
#define DEFAULT_BG_BLUE		0.7
#define DEFAULT_BG_ALPHA	1.0

GdkRGBA bg_colour = {DEFAULT_BG_RED, DEFAULT_BG_GREEN,
		     DEFAULT_BG_BLUE, DEFAULT_BG_ALPHA};

mode_e mode = MODE_2D;
coords_e coords = COORDS_CARTESIAN;
gint x_index = -1;
gint y_index = -1;

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
  GtkWidget *hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_pack_start (GTK_BOX (outer_vbox), GTK_WIDGET (hbox), TRUE, TRUE, 4);

  GtkWidget *grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (grid), TRUE, TRUE, 4);
  gtk_grid_set_row_spacing (GTK_GRID (grid), 4);
  gtk_grid_set_column_spacing (GTK_GRID (grid), 4);
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

  GtkWidget *colour_chooser = gtk_color_chooser_widget_new ();
  gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (colour_chooser), &bg_colour);
  gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (colour_chooser),
		      TRUE, TRUE, 4);
  
  GtkWidget *separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (outer_vbox), GTK_WIDGET (separator),
		      FALSE, FALSE, 8);
 
  GtkWidget *two_panes = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start (GTK_BOX (outer_vbox), GTK_WIDGET (two_panes),
		      TRUE, TRUE, 4);
  GtkWidget *left_frame = gtk_frame_new (_ ("2D Operations"));
  GtkWidget *right_frame = gtk_frame_new (_ ("3D Operations"));
  gtk_paned_pack1 (GTK_PANED (two_panes), left_frame, TRUE, TRUE);
  gtk_paned_pack2 (GTK_PANED (two_panes), right_frame, TRUE, TRUE);
  
  GtkWidget *left_vbox  = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget *right_vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add (GTK_CONTAINER (left_frame), left_vbox);
  gtk_container_add (GTK_CONTAINER (right_frame), right_vbox);


  GtkWidget *mode_2d_radio =
    gtk_radio_button_new_with_label (NULL, _ ("2D"));
  gtk_box_pack_start (GTK_BOX (left_vbox), GTK_WIDGET (mode_2d_radio),
		      FALSE, FALSE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mode_2d_radio),
				mode == MODE_2D);

  /******* left coordinate selection *******/
  
  GtkWidget *left_coord_frame = gtk_frame_new (_ ("2D Coordinate Systems"));
  gtk_box_pack_start (GTK_BOX (left_vbox), GTK_WIDGET (left_coord_frame),
		      FALSE, FALSE, 4);
  GtkWidget *left_coord_hbox  = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_add (GTK_CONTAINER (left_coord_frame),
		     left_coord_hbox);
  GtkWidget *left_cartesian_radio =
    gtk_radio_button_new_with_label (NULL, _ ("Cartesian"));
  gtk_box_pack_start (GTK_BOX (left_coord_hbox),
		      GTK_WIDGET (left_cartesian_radio), TRUE, TRUE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (left_cartesian_radio),
				coords == COORDS_CARTESIAN);
  GtkWidget *left_polar_radio =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (left_cartesian_radio), _ ("Polar"));
  gtk_box_pack_start (GTK_BOX (left_coord_hbox),
		      GTK_WIDGET (left_polar_radio), TRUE, TRUE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (left_polar_radio),
				coords == COORDS_POLAR);
  
  /******* end left coordinate selection *******/

  /******* left index selection *******/

  GtkWidget *left_grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (left_vbox), GTK_WIDGET (left_grid),
		      TRUE, TRUE, 4);
  gtk_grid_set_row_spacing (GTK_GRID (left_grid), 4);
  gtk_grid_set_column_spacing (GTK_GRID (left_grid), 4);
  gtk_grid_set_row_homogeneous (GTK_GRID (left_grid), FALSE);
  gtk_grid_set_column_homogeneous (GTK_GRID (left_grid), FALSE);

  GtkWidget *left_i_label = gtk_label_new (_ ("Index select*"));
  gtk_widget_set_tooltip_text (left_i_label,
			       _ ("Given a rank-2 matrix, sets which row is to be used as the x, or independent, axis while all other rows will be used as one or more y, or dependent, curves.  A value of -1 makes all the rows dependent, using a zero-based index for the independent axis."));
  gtk_grid_attach (GTK_GRID (left_grid), left_i_label, 0, 0, 1, 1);
  GtkWidget *left_i_select =
    gtk_spin_button_new_with_range (-1.0, 10000.0, 1.0);
  gtk_grid_attach (GTK_GRID (left_grid), left_i_select, 1, 0, 1, 1);
  
  /******* end left index selection *******/



  GtkWidget *mode_3d_radio =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (mode_2d_radio), _ ("3D"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mode_3d_radio),
				mode == MODE_3D);
  gtk_box_pack_start (GTK_BOX (right_vbox), GTK_WIDGET (mode_3d_radio),
		      FALSE, FALSE, 4);

  /******* right coordinate selection *******/
  
  GtkWidget *right_coord_frame = gtk_frame_new (_ ("3D Coordinate Systems"));
  gtk_box_pack_start (GTK_BOX (right_vbox), GTK_WIDGET (right_coord_frame),
		      FALSE, FALSE, 4);
  GtkWidget *right_coord_hbox  = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_container_add (GTK_CONTAINER (right_coord_frame),
		     right_coord_hbox);
  GtkWidget *right_cartesian_radio =
    gtk_radio_button_new_with_label (NULL, _ ("Cartesian"));
  gtk_box_pack_start (GTK_BOX (right_coord_hbox),
		      GTK_WIDGET (right_cartesian_radio), TRUE, TRUE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (right_cartesian_radio),
				coords == COORDS_CARTESIAN);
  GtkWidget *right_cylindrical_radio =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (right_cartesian_radio), _ ("Cylindrical"));
  gtk_box_pack_start (GTK_BOX (right_coord_hbox),
		      GTK_WIDGET (right_cylindrical_radio), TRUE, TRUE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (right_cylindrical_radio),
				coords == COORDS_CYLINDRICAL);
  GtkWidget *right_spherical_radio =
    gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (right_cartesian_radio), _ ("Spherical"));
  gtk_box_pack_start (GTK_BOX (right_coord_hbox),
		      GTK_WIDGET (right_spherical_radio), TRUE, TRUE, 4);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (right_spherical_radio),
				coords == COORDS_SPHERICAL);
  
  /******* end right coordinate selection *******/
  
  /******* right index selection *******/

  GtkWidget *right_grid = gtk_grid_new ();
  gtk_box_pack_start (GTK_BOX (right_vbox), GTK_WIDGET (right_grid),
		      TRUE, TRUE, 4);
  gtk_grid_set_row_spacing (GTK_GRID (right_grid), 4);
  gtk_grid_set_column_spacing (GTK_GRID (right_grid), 4);
  gtk_grid_set_row_homogeneous (GTK_GRID (right_grid), FALSE);
  gtk_grid_set_column_homogeneous (GTK_GRID (right_grid), FALSE);

  GtkWidget *right_xi_label = gtk_label_new (_ ("X index select*"));
  gtk_widget_set_tooltip_text (right_xi_label,
			       _ ("Given a rank-2 matrix, sets which rows are to be used as the x and, or independent, axes while all other rows will be used as one or more z, or values, curves.  A value of -1 means that axis will use a zero-based index instead a computed value."));
  gtk_grid_attach (GTK_GRID (right_grid), right_xi_label, 0, 0, 1, 1);
  GtkWidget *right_xi_select =
    gtk_spin_button_new_with_range (-1.0, 10000.0, 1.0);
  gtk_grid_attach (GTK_GRID (right_grid), right_xi_select, 1, 0, 1, 1);
  
  GtkWidget *right_yi_label = gtk_label_new (_ ("Y index select"));
  gtk_grid_attach (GTK_GRID (right_grid), right_yi_label, 0, 1, 1, 1);
  GtkWidget *right_yi_select =
    gtk_spin_button_new_with_range (-1.0, 10000.0, 1.0);
  gtk_grid_attach (GTK_GRID (right_grid), right_yi_select, 1, 1, 1, 1);

  /******* end index selection *******/
  

  
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

    mode = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (mode_2d_radio))
      ? MODE_2D : MODE_3D;

    if (mode == MODE_2D) {
      coords =
	gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (left_cartesian_radio))
	? COORDS_CARTESIAN : COORDS_POLAR;

      x_index =
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (left_i_select));
    }
    else {
      if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (right_cartesian_radio))) coords = COORDS_CARTESIAN;
      
      else 
	coords =
	  gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (right_cylindrical_radio))
	? COORDS_CYLINDRICAL : COORDS_SPHERICAL;

      x_index =
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (right_xi_select));

      y_index =
	gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (right_yi_select));
    }
  }
  gtk_widget_destroy (dialogue);
}
