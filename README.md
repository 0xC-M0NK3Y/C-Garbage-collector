# C-Garbage-collector
An idea of C garbage collector for monkey C programmers.

It tracks the stack of the main function to see if pointers still pointing to allocated address, if there nothing pointing on it, 
it will be automaticly freed at runtime.
It uses a thread that perfom the check every second.
/!\ Only of the stack of the main (its just for the idea) /!\

```c
#include <stdio.h>
#include <unistd.h>
#include "gcollector.h"

/* Example main */
int main(void)
{
    char *ptr1, *ptr2, *ptr3;

    /* Init will retriev rsp and rbp of the main function, to analyze his stack and free if pointers get lost */
    if (gcollector_init() < 0) {
        printf("%s\n", gcollector_get_error()); // gcollector_get_error() returns const char * of the last error 
        return 1;
    }

    /* 
       The principe of the garbage collector is to iterate throught all the stack of the main
       in a thread every seconde and check if there is still something pointing in the address that was 
       allocated with galloc
    */ 

    /* If galloc fails it returns NULL, you can call gcollector_get_error() to see the error */
    ptr1 = galloc(150); printf("ptr1 at %p\n", ptr1);
    ptr2 = galloc(200); printf("ptr2 at %p\n", ptr2);
    ptr3 = galloc(50);  printf("ptr3 at %p\n", ptr3);

    ptr1 = NULL; // lost of the pointer (garbage collector will free)
    ptr2 = NULL; // lost of the pointer (garbage collector will free)

    ptr3++; // pointer still considered valid by the garbage collector

    /* The garbage collector launch a thread that checks the pointers one time every second, sleep to have time see them being freed */
    sleep(1);

    gcollector_quit(); // this free all the pointers remaining allocated with galloc() no leaks possible
    return 0;
}
```

Output (there are printf inside the garbage collector code) :

```bash
$ ./gcollector
ptr1 at 0x564f8e96d470
ptr2 at 0x564f8e96d920
ptr3 at 0x564f8e96d9f0
free 0x564f8e96d470
free 0x564f8e96d920
QUIT free 0x564f8e96d9f0
```
