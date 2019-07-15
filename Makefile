CPPFLAGS = -std=c++17
LIBS = -lEGL \
	-lGL \
	-lopencv_core \
	-lopencv_imgcodecs

all: egltest

egltest: egltest.o openglcontext.o gl46.o
	g++ -o $@ $^ $(LIBS)

egltest.o: main.cpp
	g++ $(CPPFLAGS) -o $@ -c $<
openglcontext.o: openglcontext.cpp
	g++ $(CPPFLAGS) -o $@ -c $<
gl46.o: gl46.c
	gcc -o $@ -c $<

clean:
	rm -f *.o egltest
