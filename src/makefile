# "Every Makefile should contain this line".  See section 7.2.1 of the GNU Coding
# Standards [1].
SHELL := /bin/sh

# There seems to be a built-in rule that matches `track_hack.rc` which is not canceled by
# clearing the suffix list and causes a circular dependency (make: Circular track_hack.rc
# <- track_hack.rc.o dependency dropped).  This should disable all built-in rules.  See
# [2], [3], [4], and [5].
MAKEFLAGS += --no-builtin-rules

# Clear the suffix list.  See section 7.2.1 of the GNU Coding Standards [1].
.SUFFIXES:

CXX      := g++
UNAME    := $(shell uname)
WXCONFIG := wx-config
IDIRS    :=
CXXFLAGS := -Wall -Wextra -Wno-old-style-cast -Wno-deprecated-declarations -pedantic
CPPFLAGS :=
LDDIRS   :=
LDFLAGS  :=
LDLIBS   :=

##########################################################################################

# Create a release build by default so users can simply run `make`.  See [6] and [7].
DEBUG ?= 0
ifeq ($(DEBUG), 0)
   CXXFLAGS := -O3 -flto -fuse-linker-plugin $(CXXFLAGS)
   OBJDIR := ../build/release
else
   CXXFLAGS := -DDEBUG -g $(CXXFLAGS)
   OBJDIR := ../build/debug
endif

program := $(addprefix $(OBJDIR)/,track_hack)
sources := $(wildcard *.cpp)
objects := $(addprefix $(OBJDIR)/,$(sources:.cpp=.o))
depends := $(addprefix $(OBJDIR)/,$(sources:.cpp=.d))

CXXFLAGS := $$($(WXCONFIG) --cxxflags) -std=c++14 $(CXXFLAGS) $(addprefix -I, $(IDIRS))
CPPFLAGS := $$($(WXCONFIG) --cppflags) $(CPPFLAGS)
LDFLAGS  += $(addprefix -L, $(LDDIRS))
LDLIBS   += $$($(WXCONFIG) --libs)

ifeq ($(UNAME), Linux)
   LDLIBS += -lboost_thread -lboost_regex -lboost_filesystem -lboost_system
else
   ifeq ($(UNAME), MINGW32_NT-6.1)
      LDLIBS += -lboost_thread-mt -lboost_regex-mt -lboost_filesystem-mt -lboost_system-mt
      program := $(program:%=%.exe)
      rcfile := track_hack.rc
   endif
endif

.PHONY: all clean

all: $(program)

ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),)  # Any real goals?
   # Include existing dependency files.
   include $(filter $(depends),$(shell find $(OBJDIR) -regex '.*\.d'))
endif

# See [8].
$(OBJDIR)/%.o: $(OBJDIR)/%.d | $(OBJDIR)
	$(CXX) -MMD $(CXXFLAGS) $(CPPFLAGS) $*.cpp -c -o $@

$(OBJDIR):
	mkdir -p $@

# Running `make -p` in a directory with no makefile yields the full list of default rules
# and variables.  See [8].
$(program): $(objects) $(rcfile:%=%.o) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $+ $(LDLIBS) -o $@

# See [9].  This is a more simple approach to solve the same problem.
%.h: ;
%.H: ;
%.hh: ;
%.hpp: ;
%.hxx: ;

clean:
	$(RM) $(objects) $(rcfile:%=%.o) $(depends) $(program)

$(rcfile:%=%.o): $(rcfile)
	windres -I/mingw32/include/wx-3.0/ $< -o $@

# If one of these targets (dependency files) doesn't exist, Make will imagine it to have
# been updated when this rule is run and will also rebuilt the object file corresponding
# to the dependency file.  The dependency file itself will be created along with the
# object file and included during the next invocation of Make.
$(depends):

# [1]: https://www.gnu.org/prep/standards/standards.html#Makefile-Conventions
# [2]: https://stackoverflow.com/q/4122831
# [3]: https://gnu.org/software/make/manual/html_node/Canceling-Rules.html
# [4]: https://gnu.org/software/make/manual/html_node/Suffix-Rules.html
# [5]: https://gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html#Catalogue-of-Rules
#      "Catalogue of Built-In Rules - section 10.2 of the GNU Make Manual"
# [6]: https://stackoverflow.com/q/1079832
# [7]: https://stackoverflow.com/q/792217
# [8]: https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
# [9]: https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#norule
