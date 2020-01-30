CC = /usr/bin/g++

#OPENGL LIBS FOR LINUX
GLLIB :=  -lGL -lGLEW -lGLU -lglut
#OPENGL LIBS FOR MAC
#GLLIB := -framework OpenGL -framework GLUT
NANOGUI := -L./nanogui -lnanogui
MTHREAD := -lpthread

#COMPILER FLAGS
CCFLAGS :=

#include directories
#should include gl.h glut.h etc...
INCDIR := -I/usr/include -I./nanogui/include -I./nanogui/ext/eigen -I./nanogui/ext/glfw/include -I./nanogui/ext/nanovg/src -I./nanogui/ext/glfw/include
LDLIBS := $(GLLIB) $(NANOGUI) $(MTHREAD)

TARGET = glutdemo elements drawer controller
OBJS = demo.o elements.o drawer.o controller.o


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC)  $^ $(CCFLAGS) $(LDLIBS)  -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -o $@ -c $(LDLIBS) $(INCDIR) $<

clean:
	rm -f $(OBJS) $(TARGET)

