CC = gcc
CFLAGS = -Wall -Wextra -g
BINDIR = bin
OBJDIR = bin

all : $(BINDIR)/main

$(BINDIR)/main : $(OBJDIR)/main.o $(OBJDIR)/http.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/main.o : main.c http.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/http.o : http.c http.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(OBJDIR) :
	mkdir -p $@

main: $(BINDIR)/main
	./$<

clean :
	rm -rf $(BINDIR) *.o

.PHONY : all clean main
