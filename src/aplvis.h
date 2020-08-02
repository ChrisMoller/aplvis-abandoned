#ifndef APLVIS_H
#define APLVIS_H

typedef struct {
  GtkWidget       *axis_name;
#if 0
  GtkWidget       *axis_label;
#endif
  GtkAdjustment   *axis_min_adj;
  GtkAdjustment   *axis_max_adj;
} indep_s;

extern indep_s indep_x;
extern indep_s indep_y;

extern GtkAdjustment   *gran_adj;
extern GtkWidget       *title;
extern GtkWidget       *gran_spin;
extern GtkWidget       *expression;
extern GtkWidget       *window;
extern gboolean         loading;

void expression_activate_cb (GtkEntry *entry, gpointer  user_data);


#endif  // APLVIS_H
