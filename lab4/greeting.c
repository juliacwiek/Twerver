#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
    This program has two arguments: the first is a greeting message, and the
    second is a name.

    The message is an impersonal greeting, such as "Hi" or "Good morning".
    name is set to refer to a string holding a friend's name, such as
    "Emmanuel" or "Xiao".

    First copy the first argument to the array greeting. (Make sure it is
    properly null-terminated.)

    Write code to personalize the greeting string by appending a space and
    then the string pointed to by name.
    So, in the first example, greeting should be set to "Hi Emmanuel", and
    in the second it should be "Good morning Xiao".

    If there is not enough space in greeting, the resulting greeting should be
    truncated, but still needs to hold a proper string with a null terminator.
    For example, "Good morning" and "Emmanuel" should result in greeting
    having the value "Good morning Emmanu" and "Top of the morning to you" and
    "Patrick" should result in greeting having the value "Top of the morning ".

    Do not make changes to the code we have provided other than to add your 
    code where indicated.
*/

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: greeting message name\n");
        exit(1);
    }
    char greeting[20];
    char *name = argv[2];

    // Your code goes here
    //printf("length of argv 1 is %ld\n", strlen(argv[1]));
    // first copy greeting message into greeting
    if (strlen(argv[1]) >= 19) {
        strncpy(greeting, argv[1], 19);
        greeting[19] = '\0';
        //printf("length of greeting array is %ld\n", strlen(greeting));

    }

    else { // argv[1] is less than 19
        strcpy(greeting, argv[1]);
        greeting[strlen(argv[1])] = ' '; // thus, will have room for the space
        greeting[strlen(argv[1]) + 1] = '\0'; 
        //printf("length of greeting array is is %ld\n", strlen(greeting));
        
    }

    if ((strlen(greeting) + strlen(name)) <= 19) { 
        strcat(greeting, name);
        //printf("Size of new greeting array after adding name is %ld\n", strlen(greeting));
        //greeting[strlen(greeting)] = '\0';
    }

    else if (strlen(greeting) < 19) { // greeting is less than 19 (so has a space but can still concatenate stuff) but greeting + name is > 19
        strncat(greeting, name, 19 - strlen(greeting));
        //printf("size of greeting array is %ld\n", strlen(greeting));
        //greeting[19] = '\0'; 
    }        


    printf("%s\n", greeting);
    return 0;
}


