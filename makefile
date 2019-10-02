program: synch.c
	gcc -Wall -o synch.out synch.c -lpthread

examples: synch_start.c oracle_prod_con.c
	gcc -o synch_start.out synch_start.c -lpthread
	gcc -o oracle_prod_con.out oracle_prod_con.c -lpthread

debug: synch.c
	gcc -Wall -o synch.out -g synch.c -lpthread

run: synch.out
	./synch.out CCPPCPCP

memrun: synch.out
	valgrind --tool=helgrind ./synch.out PPCCPPCC

freerun: synch.out
	valgrind ./synch.out CPCPCPCPCPCPCPCP

clean:
	rm -f *.out
