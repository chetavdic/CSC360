all: mts

mts: mts.c queue.c
	gcc mts.c queue.c -lreadline -lhistory -ltermcap -lpthread -o mts



clean:
	rm -f *.o
	rm -f mts
