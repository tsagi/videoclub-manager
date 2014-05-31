main: main.c
	gcc main.c -lncurses -o main -I.
	
clean:
	rm -rf main
