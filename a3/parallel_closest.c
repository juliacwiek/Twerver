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


/*
 * Multi-process (parallel) implementation of the recursive divide-and-conquer
 * algorithm to find the minimal distance between any two pair of points in p[].
 * Assumes that the array p[] is sorted according to x coordinate.
 */
double closest_parallel(struct Point *p, int n, int pdmax, int *pcount) {

	//static int workers_1 = 0;
	//static int workers_2 = 0;

    // base case
    if ((n < 4) || (pdmax == 0)) {
    	double a = closest_serial(p, n);
    	//printf("BASE CASE: %f\n", a); 
    	return a;
    }

    //for (int k = 0; k < n; k++) {
    //	printf("(%d, %d) ", p[k].x, p[k].y);
    //}
    //printf("\n");

    // find middle index
    double mid = n / 2;
    int left_size;
	int right_size;

	// if size of list is even
	if (n % 2 == 0) {
		left_size = n / 2;
		right_size = n / 2;

	// if size of list is odd
	} else {
		left_size = n / 2;
		right_size = (n / 2) + 1;
	}

    //printf("n: %d mid: %f floor(mid): %d ciel(mid): %d\n", n, mid, left_size, right_size);

    // split array into two halves
    struct Point leftHalf[left_size];
    struct Point rightHalf[right_size];
 
    // initialize closest distance variables for each list
    double closest_1;
    double closest_2;
    
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

    //for (int l = 0; l < left_size; l++) {
    //	printf("(%d, %d) ", leftHalf[l].x, leftHalf[l].y);
    //}
    //printf("\n");
    //for (int m = 0; m < right_size; m++) {
    //	printf("(%d, %d) ", rightHalf[m].x, rightHalf[m].y);
    //}
    //printf("\n");

    //int workers_1;
    //int workers_2;

    // create one child process w/ pipe (this pipe is for the LEFT half array)
    int fd_1[2], result_1;

    if (pipe(fd_1) == -1) {
        perror("pipe");
        exit(1);
    }

    // call fork
    if ((result_1 = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (result_1 < 0) {   // case: a system call error
        // handle the error
        perror("fork");
        exit(1);
    } 

    else if (result_1 == 0) {  // case: a child process
    	//workers_1++;
    	//(*pcount) = workers_1;
    	//printf("CHILD: workers: %d pcount: %d\n", workers_1, *pcount);

        // child does their work here
        // we can close fd[0], reading end of child
        if (close(fd_1[0]) == -1) {
            perror("close");
            exit(1);
        }
        
        // recurse to get closest distance on the left half of the array
        double l_ret = closest_parallel(leftHalf, left_size, pdmax - 1, pcount);

        //printf("closest is %f of closest_1\n", closest_1);

        // write l_ret to the pipe as a double
        if (write(fd_1[1], &l_ret, sizeof(double)) != sizeof(double)) {
            perror("write from child to pipe");
            exit(1);
        }
 		
        // close writing end
        if (close(fd_1[1]) == -1) {
            perror("close");
            exit(1);
        }

      


        exit(*pcount); // CHANGE EXIT VALUE

    }

    // if parent process
    else {

    	//(*pcount)++;
    	//printf("PARENT: workers: %d pcount: %d\n", workers_1 + workers_2, *pcount);
    	//workers++;
        // close writing end
        if (close(fd_1[1]) == -1) {
            perror("close");
            exit(1);
        }

        int status_1;

        if (wait(&status_1) == -1) {
            perror("wait");
            exit(1);
        } else {
            if (WIFEXITED(status_1)){
        		int es = WEXITSTATUS(status_1);
        		*pcount = es + 1;
      	    }
        }

		// write to reading end
        if (read(fd_1[0], &closest_1, sizeof(double)) != sizeof(double)) {
            perror("reading from pipe from a child");
            exit(1);
        }

        
        // close reading end of the pipe
    	if (close(fd_1[0]) == -1) {
        	perror("close");
        	exit(1);
    	}

        
    } 

    

    // create one child process w/ pipe (this pipe is for the RIGHT half array)
    int fd_2[2], result_2;

    if (pipe(fd_2) == -1) {
        perror("pipe");
        exit(1);
    }
    // call fork
    if ((result_2 = fork()) == -1) {
        perror("fork");
        exit(1);
    }

    if (result_2 < 0) {   // case: a system call error
        // handle the error
        perror("fork");
        exit(1);
    } 

    else if (result_2 == 0) {  // case: a child process
    	
    	
    	//printf("CHILD: workers: %d pcount: %d\n", workers_2, *pcount);

        //(*pcount) += 1;

        //printf("pcount %d\n", *pcount);

        // child does their work here
        // we can now close fd[0], reading end
        if (close(fd_2[0]) == -1) {
            perror("close");
            exit(1);
        }
        
        // check if 
        double r_ret = closest_parallel(rightHalf, right_size, pdmax - 1, pcount);
     
        // write closest to the pipe as a double
        if (write(fd_2[1], &r_ret, sizeof(double)) != sizeof(double)) {
            perror("write from child to pipe");
            exit(1);
        }


        // close writing end
        if (close(fd_2[1]) == -1) {
            perror("close");
            exit(1);
        }

        // exit so I don't fork my own children on next loop iteration
        exit(*pcount); // CHANGE EXIT VALUE

    }

    // if parent process
    else {
    	//(*pcount)++;
    	//printf("PARENT: workers: %d pcount: %d\n", workers_1 + workers_2, *pcount);

        // close writing end
        if (close(fd_2[1]) == -1) {
            perror("close");
            exit(1);
        }

        int status_2;
        if (wait(&status_2) == -1) {
            perror("wait");
            exit(1);
        } else {
            if (WIFEXITED(status_2)){
        		
        		int es = WEXITSTATUS(status_2);
        		*pcount = es + 1;
      	    
      	    }
        }

		// write to reading end
        if (read(fd_2[0], &closest_2, sizeof(double)) != sizeof(double)) {
            perror("reading from pipe from a child");
            exit(1);
        }

        // close reading end of the pipe
    	if (close(fd_2[0]) == -1) {
        	perror("close");
        	exit(1);
    	}



    }

    
	struct Point mid_point = p[(int) mid];

     // Find the smaller of two distances 
    double d = min(closest_1, closest_2);

    // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point.
    struct Point *strip = malloc(sizeof(struct Point) * n);
    if (strip == NULL) {
        perror("malloc");
        exit(1);
    }

    int k = 0;
    for (int l = 0; l < n; l++) {
        if (abs(p[l].x - mid_point.x) < d) {
            strip[k] = p[l], k++;
        }
    }

    //printf("MAIN PARENT: WORKERS: %d, pcount %d\n", workers_1 + workers_2, *pcount);
    // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
    //(*pcount) = workers_1 + workers_2;
    double new_min = min(d, strip_closest(strip, k, d));

    //free all alocated memory
    free(strip);
    return new_min;   
}

