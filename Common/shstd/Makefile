
CXX=g++
CFLAG = -g -D__DEBUG_ -Wall
SHSTDLIB=libshstd.so
SHSTDSLIB=libshstd.a


SHSTD=shstd
HEADS = $(wildcard ./include/*.h)
HEADS += $(wildcard ./src/*.h)
CPPS = $(wildcard ./src/*.cpp)
CPPS := $(filter-out ./src/version.cpp, $(CPPS))  
CPPS += ./src/version.cpp
OBJS = $(CPPS:.cpp=.o)

.PHONY: all
all:clean $(SHSTD) test

build: $(SHSTD)

$(SHSTD): $(OBJS) $(CPPS) $(HEADS)
	mkdir -p ./lib
	$(CXX) $(INC)  $(OBJS) -fPIC -shared -fPIC -o ./lib/$(SHSTDLIB)
	ar  -crv ./lib/$(SHSTDSLIB) $(OBJS)
	#tar -zcf ./lib/forjava.shstd.tgz ./include/rmconf.h ./lib/$(SHSTDLIB)
	rm -f ./lib/$(SHSTDLIB)
	date

$(OBJS):%.o:%.cpp $(HEADS)
	$(CXX) $(INC) $(CFLAG) -fPIC -c $< -o $@
	
test:
	g++ -g ./src/main.c ./lib/libshstd.a -lpthread -o ./etest
	
clean:
	rm -f $(OBJS) ./lib/$(SHSTDLIB) ./lib/$(SHSTDSLIB) 

full:clean $(SHSTD) test 


