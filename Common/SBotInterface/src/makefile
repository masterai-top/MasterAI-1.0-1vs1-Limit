#-----------------------------------------------------------

CC	:= gcc
CXX	:= g++
LD	:= g++
AR	:= ar rc

#-----------------------------------------------------------

NAME := libSBotInterface


SRCPATH  := ../src 
CFLAGS   := -Wall -g -fPIC
CXXFLAGS := -Wall -g -fPIC
#LDFLAGS  := -shared
OUTPATH  := ../lib

LIBPATH  := ../../SBot/lib
OBJPATH  := .
COMPATH  := ../..

TARGET = $(OBJPATH)/$(NAME).a

#-----------------------------------------------------------

DIRS := $(shell find $(SRCPATH) -type d | grep -v .svn)  
DIRS := $(basename $(patsubst ./%, %, $(DIRS)))
DIRS := $(sort $(filter-out .svn, $(DIRS)))

SRCS := $(foreach it, $(DIRS), $(wildcard $(it)/*.c $(it)/*.cpp))

OBJS := $(patsubst %.c, %.o, $(SRCS))
OBJS := $(patsubst %.cpp, %.o, $(OBJS))


INCS = -I ../include
INCS += -I $(COMPATH)/shstd/include
INCS += -I $(COMPATH)/frame/include
INCS += -I $(COMPATH)/iconv/include
INCS += -I $(COMPATH)/event/include
#INCS += -I $(COMPATH)/luajit-2.0/include
INCS += -I $(COMPATH)/protobuf/include
INCS += -I $(COMPATH)/public/include
#INCS += -I $(COMPATH)/hiredis/include
#INCS += -I $(COMPATH)/r3c/src
#INCS += -I $(COMPATH)/acpc/include
INCS += -I $(COMPATH)/proto/include
INCS += -I $(COMPATH)/include
INCS += -I $(COMPATH)/frame/src/net
INCS += -I $(SRCPATH)
INCS += -I $(COMPATH)/jsoncpp/jsoncpp-src-0.5.0/include
INCS += -I $(COMPATH)/SBot/include

LIBS = -L $(COMPATH)/frame/lib/$(LIBPATH) -lframe
LIBS += -L $(COMPATH)/event/lib/$(LIBPATH) -levent -levent_pthreads
#LIBS += -L $(COMPATH)/luajit-2.0/lib/$(LIBPATH) -lluajit-5.1
LIBS += -L $(COMPATH)/iconv/lib/$(LIBPATH) -liconv
LIBS += -L $(COMPATH)/tinyxml/lib/$(LIBPATH) -ltinyxml
LIBS += -L $(COMPATH)/jsoncpp/jsoncpp-src-0.5.0/libs/linux-gcc-7 -ljson_linux-gcc-7_libmt
LIBS += -L $(COMPATH)/public/lib -lDTPublic
#LIBS += -L $(COMPATH)/r3c/lib -lr3c
#LIBS += -L $(COMPATH)/hiredis/lib -lhiredis
#LIBS += -L $(COMPATH)/acpc/lib -lacpc
LIBS += -L $(COMPATH)/proto/lib -lproto
LIBS += -L $(COMPATH)/protobuf/lib/$(LIBPATH) -lprotobuf$(LIBEXT)
LIBS += -L /usr/lib -lpthread -ldl -lrt


#DEFS += -DBUILDING_LIBACPC


CFLAGS   := $(CFLAGS)   $(DEFS)
CXXFLAGS := $(CXXFLAGS) $(DEFS)

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
	cp -f $(TARGET) $(OUTPATH)
	rm -f $(LIBPATH)/$(NAME).a
	cp -f $(TARGET) $(LIBPATH)
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 
%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(INCS) -o $(addprefix $(OBJPATH)/, $(notdir $@)) $< 	
	
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
