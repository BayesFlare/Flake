CPPFLAGS = -I/usr/local/include/dnest3
CFLAGS = -m64 -Ofast -flto -march=native -funroll-loops -fPIC -Wall -Wextra -ansi -pedantic -DNDEBUG
LIBS = -L/usr/local/lib -ldnest3 -lgsl -lgslcblas -lboost_thread -lboost_system

default:
	g++ $(CFLAGS) $(CPPFLAGS) -c *.cpp Distributions/*.cpp
	g++ -o main *.o $(LIBS)
	g++ -shared -o librjobject.so BasicCircular.o  ClassicMassInf.o  Distribution.o Pareto.o
	rm -f *.o

clean:
	rm -f *.o
	rm -f main
