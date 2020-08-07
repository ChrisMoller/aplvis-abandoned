

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


#define KEYWORD_X_LABEL "x_label"
#define KWD_X_LABEL 1021 


#define KEYWORD_Y_LABEL "y_label"
#define KWD_Y_LABEL 1022 


#define KEYWORD_Z_LABEL "z_label"
#define KWD_Z_LABEL 1023 


#define KEYWORD_MODE "mode"
#define KWD_MODE 1024 


#define KEYWORD_COORDS "coords"
#define KWD_COORDS 1025 


#define KEYWORD_X_INDEX_2D "x_index_2d"
#define KWD_X_INDEX_2D 1026 


#define KEYWORD_X_INDEX_3D "x_index_3d"
#define KWD_X_INDEX_3D 1027 


#define KEYWORD_Y_INDEX_3D "y_index_3d"
#define KWD_Y_INDEX_3D 1028 


#define KEYWORD_CARTESIAN "cartesian"
#define KWD_CARTESIAN 1029 


#define KEYWORD_POLAR "polar"
#define KWD_POLAR 1030 


#define KEYWORD_CYLINDRICAL "cylindrical"
#define KWD_CYLINDRICAL 1031 


#define KEYWORD_SPHERICAL "spherical"
#define KWD_SPHERICAL 1032 


#define KEYWORD_2D "2d"
#define KWD_2D 1033 


#define KEYWORD_3D "3d"
#define KWD_3D 1034 


#define KEYWORD_COLOURS "colours"
#define KWD_COLOURS 1035 


#define KEYWORD_COLOUR "colour"
#define KWD_COLOUR 1036 


#define KEYWORD_INDEX "index"
#define KWD_INDEX 1037 


#define KEYWORD_RED "red"
#define KWD_RED 1038 


#define KEYWORD_GREEN "green"
#define KWD_GREEN 1039 


#define KEYWORD_BLUE "blue"
#define KWD_BLUE 1040 


#define KEYWORD_ALPHA "alpha"
#define KWD_ALPHA 1041 




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
  { KEYWORD_X_LABEL, KWD_X_LABEL },
  { KEYWORD_Y_LABEL, KWD_Y_LABEL },
  { KEYWORD_Z_LABEL, KWD_Z_LABEL },
  { KEYWORD_MODE, KWD_MODE },
  { KEYWORD_COORDS, KWD_COORDS },
  { KEYWORD_X_INDEX_2D, KWD_X_INDEX_2D },
  { KEYWORD_X_INDEX_3D, KWD_X_INDEX_3D },
  { KEYWORD_Y_INDEX_3D, KWD_Y_INDEX_3D },
  { KEYWORD_CARTESIAN, KWD_CARTESIAN },
  { KEYWORD_POLAR, KWD_POLAR },
  { KEYWORD_CYLINDRICAL, KWD_CYLINDRICAL },
  { KEYWORD_SPHERICAL, KWD_SPHERICAL },
  { KEYWORD_2D, KWD_2D },
  { KEYWORD_3D, KWD_3D },
  { KEYWORD_COLOURS, KWD_COLOURS },
  { KEYWORD_COLOUR, KWD_COLOUR },
  { KEYWORD_INDEX, KWD_INDEX },
  { KEYWORD_RED, KWD_RED },
  { KEYWORD_GREEN, KWD_GREEN },
  { KEYWORD_BLUE, KWD_BLUE },
  { KEYWORD_ALPHA, KWD_ALPHA },

};
gint nr_keys = 42;
#endif /* XML_KWDS_H*/
