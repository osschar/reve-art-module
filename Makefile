all: test

libMockEventDisplayManager.so: MockEventDisplayManager.o Dict.o
	c++ -shared -pthread $^ -o $@

test: libMockEventDisplayManager.so test_evd.o
	c++ `root-config --cflags` test_evd.o -L. -lMockEventDisplayManager  -L`root-config --libdir` -lROOTEve -lROOTWebDisplay -lCore -lMathCore -g -O0 -o test

%.o: %.cc
	c++  `root-config --cflags` -fPIC -c $^ -o $@

%.o: %.cpp
	c++  `root-config --cflags` -fPIC -c $^ -o $@

Dict.cc: MockEventDisplayManager.h LinkDef.h
	rootcling -f Dict.cc MockEventDisplayManager.h  LinkDef.h

clean:
	rm -f *.o *.so
	rm -f Dict* test
