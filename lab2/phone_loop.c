#include <stdio.h>

int main() {
	char phone[11];
	int number;
	int number_of_errors = 0;

	scanf("%s", &phone[0]);
	
	while (scanf("%d", &number) == 1) {
		if (number == -1) {
		    printf("%s\n", phone);
		}

		else if (number >= 0 && number <= 9) {
			printf("%c\n", phone[number]);
		}

		else if (number < -1 || number > 9) {
			printf("ERROR\n");
			number_of_errors++;
		}
	
	}

if (number_of_errors > 0) {
    return 1; 

}
return 0;

}
