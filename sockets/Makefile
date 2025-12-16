CC = gcc
CFLAGS = -Wall -Wextra -g
BINDIR = bin
OBJDIR = bin

all: $(BINDIR)/server $(BINDIR)/client

$(BINDIR)/server: $(OBJDIR)/server.o $(OBJDIR)/shared.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/client: $(OBJDIR)/client.o $(OBJDIR)/shared.o | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/server.o: server.c shared.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/client.o: client.c shared.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/shared.o: shared.c shared.h | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(OBJDIR):
	mkdir -p $@

server: $(BINDIR)/server
	./$<

client: $(BINDIR)/client
	./$<

clean:
	rm -rf $(BINDIR) *.o

.PHONY: all server client clean
