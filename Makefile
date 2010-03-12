# Makefile

CXX = g++
CC =  gcc
OBJS = bin.o pgm.o main.o
SRC = $(OBJ:.o=.cc)

# define fourth window if your graphics card support 3d texture
DWIN4 = -D_DWIN4

INCLUDES = -I$(HOME)/lcgtk

LIBDIRS = -L$(HOME)/lcgtk/lib

# Windows
#EXECUTABLE = image3dViewer.exe
#RM =	del
#LIBS =	-lGLee -lglut32 -lopengl32 -mwindows \
#	-D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread	

#Linux
EXECUTABLE = image3dviewer
RM = 	rm -f
LIBS =	-lglut -lGL -lGLU -lGLee -lXext -lXmu -lX11 -lm -lXi \
	-D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread	

CXXFLAGS =	-g $(INCLUDES) -Wall -Wno-deprecated $(DWIN4) \
#			-D_GLIBCXX_GTHREAD_USE_WEAK=0 -pthread
#			-D_BSD_SOURCE -fexpensive-optimizations

#--------------------------------------------------------------------

($EXECUTABLE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) $(OBJS) $(LIBDIRS) $(LIBS)

depend:
	$(CC) $(CXXFLAGS) -M *.cc > .depend

clean:
	$(RM) $(EXECUTABLE) *.o *~ .depend

all: clean depend ($EXECUTABLE)

ifeq (.depend,$(wildcard .depend))
include .depend
endif

