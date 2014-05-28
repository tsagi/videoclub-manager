main: main.c
	gcc main.c -o main -I.
	
clean:
	rm -rf main
