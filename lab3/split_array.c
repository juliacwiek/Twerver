#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) { 


    /* if size is even, then there are equal numbers of odds and evens (half half)
    if size is odd, then there are floor(size / 2) + 1 evens and floor(size/2) odds */
    int *first_array;
    int *second_array;

    if (length % 2 == 0) { // length of initial array is even
        first_array = (int*) malloc(sizeof(int) * (length / 2)); 
        second_array = (int*) malloc(sizeof(int) * (length / 2));
    }
    
    else { // length of initial array is odd
        first_array = (int*) malloc(sizeof(int) * ((length / 2) + 1));
        second_array = (int*) malloc(sizeof(int) * (length / 2));
    }

    
    int a = 0; 
    int b = 0;
    
    for (int i = 0; i < length; i++) {
        if (i % 2 == 0) { // i is even
            first_array[a] = s[i];
            a++;
        }
        else { // i is odd
            second_array[b] = s[i];
            b++;
        }
    }


    int **pointer = (int**) malloc(sizeof(int**) * 2);
    pointer[0] = first_array;
    pointer[1] = second_array;
    return pointer;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    
    int *new_array = (int*) malloc(sizeof(int) * (size - 1));

    int a = 0;
    for (int i = 1; i < size; i++) {
        int new_int = strtol(strs[i], NULL, 10);
        new_array[a] = new_int;
        a++;
    }

    return new_array;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */

    int *full_array = build_array(argv, argc); 
    int **result = split_array(full_array, argc - 1); 

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}
