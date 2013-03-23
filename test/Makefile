CFLAGS = `pkg-config --cflags opencv`
LDLIBS = `pkg-config --libs opencv`

test :main.cpp ipoint.h 
	g++ $(CFLAGS) -o test main.cpp ipoint.h ./libsurf.so $(LDLIBS)
