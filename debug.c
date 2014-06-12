

#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include "machine.h"

bool debug_ask(Machine *pmach) {
	bool debug_mode = true;
	char buff[2];
	while (debug_mode) {
		printf("DEBUG?");
		fflush(stdin)
		fgets(buff, 2, stdin);
		int c;
		while ((c=getchar()) != '\n' && c != EOF);
			switch (buff[0]) {
			case 'h':
				printf("Available commands:\n");
				printf("\th\thelp\n");
				printf("\tc\tcontinue (exit interactive debug mode)\n");
				printf("\ts\tstep by step (next instruction)\n");
				printf("\tRET\tstep by step (next instruction)\n");
				printf("\tr\tprint registers\n");
				printf("\td\tprint data memory\n");
				printf("\tt\tprint text (program) memory\n");
				printf("\tp\tprint text (program) memory\n");
				printf("\tm\tprint registers and data memory\n");
				break;
			case 'c':
				return false;
				break;
			case '\n':
			case 's':
				return true;
				break;
			case 'r':
				print_cpu(pmach);
				break;
			case 'd':
				print_data(pmach);
				break;
			case 't':
				print_program(pmach);
				break;
			case 'p':
				print_program(pmach);
				break;
			case 'm':
				print_cpu(pmach);
				print_data(pmach);
				break;
			}

	}
	return false;
}