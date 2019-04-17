CXX = g++
CXXFLAGS = -ggdb -Wall -std=c++11

TARGET_LIB = libmylog.a
SOURCE_OBJ = log.o

$(TARGET_LIB): $(SOURCE_OBJ)
	ar -r $(TARGET_LIB) $(SOURCE_OBJ)
	mv $(TARGET_LIB) ../lib

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o: %.cc %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@ 

.PHONY: clean
clean:
	rm -rf ./*.o
