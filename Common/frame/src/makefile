#-----------------------------------------------------------

CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= ar rc

#-----------------------------------------------------------

BASEPATH := ../..
FRAMEPATH := $(BASEPATH)/frame
SRCPATH := $(FRAMEPATH)/src
COMPATH  := ../..
NAME := libframe

ifndef $(DEBUG)
    DEBUG := NO
endif

ifeq (YES, $(shell echo $(DEBUG) | tr '[a-z]' '[A-Z]'))
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     := 
    LIBPATH     := $(FRAMEPATH)/lib/gcc_x64_debug
    OBJPATH     := $(FRAMEPATH)/obj/gcc_x64_debug/$(NAME)
    OUTPATH 	:= $(FRAMEPATH)/lib/gcc_x64_debug
    LIBEXT	:= d
    DEFS    	:= -D_DEBUG
	SUBLIBPATH	:= /lib/gcc_x64_debug
else
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     :=  
    LIBPATH 	:= $(FRAMEPATH)/lib/gcc_x64_release
    OBJPATH     := $(FRAMEPATH)/obj/gcc_x64_release/$(NAME)
    OUTPATH 	:= $(FRAMEPATH)/lib/gcc_x64_release
    LIBEXT      :=
    DEFS    	:=
	SUBLIBPATH	:= /lib/gcc_x64_release
endif

TARGET := $(OUTPATH)/$(NAME).a

#-----------------------------------------------------------

DIRS := $(shell find $(SRCPATH) -type d | grep -v .svn)  
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(sort $(filter-out .svn, $(DIRS)))

SRCS := $(foreach it, $(DIRS), $(wildcard $(it)/*.cpp))

OBJS := $(patsubst %.cpp, %.o, $(SRCS))

INCS = -I $(BASEPATH)/iconv/include
INCS += -I $(BASEPATH)/event/include
#INCS += -I $(BASEPATH)/protobuf/include
#INCS += -I $(BASEPATH)/proto/include
#INCS += -I $(BASEPATH)/luajit-2.0/include
INCS += -I $(BASEPATH)/shstd/include
INCS += -I $(BASEPATH)/frame/include
#INCS += -I $(BASEPATH)/tinyxml/include
INCS += -I $(SRCPATH)
#INCS += -I $(SRCPATH)/net
INCS += -I $(FRAMEPATH)/include/log
INCS += -I $(BASEPATH)/include
#INCS += -I $(COMPATH)/encoding/include
INCS += -I $(COMPATH)/util/include
#INCS += -I $(COMPATH)/script/include

#LIBS = -L $(BASEPATH)/event/$(SUBLIBPATH) -levent -levent_pthreads 
#LIBS += -L $(BASEPATH)/iconv/$(SUBLIBPATH) -liconv 
#LIBS += -L $(BASEPATH)/luajit-2.0/$(SUBLIBPATH) -lluajit-5.1
#LIBS += -L $(BASEPATH)/tinyxml/$(SUBLIBPATH) -ltinyxml
#LIBS += -L $(BASEPATH)/protobuf/$(SUBLIBPATH) -lprotobuf
#LIBS += -L /usr/lib -lpthread -ldl -lrt

DEFS += -DBUILD_LIBFRAME #-DLIMIT_LOG_ENABLE

CFLAGS   := $(CFLAGS)   $(DEFS)
CXXFLAGS := $(CXXFLAGS) $(DEFS)

#-----------------------------------------------------------

$(shell if [ ! -f $(OBJPATH) ]; then mkdir -p $(OBJPATH) ; fi)
$(shell if [ ! -f $(OUTPATH) ]; then mkdir -p $(OUTPATH) ; fi)

#-----------------------------------------------------------

.PHONY: all install clean help test

all: $(TARGET) 

$(TARGET): $(OBJS)
	#$(CXX) $(LDFLAGS) $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(LIBS) -o $@
	$(AR) $@ $(LDFLAGS) $(addprefix $(OBJPATH)/, $(notdir $(OBJS)))

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 

install:
	cp $(TARGET) $(OUTPATH)
test:
	rm -f /home/zhonghailong/awzs/sbin/$(NAME).a 
	#cp -f $(TARGET) /home/zhonghailong/awzs/sbin/
clean:
	rm -f $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(TARGET)

help:
	@echo "make DEBUG=yes		Enable debug options"
	@echo "make clean DEBUG=yes	Clean all debug .o and .so files"
	@echo "make install DEBUG=yes	Copy debug libjjserver.so to debug dir"
	@echo "make			Disable debug options"
	@echo "make clean		Clean all release .o and .so files"
	@echo "make install		Copy release libjjserver.so to release dir"
