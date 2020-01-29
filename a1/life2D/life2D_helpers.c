#include <stdio.h>
#include <stdlib.h>


void print_state(int *board, int num_rows, int num_cols) {
    for (int i = 0; i < num_rows * num_cols; i++) {
        printf("%d", board[i]);
        if (((i + 1) % num_cols) == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void update_state(int *board, int num_rows, int num_cols) {
    // TODO: Implement.
    if ((num_rows > 2) && (num_cols > 2)) { 

        int copied[num_rows * num_cols];
        for (int loop = 0; loop < num_rows * num_cols; loop++) {
            copied[loop] = board[loop];
        }

        for (int i = 0; i < (num_rows * num_cols); i++) {
            if ((i >= num_cols) && (i < num_cols * num_rows - num_cols) && ((i + 1) % num_cols != 0) && ((i + num_cols) % num_cols != 0)) {

		int neighbors = 0;
               
	        if ((board)[i - 1] == 1) { neighbors++; }
		if ((board)[i + 1] == 1) { neighbors++; }
		if ((board)[i - num_cols] == 1) { neighbors++; }
		if ((board)[i + num_cols] == 1) { neighbors++; }
		if ((board)[i - num_cols - 1] == 1) { neighbors++; }
		if ((board)[i - num_cols + 1] == 1) { neighbors++; }
		if ((board)[i + num_cols - 1] == 1) { neighbors++; }
		if ((board)[i + num_cols + 1] == 1) { neighbors++; }

		if (((neighbors < 2) || (neighbors > 3)) && ((board)[i] == 1)) { copied[i] = 0; }
		else if (((neighbors == 2) || (neighbors == 3)) && ((board)[i] == 0)) { copied[i] = 1; }
            }
        }

        for (int loop = 0; loop < num_rows * num_cols; loop++) {
            (board)[loop] = copied[loop];
        }
    }
           
}