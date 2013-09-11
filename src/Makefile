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
SOURCES = Daemon.cpp\
	XmlHttp.cpp\
	ServerFrameLockAccept.cpp
OBJS := $(patsubst %.cpp,%.o,$(SOURCES))

CMDIR := ../cppcommon
CMLIB := $(CMDIR)/libcm.a

TMPDIR := ./huskytmp

LIBA := libhusky.a

# remove the objs after compilation
.INTERMEDIATE: 
.PHONY: clean $(CMLIB) 

all: $(LIBA)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $<

${LIBA}: $(TMPDIR) $(OBJS) $(CMLIB) 
	cp $(CMLIB) $(TMPDIR) && cd $(TMPDIR) && ar x `basename $(CMLIB)`
	$(DOPACK) $@ $(OBJS) $(TMPDIR)/*.o 
	rm -rf $(TMPDIR)

$(TMPDIR):
	mkdir $@

$(CMLIB):
	cd $(CMDIR) && $(MAKE)

all:  ${LIBA}

clean:
	rm -f *.o *.d *.d.* $(LIBA) 
	rm -rf $(TMPDIR)
	cd $(CMDIR) && make clean

sinclude $(SOURCES:.cpp=.d)
%.d:%.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $< > $@.$$$$; \
	sed 's,\($*\).o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
