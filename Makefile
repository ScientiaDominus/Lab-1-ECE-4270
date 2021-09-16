mu-mips: mu-mips.c
	gcc -Wall -g -O2 $^ -o $@ -lm

.PHONY: clean
clean:
	rm -rf *.o *~ mu-mips
