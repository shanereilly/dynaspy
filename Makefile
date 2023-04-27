dynaspy: src/dynaspy.c
	gcc src/dynaspy.c -o dynaspy

test: libtestlib.so test/test.c
	gcc -L./ -Wl,-rpath=./ test/test.c -o test_example -ltestlib
libtestlib.so: test/testlib.o
	gcc -shared -o libtestlib.so test/testlib.o
testlib.o: test/testlib.c
	gcc -c -fpic -o test/testlib.o test/testlib.c

man:
	pandoc src/dynaspy.1.md -s -t man -o dynaspy.1
clean:
	rm test/testlib.o


