#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSIZE 1000000
#define MAXN 50

int stack[MAXSIZE];
int size = 0;

int peek(); // return the upper element in stack (without poping it)
void push(int data); // push new element into the stack
void pop(); // pop upper element from the stack
int empty(); // check if the stack is empty
void display(); // print the stack
void create(); // create a new empty stack
int stack_size(); // print stack size


int peek(){
    return stack[size - 1];
}


void push(int data){
    stack[size] = data;
    size += 1;
}

void pop(){
    if (!empty())
        stack[size - 1] = 0;
        size -= 1;
}

int empty(){
    return (size == 0);
}

void display(){
    for (int i = size - 1; i >= 0; i--)
        printf("%d | ", stack[i]);
    printf("\n");
}

void create(){
    for (int i = size - 1; i >= 0; i--)
        pop();
    size = 0;
}

int stack_size(){
    return size;
}


main()
{
    int p[2], pid, n;
    char strin[MAXN], strout[MAXN];

    pipe(p);

    pid = fork();

    if (pid > 0){ // parent -- client
        close(p[0]);
        scanf("%d", n); // user enters number of commands
        write(p[1], &n, sizeof(n));
        for (int i = 0; i < n; i++){
            scanf("%s", strin);
            write(p[1], strin, MAXN);
        }
        close(p[1]);
    }
    else if (pid == 0){ // child -- server
        close(p[1]);
        int m;
        read(p[0], &m, sizeof(m));
        for (int i = 0; i < m; i++){
            if (strout[0] == 'p' &&
                    strout[1] == 'u' &&
                    strout[2] == 's' &&
                    strout[3] == 'h'){ // checking if it is "push()" command
                read(p[0], strout, MAXN);
                int j = 5, data = 0;
                while (strout[j] != ')')
                    data = data * 10 + (int) (strout[j] - '0'); // building int from string
                push(data);
            }
            else if (strout == "peek()")
                printf("%d", peek());
            else if (strout == "pop()")
                pop();
            else if (strout == "empty()")
                printf("%d", empty());
            else if (strout == "display()")
                display();
            else if (strout == "create()")
                create();
            else if (strout == "stack_size()")
                printf("%d", stack_size());
            else
                printf("wrong command!\n");

            read(p[0], strout, MAXN);
        }
        close(p[0]);
    }
    else{ // error
        printf("fork() executed with error\n");
    }

    return 0;
}
