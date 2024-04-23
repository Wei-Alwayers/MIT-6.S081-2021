//
// Created by 魏浩铭 on 2024/4/23.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
    static char buf[DIRSIZ+1];
    char *p;

    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

void
find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    // get file descriptor
    if((fd = open(path, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", path);
        return;
    }
    // get file status
    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch (st.type) {
        case T_FILE:
            // Find first character after last slash.
            for(p=path+strlen(path); p >= path && *p != '/'; p--)
                ;
            p++;
            if(strcmp(p, target) == 0){
                // find target file path
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf+strlen(buf);
            *p++ = '/';
            while (read(fd, &de, sizeof (de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                // skip recurse "." & ".."
                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0){
                    continue;
                }
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                // recursion find
                find(buf, target);
            }
            break;
    }
    close(fd);
}
int
main(int argc, char *argv[]){
    if(argc < 3){
        fprintf(2, "Usage: find <path-name> <file-name>");
        exit(0);
    }
    find(argv[1], argv[2]);
    exit(0);
}