CXX = g++
CXXFLAGS = -ggdb -Wall -std=c++11

TARGET_BIN = uno
SOURCE_OBJ = game.o main.o mysql_db.o player.o table.o

LIBS = -L./lib -lws -lmylog -lproto -lprotobuf -lpthread
MYSQL_CONFIG = $(shell mysql_config --libs)

$(TARGET_BIN): $(SOURCE_OBJ)
	g++ -o $@ $(SOURCE_OBJ) $(LIBS) $(MYSQL_CONFIG)  
	#mv $(TARGET_BIN) ../

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@ 

#mysql_db.o: mysql_db.cc mysql_db.h
#$(CXX) -c $(CXXFLAGS) $(MYSQL_FLAGS) $< -o $@

.PHONY: clean all
clean:
	rm -rf ./*.o

all:
	#编译日志库
	cd ./log; make clean; make
	@echo
	#编译proto库
	cd ./proto; make clean; make
	@echo
	#编译WebSocket
	cd ./web_socket; make clean; make
	@echo
	#编译主程序
	make clean; make
