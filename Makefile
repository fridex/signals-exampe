all: proj1

.PHONY: clean

proj1:
	gcc -Wall -std=c99 proj1.c -pedantic -o proj1

clean:
	rm -f proj1

