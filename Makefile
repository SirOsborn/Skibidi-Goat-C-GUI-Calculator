CC = gcc
CFLAGS = -Wall -Wextra -O2 -g $(shell pkg-config --cflags gtk4) -I/usr/include/gtk-4.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/graphene-1.0
LDFLAGS = $(shell pkg-config --libs gtk4) -lm

SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = calculator

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS) 