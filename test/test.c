#include <stdio.h>
#include <stdlib.h>
#include "testlib.h"

int main(int argc, char **argv)
{
	printf("This is a print from inside the program.\n");
	testlib();
	return(EXIT_SUCCESS);
}
