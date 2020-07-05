# Project & compiler information
NAME=kermit
CFLAGS=-s -O3 -Wall -Wno-deprecated-declarations $(shell pkg-config --cflags vte-2.91)
LIBS=$(shell pkg-config --libs vte-2.91)
CC=gcc
all: clean build
# Build the project
build:
	mkdir build || true
	$(CC) $(CFLAGS) src/$(NAME).c -o build/$(NAME) $(LIBS)
	cp -prf .config/$(NAME).desktop build/$(NAME).desktop
# Make the installation
install:
	# Create directories if doesn't exist
	mkdir $(TARGET)/usr/bin || true
	mkdir $(TARGET)/usr/share/applications || true
	# Install
	install build/$(NAME) $(TARGET)/usr/bin/$(NAME)
	install build/$(NAME).desktop $(TARGET)/usr/share/applications/$(NAME).desktop
# Clean
clean:
	rm -rf build
