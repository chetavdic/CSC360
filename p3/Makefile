all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c diskfunctions.c
	gcc diskinfo.c diskfunctions.c -lreadline -lhistory -ltermcap -lpthread -o diskinfo

disklist: disklist.c diskfunctions.c
	gcc disklist.c diskfunctions.c -lreadline -lhistory -ltermcap -lpthread -o disklist

diskget: diskget.c diskfunctions.c
	gcc diskget.c diskfunctions.c -lreadline -lhistory -ltermcap -lpthread -o diskget

diskput: diskput.c diskfunctions.c
	gcc diskput.c diskfunctions.c -lreadline -lhistory -ltermcap -lpthread -o diskput 



clean:
	rm -f *.o
	rm -f diskinfo
	rm -f disklist
	rm -f diskget
	rm -f diskput
