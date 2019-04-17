CXX = g++
CXXFLAGS = -ggdb -Wall -std=c++11

TARGET_LIB = libproto.a
SOURCE_OBJ = uno.pb.o

$(TARGET_LIB): makeproto $(SOURCE_OBJ)
	ar -r $(TARGET_LIB) $(SOURCE_OBJ)
	mv $(TARGET_LIB) ../lib

.PHONY: clean makeproto
makeproto:
	protoc -I=./ --cpp_out=./ *.proto

clean:
	rm -rf ./*.o

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.cc %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@ 

