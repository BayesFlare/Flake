CPPFLAGS = -I/usr/local/include/dnest3 -I.. -I/usr/local
CFLAGS = -m64 -Ofast -flto -march=native -funroll-loops -Wall -Wextra -ansi -pedantic
LIBS =  -L/usr/local/lib -L.. -L/usr/lib/x86_64-linux-gnu -lrjobject -ldnest3 -lgsl -lgslcblas -lboost_system -lboost_thread -lcfitsio -lCCfits

default:
	g++ $(CFLAGS) $(CPPFLAGS) -c *.cpp
	g++ -o flake *.o $(LIBS)
	rm -f *.o

