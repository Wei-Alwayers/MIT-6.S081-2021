//
// Created by 魏浩铭 on 2024/4/23.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define MAX_NUM 35

void
prime(int *p){
    // child process
    close(p[1]);
    // print prime
    int res;
    read(p[0], &res, sizeof(res));
    if(res != 0){
        printf("prime %d\n", res);
    } else{
        exit(0);
    }

    // child pipe
    int child_p[2];
    pipe(child_p);
    if(fork() == 0){
        // child process
        close(p[0]);
        prime(child_p);
    }else{
        // parent process
        close(child_p[0]);
        int num = res;
        // write possible primes to child pipe
        while (num != 0){
            num = 0;
            read(p[0], &num, sizeof(num));
            if(num % res != 0){
                write(child_p[1], &num, sizeof (num));
            }
        }
        close(p[0]);
        close(child_p[1]);
    }
}

int
main(int argc, char *argv[]){
    int p[2];
    pipe(p);
    if(fork() == 0){
        // child process
        prime(p);
    } else{
        // parent process
        close(p[0]);
        for(int i = 2; i <= MAX_NUM; i++){
            write(p[1], &i, sizeof(i));
        }
        close(p[1]);
    }
    wait(0);
    exit(0);
}