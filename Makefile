.PHONY: run forth

CFLAGS=-Wall -Werror -Wpedantic -O3

run: forth
	./forth

forth: forth.c
	gcc $(CFLAGS) -o $@ $<
