dynaspy: src/dynaspy.c
	gcc src/dynaspy.c -o dynaspy

testprog: libtestlib.so test/test.c
	gcc -L./ -Wl,-rpath=./ test/test.c -o testprog -ltestlib
libtestlib.so: test/testlib.o
	gcc -shared -o libtestlib.so test/testlib.o
testlib.o: test/testlib.c
	gcc -c -fpic -o test/testlib.o test/testlib.c

clean:
	rm test/testlib.o


