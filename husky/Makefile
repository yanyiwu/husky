CXX    := g++
LD     := g++
AR     := ar rc

INCS := -I. -I./cppcommon

DEBUG_CXXFLAGS     := -g -Wall -DDEBUG 
RELEASE_CXXFLAGS   := -Wall -O3

ifeq (YES, ${RELEASE})
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
else
   CXXFLAGS       := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
endif

DOLINK := $(LD) $(LDFLAGS) -o $@ $^
DOPACK := $(AR)  
SOURCES = Daemon.cpp ServerFrameLockAccept.cpp
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))

LIBA := libhusky.a

# remove the objs after compilation
.INTERMEDIATE: 
.PHONY: clean 

all: $(LIBA)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

${LIBA}: $(OBJS) 
	$(DOPACK) $@ $(OBJS) 

all:  ${LIBA}

clean:
	rm -f *.o *.d *.d.* $(LIBA) 

sinclude $(SOURCES:.cpp=.d)
%.d:%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
