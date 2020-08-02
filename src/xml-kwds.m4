dnl generator for xml-kwds.m4
define(`RQ',`changequote(<,>)dnl`
'changequote`'')
dnl start at 1000 just so we don't have to with a null pointer
define(`offset', 1000)dnl
define(`cnt',    offset)dnl
define(`xinc', `define(`$1',incr($1))')dnl
define(`upcase', `translit($1, `a-z', `A-Z')')dnl
define(`entry', ``#'define `KEYWORD_'upcase($1) "$1"
`#'define `KWD_'upcase($1) cnt xinc(`cnt')
divert(1)  `{' `KEYWORD_'upcase($1), `KWD_'upcase($1) `},'
divert(0)'
)dnl

`/********* DON'RQ()`T MODIFY THIS FILE! ********/'
`/**** Make all changes in xml-kwds.m4. ****/'

#ifndef XML_KWDS_H
#define XML_KWDS_H

entry(aplvis)
entry(vis_version)
entry(title)
entry(settings)
entry(expression)
entry(independent)
entry(axis)
entry(x_axis)
entry(y_axis)
entry(name)
entry(label)
entry(range)
entry(limit)
entry(minv)
entry(maxv)
entry(value)
entry(lower)
entry(upper)
entry(page_size)
entry(page_increment)
entry(step_increment)
entry(bg_red)
entry(bg_green)
entry(bg_blue)
entry(bg_alpha)


`typedef struct {
  gchar *keyword;
  gint   keyvalue;
} keyword_s;

keyword_s keywords[] = {'
undivert
`};
gint nr_keys =' eval(cnt - offset)`;'
#endif /* XML_KWDS_H*/
