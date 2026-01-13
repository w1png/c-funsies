CC = cc
CFLAGS = `pkg-config --cflags raylib`
LIBS = `pkg-config --libs raylib` -framework IOKit -framework Cocoa -framework OpenGL

game: game.c
	$(CC) game.c $(CFLAGS) $(LIBS) -o game
