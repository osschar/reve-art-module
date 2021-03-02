all: test

test: MockEventDisplayManager.cc test_evd.cpp Dict.cc
	c++ `root-config --cflags` -L`root-config --libdir` MockEventDisplayManager.cc Dict.cc test_evd.cpp -lROOTEve -lROOTWebDisplay -lCore -g -O0 -o test

Dict.cc: MockEventDisplayManager.h LinkDef.h
	rootcling -f Dict.cc MockEventDisplayManager.h  LinkDef.h

clean:
	rm -f Dict* test
