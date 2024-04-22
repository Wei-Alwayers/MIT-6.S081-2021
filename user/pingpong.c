//
// Created by 魏浩铭 on 2024/4/22.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);
    if(fork() == 0) {
        // child process
        char *child_char = "";
        read(p[0], child_char, 1);
        close(p[0]);
        write(p[1], "o", 1);
        close(p[1]);
        int child_pid = getpid();
        printf("%d: received p%sng\n", child_pid, child_char);
        exit(0);
    } else{
        // parent process
        write(p[1], "i", 1);
        close(p[1]);
        char *parent_char = "";
        read(p[0], parent_char, 1);
        close(p[0]);
        int parent_pid = getpid();
        sleep(10); // Make the output more presentable
        printf("%d: received p%sng\n", parent_pid, parent_char);
        exit(0);
    }
}