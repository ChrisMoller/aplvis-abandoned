bin_PROGRAMS = aplvis
#AM_YFLAGS = -d -v
#AM_LFLAGS = -o$(LEX_OUTPUT_ROOT).c
aplvis_SOURCES = aplvis.c \
DISTCLEANFILES = .deps/*.P
APL_LIB = -L `apl --show_lib_dir` -lapl -lstdc++


aplvis_CFLAGS = -std=c99 -Wall -Werror \
              $(GTK_CFLAGS) $(CAIRO_CFLAGS) $(MAGICKWAND_CFLAGS)  \
              $(FONTCONFIG_CFLAGS) $(PLPLOT_CFLAGS)

aplvis_LDFLAGS = -lm $(GTK_LIBS) $(CAIRO_LIBS) $(MAGICKWAND_LIBS) \
              $(FONTCONFIG_LIBS)  $(PLPLOT_LIBS) $(APL_LIB)

# -leak-check=full
#  algrind --tool=memcheck --leak-check=full --leak-resolution=high -
#                -num-callers=20 --log-file=vgdump ./gfig
#  
#VALGRIND_FLAGS = --gen-suppressions=all --run-libc-freeres=yes \
#    --suppressions=valgrind.supp
#VALGRIND_FLAGS = --gen-suppressions=all --run-libc-freeres=yes

valgrind.log : aplvis
	valgrind $(VALGRIND_FLAGS) --log-file=$@ ./vis

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	rm -rf vis-parse.c vis-parse.h vis-lex.c