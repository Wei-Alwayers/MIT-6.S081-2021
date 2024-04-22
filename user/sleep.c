//
// Created by 魏浩铭 on 2024/4/22.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int
main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(2, "Usage: sleep pid...");
        exit(1);
    }
    int nums = atoi(argv[1]);
    sleep(nums);
    exit(0);
}
