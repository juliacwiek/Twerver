#include <stdio.h>

int main() {
	int number;
	char phone[11];


	scanf("%s %d", &phone[0], &number);
	
        if (number == -1) {
	    printf("%s", phone);
	    return 0;
	}

	else if (number >= 0 && number <= 9) {
	    printf("%c", phone[number]);
	    return 0;
	}

	else if (number < -1 || number > 9) {
	    printf("ERROR");
	    return 1;
	}
  
}

