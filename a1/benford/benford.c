#include <stdio.h>
#include <stdlib.h>

#include "benford_helpers.h"

/*
 * The only print statement that you may use in your main function is the following:
 * - printf("%ds: %d\n")
 *
 */
int main(int argc, char **argv) {

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "benford position [datafile]\n");
        return 1;
    }

    // TODO: Implement.
    
    else if (argc == 2) {
    // read from stdin
        int position = strtol(argv[1], NULL, 10);
        int tally[BASE];

        // initialize all elements in tally to 0
        for (int a = 0; a < BASE; a++) {
            tally[a] = 0;
        }

        int number1;
	
	while (scanf("%d\n", &number1) == 1) {
            add_to_tally(number1, position, tally);
        }
	
	for (int c = 0; c < BASE; c++) {
            printf("%ds: %d\n", c, tally[c]);
        }
    }

    else { // reading from a file
        int position = strtol(argv[1], NULL, 10);
        int tally[BASE];

        // initialize all elements in tally to 0
        for (int a = 0; a < BASE; a++) {
            tally[a] = 0;
        }
        int number2;
        FILE *file = fopen(argv[2], "r");

        if (file == NULL) {
            return 1;
        }
	
	while (fscanf(file, "%d", &number2) == 1) {
	    add_to_tally(number2, position, tally);
	}

        for (int c = 0; c < BASE; c++) {
            printf("%ds: %d\n", c, tally[c]);
        }

        if (fclose(file) != 0) {
            return 1;
        }

    }

    return 0;
}