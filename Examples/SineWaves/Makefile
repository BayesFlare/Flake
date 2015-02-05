CPPFLAGS = -I/usr/local/include/dnest3 -I../..
CFLAGS = -m64 -Ofast -flto -march=native -funroll-loops -Wall -Wextra -ansi -pedantic
LIBS =  -L/usr/local/lib -L../.. -lrjobject -ldnest3 -lgsl -lgslcblas -lboost_system -lboost_thread

default:
	g++ $(CFLAGS) $(CPPFLAGS) -c *.cpp
	g++ -o main *.o $(LIBS)
	rm -f *.o

