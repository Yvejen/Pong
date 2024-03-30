
Pong: main.o
	cc -lSDL2 -lm -o Pong main.o

main.o : main.c
	cc -Wall -c main.c -o main.o

.PHONY: clean
clean:
	rm Pong main.o
