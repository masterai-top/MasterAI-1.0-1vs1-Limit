#-----------------------------------------------------------

CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= ar rc

#-----------------------------------------------------------

NAME := apgisvr

COMPATH := ../../Common
SRCPATH := ../src

ifndef $(DEBUG)
    DEBUG := NO
endif

ifeq (YES, $(shell echo $(DEBUG) | tr '[a-z]' '[A-Z]'))
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     :=  
    #LIBPATH     := gcc_x64_debug
    LIBPATH     := gcc_x64_release
    OBJPATH     := .
    OUTPATH     := ../bin
    LIBEXT      := 
    DEFS        := -D_DEBUG
else
    #CFLAGS      := -Wall -O3 -fPIC
    #CXXFLAGS    := -Wall -O3 -fPIC
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     := 
    LIBPATH     := gcc_x64_release
    OBJPATH     := .
    OUTPATH     := ../bin
    LIBEXT      :=
    DEFS        :=
endif

TARGET = $(OBJPATH)/$(NAME)

#-----------------------------------------------------------

DIRS := $(shell find $(SRCPATH) -type d | grep -v .svn)  
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(sort $(filter-out .svn, $(DIRS)))

SRCS := $(foreach it, $(DIRS), $(wildcard $(it)/*.cpp))

OBJS := $(patsubst %.cpp, %.o, $(SRCS))

INCS = -I ../include
INCS += -I $(COMPATH)/util/include
INCS += -I $(COMPATH)/script/include
INCS += -I $(COMPATH)/network/include
INCS += -I $(COMPATH)/frame/include
INCS += -I $(COMPATH)/iconv/include
INCS += -I $(COMPATH)/public/include
INCS += -I $(COMPATH)/event/include
#INCS += -I $(COMPATH)/luajit-2.0/include
INCS += -I $(COMPATH)/protobuf/include
INCS += -I $(COMPATH)/hiredis/include
INCS += -I $(COMPATH)/shstd/include
INCS += -I $(COMPATH)/r3c/src
INCS += -I $(COMPATH)/acpc/include
INCS += -I $(COMPATH)/proto/include
INCS += -I $(COMPATH)/include
INCS += -I $(COMPATH)/frame/src/net
INCS += -I $(COMPATH)/jsoncpp/jsoncpp-src-0.5.0/include
INCS += -I $(COMPATH)/SBot/include
INCS += -I $(SRCPATH)


LIBS = -L $(COMPATH)/frame/lib/$(LIBPATH) -lframe
LIBS += -L $(COMPATH)/script/lib -lDTScript
LIBS += -L $(COMPATH)/util/lib -lDTUtil
LIBS += -L $(COMPATH)/event/lib/$(LIBPATH) -levent -levent_pthreads
#LIBS += -L $(COMPATH)/luajit-2.0/lib/$(LIBPATH) -lluajit-5.1
LIBS += -L $(COMPATH)/iconv/lib/$(LIBPATH) -liconv
LIBS += -L $(COMPATH)/tinyxml/lib/$(LIBPATH) -ltinyxml
LIBS += -L $(COMPATH)/shstd/lib -lshstd
LIBS += -L $(COMPATH)/r3c/lib -lr3c
LIBS += -L $(COMPATH)/version/lib -lcomVer
LIBS += -L $(COMPATH)/jsoncpp/jsoncpp-src-0.5.0/libs/linux-gcc-7 -ljson_linux-gcc-7_libmt
LIBS += -L $(COMPATH)/public/lib -lDTPublic
LIBS += -L $(COMPATH)/network/lib -lDTNetwork
LIBS += -L $(COMPATH)/hiredis/lib -lhiredis
LIBS += -L $(COMPATH)/acpc/lib -lacpc
LIBS += -L $(COMPATH)/proto/lib -lproto
LIBS += -L $(COMPATH)/SBot/lib -lSBotInterface
LIBS += -L $(COMPATH)/protobuf/lib/$(LIBPATH) -lprotobuf$(LIBEXT)
LIBS += -L /usr/lib -lpthread -ldl -lrt

CFLAGS   := $(CFLAGS)   $(DEFS)
CXXFLAGS := $(CXXFLAGS) $(DEFS) -DGAME_FILE_C

#-----------------------------------------------------------

$(shell if [ ! -f $(OBJPATH) ]; then mkdir -p $(OBJPATH) ; fi)
$(shell if [ ! -f $(OUTPATH) ]; then mkdir -p $(OUTPATH) ; fi)

#-----------------------------------------------------------

#-----------------------------------------------------------
#all clean
.PHONY: all install clean help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(LIBS) -o $@

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 

install:
	cp -f $(TARGET) $(OUTPATH)
	cp -f $(COMPATH)/frame/lib/$(LIBPATH)/libframe.so $(OUTPATH)

test:
	#rm -f ../../../../awzs/sbin/$(NAME)
	#cp -f $(TARGET) ../../../../awzs/sbin/

clean:
	rm -f $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(TARGET)

help:
	@echo "make DEBUG=yes           Enable debug options"
	@echo "make clean DEBUG=yes     Clean all debug .o and out files"
	@echo "make install DEBUG=yes   Copy debug $(NAME) to debug dir"
	@echo "make                     Disable debug options"
	@echo "make clean               Clean all release .o and out files"
	@echo "make install             Copy release $(NAME) to release dir"
	
