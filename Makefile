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
	cp .config/$(NAME).desktop build/
	cp README.md build/
	gzip -cn man/$(NAME).1 > build/$(NAME).1.gz

# Make the installation
install:
	# Create directories if they don't exist
	mkdir $(TARGET)/usr/bin || true
	mkdir $(TARGET)/usr/share/applications || true
	mkdir $(TARGET)/usr/share/doc || true
	mkdir $(TARGET)/usr/share/man/man1 || true
	# Install binary
	install -Dm 755 build/$(NAME) -t $(TARGET)/usr/bin
	# Install desktop entry
	install -Dm 644 build/$(NAME).desktop -t $(TARGET)/usr/share/applications
	# Install README.md
	install -Dm 644 README.md -t $(TARGET)/usr/share/doc
	# Install man page
	install -Dm 644 build/$(NAME).1.gz -t $(TARGET)/usr/share/man/man1

# Clean
clean:
	rm -rf build
