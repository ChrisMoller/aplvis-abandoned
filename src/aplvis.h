#ifndef APLVIS_H
#define APLVIS_H

typedef struct {
  GtkWidget       *axis_name;
  GtkWidget       *axis_label;
  GtkAdjustment   *axis_min_adj;
  GtkAdjustment   *axis_max_adj;
} indep_s;

extern indep_s indep_x;
extern indep_s indep_y;

extern GtkWidget       *title;
extern GtkWidget       *expression;
extern GtkWidget       *window;


#endif  // APLVIS_H
