testprog: libtestlib.so test.c
	gcc -L./ -Wl,-rpath=./ test.c -o testprog -ltestlib
libtestlib.so: testlib.o
	gcc -shared -o libtestlib.so testlib.o
testlib.o: testlib.c
	gcc -c -fpic -o testlib.o testlib.c

clean:
	rm testlib.o

	
