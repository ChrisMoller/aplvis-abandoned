

/********* DON'T MODIFY THIS FILE! ********/
/**** Make all changes in xml-kwds.m4. ****/

#ifndef XML_KWDS_H
#define XML_KWDS_H

#define APLVIS "aplvis"
#define KWD_APLVIS 1000 


#define VIS_VERSION "vis_version"
#define KWD_VIS_VERSION 1001 


#define TITLE "title"
#define KWD_TITLE 1002 


#define SETTINGS "settings"
#define KWD_SETTINGS 1003 


#define EXPRESSION "expression"
#define KWD_EXPRESSION 1004 


#define INDEPENDENT "independent"
#define KWD_INDEPENDENT 1005 


#define AXIS "axis"
#define KWD_AXIS 1006 


#define X_AXIS "x_axis"
#define KWD_X_AXIS 1007 


#define Y_AXIS "y_axis"
#define KWD_Y_AXIS 1008 


#define NAME "name"
#define KWD_NAME 1009 


#define LABEL "label"
#define KWD_LABEL 1010 


#define RANGE "range"
#define KWD_RANGE 1011 


#define LIMIT "limit"
#define KWD_LIMIT 1012 


#define MINV "minv"
#define KWD_MINV 1013 


#define MAXV "maxv"
#define KWD_MAXV 1014 


#define VALUE "value"
#define KWD_VALUE 1015 


#define LOWER "lower"
#define KWD_LOWER 1016 


#define UPPER "upper"
#define KWD_UPPER 1017 


#define PAGE_SIZE "page_size"
#define KWD_PAGE_SIZE 1018 


#define PAGE_INCREMENT "page_increment"
#define KWD_PAGE_INCREMENT 1019 


#define STEP_INCREMENT "step_increment"
#define KWD_STEP_INCREMENT 1020 




typedef struct {
  gchar *keyword;
  gint   keyvalue;
} keyword_s;

keyword_s keywords[] = {
  { APLVIS, KWD_APLVIS },
  { VIS_VERSION, KWD_VIS_VERSION },
  { TITLE, KWD_TITLE },
  { SETTINGS, KWD_SETTINGS },
  { EXPRESSION, KWD_EXPRESSION },
  { INDEPENDENT, KWD_INDEPENDENT },
  { AXIS, KWD_AXIS },
  { X_AXIS, KWD_X_AXIS },
  { Y_AXIS, KWD_Y_AXIS },
  { NAME, KWD_NAME },
  { LABEL, KWD_LABEL },
  { RANGE, KWD_RANGE },
  { LIMIT, KWD_LIMIT },
  { MINV, KWD_MINV },
  { MAXV, KWD_MAXV },
  { VALUE, KWD_VALUE },
  { LOWER, KWD_LOWER },
  { UPPER, KWD_UPPER },
  { PAGE_SIZE, KWD_PAGE_SIZE },
  { PAGE_INCREMENT, KWD_PAGE_INCREMENT },
  { STEP_INCREMENT, KWD_STEP_INCREMENT },

};
gint nr_keys = 21;
#endif /* XML_KWDS_H*/
