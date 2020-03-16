#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

#include "point.h"
#include "serial_closest.h"
#include "utilities_closest.h"

void *malloc_wrapper(int size) {
	void *ptr;
	if ((ptr = malloc(size)) == NULL) {
		perror("malloc");
		exit(1);
	}
	return ptr;
}

int fork_wrapper() {
	int res;
	if ((res = fork()) == -1) {
		perror("fork");
		exit(1);
	}
	return res;
}

void pipe_wrapper(int *fd) {
	if (pipe(fd) == -1) {
		perror("pipe");
		exit(1);
	}
}

void close_reading_wrapper(int *fd) {
	if (close(fd[0]) == -1) {
        perror("close");
        exit(1);
    }
}

void close_writing_wrapper(int *fd) {
	if (close(fd[1]) == -1) {
        perror("close");
        exit(1);
    }
}
/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {

	int count = 0;

    // base case
    if ((n < 4) || (pdmax == 0)) { 
    	return closest_serial(p, n); 
    }

    // find middle index
    double mid = n / 2;

    // size of left half array and right half array
    int left_size;
	int right_size;

	// if size of original array is even
	if (n % 2 == 0) {
		left_size = n / 2;
		right_size = n / 2;

	// if size of original array is odd
	} else {
		left_size = n / 2;
		right_size = (n / 2) + 1;
	}

    // split original array into two halves
    struct Point leftHalf[left_size];
    struct Point rightHalf[right_size];
 
    // initialize closest distance variables for each list
    double closest_1; // will be for left array
    double closest_2; // will be for right array
    
    // make left half array
    for (int i = 0; i < left_size; i++) {
        leftHalf[i] = p[i];
    }
    
    // make right half array
    int index = 0;
    for (int i = left_size; i < n; i++) {
	   rightHalf[index] = p[i];
	   index++;
    }

    // create one child process w/ pipe (for the LEFT half array)
    int fd_1[2], result_1;

    // call pipe and error check
    pipe_wrapper(fd_1);

    // call fork
    result_1 = fork_wrapper();

    if (result_1 < 0) {   // case: a system call error
        // handle the error
        perror("fork");
        exit(1);
    } 

    else if (result_1 == 0) {  // case: a child process
     
        // we can close fd_1[0], reading end of child
        close_reading_wrapper(fd_1);
        
        // recurse to get closest distance on the left half of the array
        double l_ret = closest_parallel(leftHalf, left_size, pdmax - 1, pcount);

        // write l_ret to the pipe as a double
        if (write(fd_1[1], &l_ret, sizeof(double)) != sizeof(double)) {
            perror("write from child to pipe");
            exit(1);
        }
 		
        // close writing end
        close_writing_wrapper(fd_1);

        // exit with number of workers
        exit(*pcount); 
    }

    // if parent process
    else {

        // close writing end
        close_writing_wrapper(fd_1);

        int status_1;

        if (wait(&status_1) == -1) {
            perror("wait");
            exit(1);
        } else {
            if (WIFEXITED(status_1)){
        		int es = WEXITSTATUS(status_1);
        		count += es + 1;
      	    }
        }

		// write to reading end of pipe (takes in value from child)
        if (read(fd_1[0], &closest_1, sizeof(double)) != sizeof(double)) {
            perror("reading from pipe from a child");
            exit(1);
        }

        // close reading end of pipe
    	close_reading_wrapper(fd_1);
    } 

    // create one child process w/ pipe (for the RIGHT half array)
    int fd_2[2], result_2;

    // call pipe and error check
    pipe_wrapper(fd_2);

    // call fork
    result_2 = fork_wrapper(fd_2);

    if (result_2 < 0) {   // case: a system call error
        // handle the error
        perror("fork");
        exit(1);
    } 

    else if (result_2 == 0) {  // case: a child process

        // close reading end, fd_2[0], of child
        close_reading_wrapper(fd_2);
        
        // get closest value in right half array
        double r_ret = closest_parallel(rightHalf, right_size, pdmax - 1, pcount);
     
        // write r_ret to the pipe as a double
        if (write(fd_2[1], &r_ret, sizeof(double)) != sizeof(double)) {
            perror("write from child to pipe");
            exit(1);
        }

        // close writing end
        close_writing_wrapper(fd_2);

        // exit with number of workers
        exit(*pcount); 
    }

    // if parent process
    else {

        // close writing end
        close_writing_wrapper(fd_2);

        int status_2;
        if (wait(&status_2) == -1) {
            perror("wait");
            exit(1);
        } else {
            if (WIFEXITED(status_2)){
        		int es = WEXITSTATUS(status_2);
        		count += es + 1;
      	    }
        }

		// write to reading end
        if (read(fd_2[0], &closest_2, sizeof(double)) != sizeof(double)) {
            perror("reading from pipe from a child");
            exit(1);
        }

        // close reading end of the pipe
    	close_reading_wrapper(fd_2);
    }

	struct Point mid_point = p[(int) mid];

     // Find the smaller of two distances
    double d = min(closest_1, closest_2);

    // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point
    struct Point *strip = malloc_wrapper(sizeof(struct Point) * n);

    int k = 0;
    for (int l = 0; l < n; l++) {
        if (abs(p[l].x - mid_point.x) < d) {
            strip[k] = p[l], k++;
        }
    }

    double new_min = min(d, strip_closest(strip, k, d));

    //free all alocated memory
    free(strip);

    *pcount += count;
    return new_min;   
}

