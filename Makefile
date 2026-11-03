CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

all:
	$(CC) $(CFLAGS) src/main.c src/game.c -o game_of_life.exe

run: all
	./game_of_life.exe

clean:
	del /f /q game_of_life.exe 2>nul || rm -f game_of_life.exe