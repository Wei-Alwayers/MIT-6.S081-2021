//
// Created by 魏浩铭 on 2024/4/23.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(2, "Usage: command1 | xargs command2\n");
        exit(0);
    }
    char ch;
    char buf[512];
    int index = 0;

    while (read(0, &ch, sizeof(char)) == sizeof(char)) {
        if (ch == '\n') {
            // Null-terminate the buffer to form a string
            buf[index] = '\0';
            // Fork a new process
            if (fork() == 0) {
                // Prepare the arguments for exec
                int new_argc = argc - 1 + 2;
                char *args[new_argc];  // Maximum arguments including command name and NULL terminator
                for (int i = 1; i < argc; i++) {
                    args[i - 1] = argv[i];  // Additional arguments
                }
                args[new_argc - 2] = buf;  // Append the buffer as the last argument
                args[new_argc - 1] = 0;
                // Execute the command with arguments
                exec(argv[1], args);
                // If exec returns, it indicates an error
                fprintf(2, "xargs: exec %s failed\n", argv[1]);
                exit(1);
            }
            // Reset the buffer index for the next argument
            index = 0;
        } else {
            // Add the character to the buffer
            buf[index++] = ch;

            // Check if buffer exceeds its maximum size
            if (index >= sizeof(buf)) {
                fprintf(2, "xargs: input line too long\n");
                exit(1);
            }
        }
    }
    wait(0);
    exit(0);

}