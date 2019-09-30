program: synch.c
	gcc -o synch.out synch.c -lpthread

examples: synch_start.c oracle_prod_con.c
	gcc -o synch_start.out synch_start.c -lpthread
	gcc -o oracle_prod_con.out oracle_prod_con.c -lpthread

debug: synch.c
	gcc -o synch.out -g synch.c -lpthread

clean:
	rm -f *.out
