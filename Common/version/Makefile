
CXX=g++
CFLAG = -g -D__DEBUG_ -Wall
SHSTDLIB=libcomVer.so
SHSTDSLIB=libcomVer.a


SHSTD=comVer
HEADS = $(wildcard ./include/*.h)
HEADS += $(wildcard ./*.h)
CPPS = $(wildcard ./*.cpp)
CPPS := $(filter-out ./version.cpp, $(CPPS))  
CPPS += ./version.cpp
OBJS = $(CPPS:.cpp=.o)

.PHONY: all
all:clean $(SHSTD) 

build: $(SHSTD)

$(SHSTD): $(OBJS) $(CPPS) $(HEADS)
	mkdir -p ./lib
	$(CXX) $(INC)  $(OBJS) -fPIC -shared -fPIC -o ./lib/$(SHSTDLIB)
	ar  -crv ./lib/$(SHSTDSLIB) $(OBJS)
	rm -f ./lib/$(SHSTDLIB)
	date
	
$(OBJS):%.o:%.cpp $(HEADS)
	$(CXX) $(INC) $(CFLAG) -fPIC -c $< -o $@
	
clean:
	rm -f $(OBJS) ./lib/$(SHSTDLIB) ./lib/$(SHSTDSLIB) 

full:clean $(SHSTD) 


