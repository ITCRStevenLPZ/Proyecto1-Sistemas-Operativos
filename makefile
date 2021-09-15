#Proyecto1 SO (Ronald Esquivel, Ricardo Murillo y Dylan Gonzalez)
OBJECTS=puenteEstrecho.o
OUTPUT=puenteEstrecho

#CFLAGS=-I/usr/local/Mesa-3.4/include
LDLIBS=-lpthread -lrt -lm
#LDFLAGS=-L/usr/local/Mesa-3.4/lib -L/usr/X11R6/lib

$(OUTPUT): $(OBJECTS)
	cc $(OBJECTS) -o $(OUTPUT)  $(LDLIBS)

#$(CFLAGS) $(LDFLAGS)
$(OBJECTS):puenteEstrecho.h

clean:
	rm -f *.o
	rm -f puenteEstrecho

#LDLIBS=-lX11 -lglut -lGLU -lGL -lm -lXext -lpthread -lrt