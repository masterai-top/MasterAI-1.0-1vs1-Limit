#-----------------------------------------------------------

CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= ar rc

#-----------------------------------------------------------

NAME := libDTUtil


SRCPATH  := ../src 
FLAGS    := -Wall -g -fPIC
#CXXFLAGS := -Wall -g -fPIC
OUTPATH  := ../lib
OBJPATH  := .
COMPATH  := ../..
DEFS     :=

TARGET = $(OBJPATH)/$(NAME).a


#-----------------------------------------------------------

DIRS := $(shell find $(SRCPATH) -type d | grep -v .svn)  
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(sort $(filter-out .svn, $(DIRS)))

SRCS := $(foreach it, $(DIRS), $(wildcard $(it)/*.c $(it)/*.cpp))

OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(patsubst %.cpp, %.o, $(OBJS))


INCS += -I ../include
INCS += -I $(COMPATH)/include
#INCS += -I $(COMPATH)/encoding/include
#INCS += -I $(COMPATH)/shstd/include
#INCS += -I $(COMPATH)/frame/include
#INCS += -I $(COMPATH)/frame/src/net
#INCS += -I $(COMPATH)/acpc/include
#INCS += -I $(COMPATH)/event/include
#INCS += -I $(COMPATH)/protobuf/include
#INCS += -I $(COMPATH)/proto/include



#INCS += -I $(SRCPATH)


DEFS += -DBUILD_LIBUTIL


FLAGS   := $(FLAGS)   $(DEFS)
#CXXFLAGS := $(CXXFLAGS) $(DEFS)

#-----------------------------------------------------------

$(shell if [ ! -f $(OBJPATH) ]; then mkdir -p $(OBJPATH) ; fi)
$(shell if [ ! -f $(OUTPATH) ]; then mkdir -p $(OUTPATH) ; fi)

#-----------------------------------------------------------

#-----------------------------------------------------------
.PHONY: all install clean help

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) $@ $(LDFLAGS) $(addprefix $(OBJPATH)/, $(notdir $(OBJS)))
	rm -f $(OUTPATH)/$(NAME).a
	mv -f $(TARGET) $(OUTPATH)
%.o: %.c
	$(CC) -c $(FLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 
%.o: %.cpp
	$(CXX) -c $(FLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 	
	
install:
	cp -f $(TARGET) $(OUTPATH)

clean:
	rm -f $(addprefix $(OBJPATH)/, $(notdir $(OBJS))) $(TARGET)

help:
	@echo "make DEBUG=yes           Enable debug options"
	@echo "make clean DEBUG=yes     Clean all debug .o and out files"
	@echo "make install DEBUG=yes   Copy debug $(NAME) to debug dir"	
	@echo "make                     Disable debug options"
	@echo "make clean               Clean all release .o and out files"
	@echo "make install             Copy release $(NAME) to release dir"
