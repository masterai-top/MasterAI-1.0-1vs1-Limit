#-----------------------------------------------------------

CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= ar rc

#-----------------------------------------------------------

NAME := libproto

SVRPATH := ../../..
SRCPATH := $(SVRPATH)/Common/proto/src

ifndef $(DEBUG)
    DEBUG := NO
endif

ifeq (YES, $(shell echo $(DEBUG) | tr '[a-z]' '[A-Z]'))
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     := 
    OBJPATH     := .
    OUTPATH 	:= $(SVRPATH)/Common/proto/lib
    LIBEXT      := d
    DEFS        := -D_DEBUG
else
    #CFLAGS      := -Wall -O3 -fPIC
    #CXXFLAGS    := -Wall -O3 -fPIC
    CFLAGS      := -Wall -g -fPIC
    CXXFLAGS    := -Wall -g -fPIC
    LDFLAGS     :=
    OBJPATH     := .
    OUTPATH 	:= $(SVRPATH)/Common/proto/lib
    LIBEXT      :=
    DEFS        :=
endif

TARGET = $(OBJPATH)/$(NAME).a

#-----------------------------------------------------------

DIRS := $(shell find $(SRCPATH) -type d | grep -v .svn)  
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(sort $(filter-out .svn, $(DIRS)))

SRCS := $(foreach it, $(DIRS), $(wildcard $(it)/*.cc))

OBJS := $(patsubst %.cc, %.o, $(SRCS))


INCS += -I $(SVRPATH)/Common/protobuf/include
INCS += -I $(SVRPATH)/Common/proto/include
INCS += -I $(SRCPATH)


#DEFS += -DBUILDING_LIBACPC


CFLAGS   := $(CFLAGS)   $(DEFS)
CXXFLAGS := $(CXXFLAGS) $(DEFS)

#-----------------------------------------------------------

$(shell if [ ! -f $(OBJPATH) ]; then mkdir -p $(OBJPATH) ; fi)
$(shell if [ ! -f $(OUTPATH) ]; then mkdir -p $(OUTPATH) ; fi)

#-----------------------------------------------------------

#-----------------------------------------------------------
.PHONY: all install clean help

all: $(TARGET)  cplib

$(TARGET): $(OBJS)
	$(AR) $@ $(LDFLAGS) $(addprefix $(OBJPATH)/, $(notdir $(OBJS)))

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 
%.o: %.cc
	$(CC) -c $(CFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 
	
install:
	cp -f $(TARGET) $(OUTPATH)

clean:
	rm -f $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(TARGET)
cplib:
	rm -f $(OUTPATH)/$(NAME).a
	cp -f $(TARGET) $(OUTPATH)

help:
	@echo "make DEBUG=yes           Enable debug options"
	@echo "make clean DEBUG=yes     Clean all debug .o and out files"
	@echo "make install DEBUG=yes   Copy debug $(NAME) to debug dir"	
	@echo "make                     Disable debug options"
	@echo "make clean               Clean all release .o and out files"
	@echo "make install             Copy release $(NAME) to release dir"
