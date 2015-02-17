SHELL := /bin/sh
CXX := g++
WXCONFIG := wx-config-2.9
IDIRS := /c/boost/boost_1_49_0
CXXFLAGS := -std=gnu++11 -Wall -Wextra -Wno-old-style-cast -pedantic -g -c \
	$$($(WXCONFIG) --cxxflags) $(addprefix -I, $(IDIRS))
CPPFLAGS := -std=gnu++0x $$($(WXCONFIG) --cppflags)
LDIRS := /c/boost/boost_1_49_0/stage/lib/
LDLIBS := -lboost_thread -lboost_regex -lboost_filesystem \
	-lboost_system $$($(WXCONFIG) --libs)
LDFLAGS := -std=c++11 -Wall -Wextra -Wno-old-style-cast -pedantic $(addprefix -L, $(LDIRS))
PROG := track_hack.exe
SRCS := app.cpp bitmap.cpp frame.cpp main_frame.cpp movie.cpp open_movie_wizard.cpp track_panel.cpp trackee.cpp tracker.cpp trackee_box.cpp one_through_three.cpp
OBJS := $(SRCS:.cpp=.o)
PREREQS := $(SRCS:.cpp=.d)

.PHONY: all clean

all: $(OBJS) $(PROG)

ifneq ($(filter-out clean,$(or $(MAKECMDGOALS),all)),) # any real goals?
    # don't emit a warning if files are missing; their existence is ensured upon building
    -include $(PREREQS) # building respective object file (thus the leading "-")
endif

$(PROG): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $(PROG)

#$(PROG): $(OBJS)
	#windres -I/c/wx/include/ track_hack.rc -o track_hack_rc.o
	#$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) track_hack_rc.o -o $(PROG)

clean:
	$(RM) $(OBJS) $(PREREQS) $(PROG)

$(PREREQS):
	@set -e; echo "building $@"; \
	rule=$$($(CXX) -MM $(CPPFLAGS) $(patsubst %.d,%.cpp,$@)); \
	rule=$@' '$${rule}; \
	echo "$$rule" > $@

.SECONDEXPANSION:

$(OBJS): $$(subst .o,.d,$$@)
	$(CXX) $(CXXFLAGS) $(patsubst %.o,%.cpp,$@) -o $@
