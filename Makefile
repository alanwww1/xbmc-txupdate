DEBUG          := YES

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

DEBUG_CXXFLAGS   := -Wall -Wno-format -g -DDEBUG
RELEASE_CXXFLAGS := -Wall -Wno-unknown-pragmas -Wno-format -O3

LIBS		 := 

DEBUG_LDFLAGS    := -g
RELEASE_LDFLAGS  :=

ifeq (YES, ${DEBUG})
   CXXFLAGS     := ${DEBUG_CXXFLAGS}
   LDFLAGS      := ${DEBUG_LDFLAGS}
else
   CXXFLAGS     := ${RELEASE_CXXFLAGS}
   LDFLAGS      := ${RELEASE_LDFLAGS}
endif

INCS := -I./Libs

OUTPUT := xbmc-txupdate

all: ${OUTPUT}

SRCS := lib/TinyXML/tinyxml.cpp lib/TinyXML/tinyxmlparser.cpp lib/TinyXML/tinystr.cpp lib/TinyXML/tinyxmlerror.cpp \
lib/POUtils/POUtils.cpp \
lib/CharsetUtils/CharsetUtils.cpp \
lib/vJSON/json.cpp lib/vJSON/block_allocator.cpp \
$(OUTPUT)

OBJS := $(addsuffix .o,$(basename ${SRCS}))

${OUTPUT}: ${OBJS} lib/xbmclangcodes.h
	${LD} -o $@ ${LDFLAGS} ${OBJS} ${LIBS}

%.o : %.cpp
	${CXX} -c ${CXXFLAGS} ${INCS} $< -o $@

dist:
	bash makedistlinux

clean:
	-rm -f core ${OBJS} ${OUTPUT}

tinyxml.o: tinyxml.h tinyxml.cpp tinystr.o tinyparser.o tinyxmlerror.o
tinyxmlparser.o: tinyxmlparser.cpp tinyxmlparser.h
tinyxmlerror.o: tinyxmlerror.cpp tinyxmlerror.h
tinystr.o: tinystr.cpp tinystr.h
POUtils.o: POUtils.h POUtils.cpp
CharsteUtils.o: CharsetUtils.h CharsetUtils.cpp
json.o: json.cpp json.h block_allocator.o
block_allocator.o: block_allocator.cpp block_allocator.h
