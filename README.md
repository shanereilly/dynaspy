# README

## Compilation instructions
Tested on 64-bit Kali Linux, kernel version 6.1.0.

To compile, simply run:

```
make
```

To compile the example test file, run:

```
make test
```

This will create a simple test file called `test_example` which loads an example shared library called `libtestlib.so`.

```
$ ./dynaspy ./test_example 1> /dev/null
Dynamic library opened: ./libtestlib.so
Dynamic library opened: /lib/x86_64-linux-gnu/libc.so.6
```

## Man page
For information on how `dynaspy` works, please refer to the man page.

To view man page, run:

```
man ./dynaspy.1
```
