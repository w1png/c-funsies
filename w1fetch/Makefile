# MAKEFILE IS VIBECODED

CC      := gcc
CFLAGS  := -Wall -Wextra -g -pedantic -std=c11
CPPFLAGS:= -Ilib
LDFLAGS :=

PREFIX  := /usr/local
BINDIR  := $(PREFIX)/bin

PROGRAM := w1fetch
BINDIR_BUILD := bin
SOURCES := main.c
OBJECTS := $(BINDIR_BUILD)/$(SOURCES:.c=.o)
TARGET  := $(BINDIR_BUILD)/$(PROGRAM)

all: $(TARGET)

$(TARGET): $(OBJECTS) | $(BINDIR_BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDFLAGS)

$(BINDIR_BUILD)/%.o: %.c lib/shared.h | $(BINDIR_BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BINDIR_BUILD):
	mkdir -p $(BINDIR_BUILD)

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(PROGRAM)
	@echo "Installed"

uninstall:
	rm -f $(BINDIR)/$(PROGRAM)
	@echo "Uninstalled"

clean:
	rm -rf $(BINDIR_BUILD)

.PHONY: all clean run install uninstall
