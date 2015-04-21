# "Every Makefile should contain this line" - GNU Coding Standards, section 7.2.1
SHELL := /bin/sh

# There seems to be a built-in rule that matches track_hack.rc which is not cancelled by
# clearing the suffix list and causes a circular dependency (make: Circular track_hack.rc
# <- track_hack.rc.o dependency dropped). This should disable all built-in rules.
# stackoverflow.com/questions/4122831/disable-make-builtin-rules-and-variables-from-inside
# gnu.org/software/make/manual/html_node/Canceling-Rules.html
# gnu.org/software/make/manual/html_node/Suffix-Rules.html
# gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html#Catalogue-of-Rules
MAKEFLAGS += --no-builtin-rules

# Clear the suffix list. See section 7.2.1 of the GNU Coding Standards.
.SUFFICES:

# gnu.org/prep/standards/html_node/Makefile-Basics.html

CXX      := g++
UNAME    := $(shell uname)
IDIRS    :=
CXXFLAGS := -Wall -Wextra -Wno-old-style-cast -pedantic
CPPFLAGS :=
LDDIRS   :=
LDFLAGS  :=
LDLIBS   :=

##########################################################################################

# Default is release build so users can do a normal make.
DEBUG ?= 0
ifeq ($(DEBUG), 0)
   CXXFLAGS := -O3 -flto -fuse-linker-plugin $(CXXFLAGS)
   OBJDIR := build/release
else
   CXXFLAGS := -DDEBUG -g $(CXXFLAGS)
   OBJDIR := build/debug
endif
# stackoverflow.com/questions/1079832/how-can-i-configure-my-makefile-for-debug-and-releas
# stackoverflow.com/questions/792217/simple-makefile-with-release-and-debug-builds-best-pr

program := $(addprefix $(OBJDIR)/,track_hack)
sources := $(wildcard *.cpp)
objects := $(addprefix $(OBJDIR)/,$(sources:.cpp=.o))
depends := $(addprefix $(OBJDIR)/,$(sources:.cpp=.d))

CXXFLAGS := $$(wx-config --cxxflags) -std=c++14 $(CXXFLAGS) $(addprefix -I, $(IDIRS))
CPPFLAGS := $$(wx-config --cppflags) $(CPPFLAGS)
LDFLAGS  += $(addprefix -L, $(LDDIRS))
LDLIBS   += $$(wx-config --libs)

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

ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),) # Any real goals?
   # Include existant dependency files.
   include $(filter $(depends),$(shell find -regex '.*\.d' -printf '%P\n'))
endif

# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
$(OBJDIR)/%.o: $(OBJDIR)/%.d | $(OBJDIR)
	$(CXX) -MMD $(CXXFLAGS) $(CPPFLAGS) $*.cpp -c -o $@

$(OBJDIR):
	mkdir -p $@

# Running 'make -p' in a directory with no makefile yields the full list of default rules
# and variables.
# https://www.gnu.org/software/make/manual/html_node/Catalogue-of-Rules.html
$(program): $(objects) $(rcfile:%=%.o) | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $+ $(LDLIBS) -o $@

# See make.mad-scientist.net/papers/advanced-auto-dependency-generation: Avoiding "No rule
# to make target ..." Errors. This is a more simple approach to solve the same problem.
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
# to the dependency file. The dependency file itself will be created along with the object
# file and included during the next invocation of Make.
$(depends):

# vim: tw=90 ts=8 sts=3 sw=3 noet
