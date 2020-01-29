#include <stdio.h>

#include "benford_helpers.h"

int count_digits(int num) {
    // TODO: Implement.
    int num_divisions = 0;
 
    if ((num == 0) || (num == 1)) {
        return 1;
    }

    int a = num;
    while (a >= 1) {
        a = a / BASE;
        num_divisions += 1;
    }

    return num_divisions;

}

int get_ith_from_right(int num, int i) {
    // TODO: Implement.
    
    int digits[count_digits(num)]; // digits in this array are stored in reverse
    int p = 0;
    while (num) {
        digits[p] = num % BASE;
        num = num / BASE;
        p++;
    }

    return digits[i];   
}

int get_ith_from_left(int num, int i) {
    // TODO: Implement.
    int p = 0;
    int size = count_digits(num);
    int digits[size]; // digits in this array are stored in reverse
    while (num) {
        digits[p] = num % BASE;
        num = num / BASE;
        p++;
    }

    return digits[size - i - 1];
}

void add_to_tally(int num, int i, int *tally) {
    // TODO: Implement.
    int left = get_ith_from_left(num, i);
    tally[left]++;
}
