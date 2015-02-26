SHELL    := /bin/sh

# There seems to be a built-in rule that matches track_hack.rc which is not cancelled by
# clearing the suffix list and causes a circular dependency (make: Circular track_hack.rc
# <- track_hack.rc.o dependency dropped). This should disable all built-in rules.
MAKEFLAGS += --no-builtin-rules
# stackoverflow.com/questions/4122831/disable-make-builtin-rules-and-variables-from-inside
# gnu.org/software/make/manual/html_node/Canceling-Rules.html
# gnu.org/software/make/manual/html_node/Suffix-Rules.html
# gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html#Catalogue-of-Rules
# gnu.org/prep/standards/html_node/Makefile-Basics.html

# Clear the suffix list. See section 7.2.1 of the GNU Coding Standards.
.SUFFICES:

UNAME    := $(shell uname)
CXX      := g++
WXCONFIG := wx-config
IDIRS    :=
CXXFLAGS := $$($(WXCONFIG) --cxxflags) -std=c++14 -Wall -Wextra -Wno-old-style-cast \
            -pedantic $(addprefix -I, $(IDIRS))
CPPFLAGS := -std=c++14 $$($(WXCONFIG) --cppflags)
LDIRS    :=
LDLIBS   := $$($(WXCONFIG) --libs)
LDFLAGS  := $(CXXFLAGS) $(addprefix -L, $(LDIRS))
PROG     := track_hack
SRCS     := app.cpp bitmap.cpp color_pool.cpp frame.cpp main_frame.cpp movie.cpp \
            open_movie_wizard.cpp track_panel.cpp trackee.cpp tracker.cpp \
            trackee_box.cpp ibidi_export.cpp one_through_three.cpp
OBJS     := $(SRCS:.cpp=.o)
PREREQS  := $(SRCS:.cpp=.d)

debug:   CXXFLAGS += -g
release: CXXFLAGS += -Ofast -flto -fuse-linker-plugin
release: LDFLAGS  += -Ofast -flto -fuse-linker-plugin

ifeq ($(UNAME), Linux)
   LDLIBS += -lboost_thread -lboost_regex -lboost_filesystem -lboost_system
else
   ifeq ($(UNAME), MINGW32_NT-6.1)
      LDLIBS += -lboost_thread-mt -lboost_regex-mt -lboost_filesystem-mt -lboost_system-mt
      PROG := $(PROG:%=%.exe)
      RCFILE := track_hack.rc
   endif
endif

.PHONY: all clean

all: $(PROG)

debug:   all
release: all

ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),) # Any real goals?
   # Don't emit a warning if files are missing (leading "-"); their existence is ensured
   -include $(PREREQS) # when building the respective object files.
endif

# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
$(PROG): $(OBJS) $(RCFILE:%=%.o)
	$(CXX) $(LDFLAGS) $+ $(LDLIBS) -o $@

clean:
	$(RM) $(OBJS) $(RCFILE:%=%.o) $(PREREQS) $(PROG)

$(PREREQS):
	@set -e; echo "building $@"; \
	rule=$$($(CXX) -MM $(CPPFLAGS) $(patsubst %.d,%.cpp,$@)); \
	rule=$@' '$${rule}; \
	echo "$$rule" > $@

$(RCFILE:%=%.o): $(RCFILE)
	windres -I/mingw32/include/wx-3.0/ $< -o $@

.SECONDEXPANSION:

# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
$(OBJS): $$(subst .o,.d,$$@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(patsubst %.o,%.cpp,$@) -c -o $@

# vim: tw=90 ts=8 sw=3 noet
