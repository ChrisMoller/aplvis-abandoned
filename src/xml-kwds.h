

/********* DON'T MODIFY THIS FILE! ********/
/**** Make all changes in xml-kwds.m4. ****/

#ifndef XML_KWDS_H
#define XML_KWDS_H

#define KEYWORD_APLVIS "aplvis"
#define KWD_APLVIS 1000 


#define KEYWORD_VIS_VERSION "vis_version"
#define KWD_VIS_VERSION 1001 


#define KEYWORD_TITLE "title"
#define KWD_TITLE 1002 


#define KEYWORD_SETTINGS "settings"
#define KWD_SETTINGS 1003 


#define KEYWORD_EXPRESSION "expression"
#define KWD_EXPRESSION 1004 


#define KEYWORD_INDEPENDENT "independent"
#define KWD_INDEPENDENT 1005 


#define KEYWORD_AXIS "axis"
#define KWD_AXIS 1006 


#define KEYWORD_X_AXIS "x_axis"
#define KWD_X_AXIS 1007 


#define KEYWORD_Y_AXIS "y_axis"
#define KWD_Y_AXIS 1008 


#define KEYWORD_NAME "name"
#define KWD_NAME 1009 


#define KEYWORD_LABEL "label"
#define KWD_LABEL 1010 


#define KEYWORD_RANGE "range"
#define KWD_RANGE 1011 


#define KEYWORD_LIMIT "limit"
#define KWD_LIMIT 1012 


#define KEYWORD_MINV "minv"
#define KWD_MINV 1013 


#define KEYWORD_MAXV "maxv"
#define KWD_MAXV 1014 


#define KEYWORD_VALUE "value"
#define KWD_VALUE 1015 


#define KEYWORD_LOWER "lower"
#define KWD_LOWER 1016 


#define KEYWORD_UPPER "upper"
#define KWD_UPPER 1017 


#define KEYWORD_PAGE_SIZE "page_size"
#define KWD_PAGE_SIZE 1018 


#define KEYWORD_PAGE_INCREMENT "page_increment"
#define KWD_PAGE_INCREMENT 1019 


#define KEYWORD_STEP_INCREMENT "step_increment"
#define KWD_STEP_INCREMENT 1020 


#define KEYWORD_BG_RED "bg_red"
#define KWD_BG_RED 1021 


#define KEYWORD_BG_GREEN "bg_green"
#define KWD_BG_GREEN 1022 


#define KEYWORD_BG_BLUE "bg_blue"
#define KWD_BG_BLUE 1023 


#define KEYWORD_BG_ALPHA "bg_alpha"
#define KWD_BG_ALPHA 1024 




typedef struct {
  gchar *keyword;
  gint   keyvalue;
} keyword_s;

keyword_s keywords[] = {
  { KEYWORD_APLVIS, KWD_APLVIS },
  { KEYWORD_VIS_VERSION, KWD_VIS_VERSION },
  { KEYWORD_TITLE, KWD_TITLE },
  { KEYWORD_SETTINGS, KWD_SETTINGS },
  { KEYWORD_EXPRESSION, KWD_EXPRESSION },
  { KEYWORD_INDEPENDENT, KWD_INDEPENDENT },
  { KEYWORD_AXIS, KWD_AXIS },
  { KEYWORD_X_AXIS, KWD_X_AXIS },
  { KEYWORD_Y_AXIS, KWD_Y_AXIS },
  { KEYWORD_NAME, KWD_NAME },
  { KEYWORD_LABEL, KWD_LABEL },
  { KEYWORD_RANGE, KWD_RANGE },
  { KEYWORD_LIMIT, KWD_LIMIT },
  { KEYWORD_MINV, KWD_MINV },
  { KEYWORD_MAXV, KWD_MAXV },
  { KEYWORD_VALUE, KWD_VALUE },
  { KEYWORD_LOWER, KWD_LOWER },
  { KEYWORD_UPPER, KWD_UPPER },
  { KEYWORD_PAGE_SIZE, KWD_PAGE_SIZE },
  { KEYWORD_PAGE_INCREMENT, KWD_PAGE_INCREMENT },
  { KEYWORD_STEP_INCREMENT, KWD_STEP_INCREMENT },
  { KEYWORD_BG_RED, KWD_BG_RED },
  { KEYWORD_BG_GREEN, KWD_BG_GREEN },
  { KEYWORD_BG_BLUE, KWD_BG_BLUE },
  { KEYWORD_BG_ALPHA, KWD_BG_ALPHA },

};
gint nr_keys = 25;
#endif /* XML_KWDS_H*/
