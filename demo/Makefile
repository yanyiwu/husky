CXX    := g++
LD     := g++
AR     := ar rc

INCS := 

DEBUG_CXXFLAGS     := -g -Wall -DDEBUG 
RELEASE_CXXFLAGS   := -Wall -O3

ifeq (YES, ${DEBUG})
   CXXFLAGS       := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

DOLINK := $(LD) $(LDFLAGS) 
DOPACK := $(AR)  
SOURCES := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))
INC := -I../limonp

SRCDIR = ../husky
SRCLIB = $(SRCDIR)/libhusky.a


LIBA =  $(SRCLIB) $(LIBCMA)

# remove the objs after compilation
.PHONY: clean $(LIBA)

TARGET = husky_demo

# Main Targets
all: $(TARGET)

# This is a suffix rule 
#.c.o: 
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< $(INC)

$(TARGET): husky_demo.cpp $(LIBA)
	$(CXX) -o $@ $< $(LIBA) $(INC) -lpthread

$(SRCLIB): 
	cd $(SRCDIR) && $(MAKE)

$(LIBCMA): 
	cd $(CMDIR) && $(MAKE)

clean:
	rm -f *.o *.ut *.d *.d.* $(TARGET)
	cd $(SRCDIR) && make clean

sinclude $(SOURCES:.cpp=.d)
%.d:%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
