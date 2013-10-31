CXX    := g++
LD     := g++
AR     := ar rc

DEBUG_CXXFLAGS     := -g -Wall -DDEBUG 
RELEASE_CXXFLAGS   := -Wall -O3

ifeq (YES, ${DEBUG})
   CXXFLAGS       := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

SOURCES := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))
DEMOS := $(patsubst %.cpp,%.demo,$(SOURCES))
INC := -I../limonp

SRCDIR = ../husky
SRCLIB = $(SRCDIR)/libhusky.a

LIBA =  $(SRCLIB) 

.PHONY: clean $(LIBA)

TARGET = $(DEMOS)

all: $(TARGET)

%.demo: %.cpp $(LIBA)
	$(CXX) -o $@ $< $(LIBA) $(INC) -lpthread

$(SRCLIB): 
	cd $(SRCDIR) && $(MAKE)

clean:
	rm -f *.o *.ut *.d *.d.* $(TARGET)
	cd $(SRCDIR) && make clean

sinclude $(SOURCES:.cpp=.d)
%.d:%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
