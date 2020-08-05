#ifndef MARKUP_H
#define MARKUP_H


extern gchar *x_label;
extern gchar *y_label;
extern gchar *z_label;

extern GdkRGBA bg_colour;

typedef enum {MODE_2D, MODE_3D} mode_e;
extern mode_e mode;

typedef enum
  {
   COORDS_CARTESIAN,
   COORDS_POLAR,
   COORDS_CYLINDRICAL,
   COORDS_SPHERICAL
  } coords_e;
extern coords_e coords;

extern gint x_index;
extern gint y_index;

extern GdkRGBA base_colours[];
extern gint base_colour_count;
void markup_dialogue (GtkWidget *widget, gpointer data);
void init_colours ();
#endif // MARKUP_H
