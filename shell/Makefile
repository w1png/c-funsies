CC = gcc
CFLAGS = -Wall -Wextra -g
BINDIR = bin
OBJDIR = bin

all: $(BINDIR)/main

$(BINDIR)/main: $(OBJDIR)/main.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/main.o: main.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(OBJDIR):
	mkdir -p $@

main: $(BINDIR)/main
	./$<

clean:
	rm -rf $(BINDIR) *.o

.PHONY: all main clean
