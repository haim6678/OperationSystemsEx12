#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>

#define SIZE 2048

int main(int argc, char *argv[]) {

    char buff[SIZE];
    char outputSource[SIZE];
    char inputSource[SIZE];
    char usersDirPath[SIZE];

    //check if we got the args
    if (argc < 2) {
        perror("wrong arguments");
        exit(-1);
    }

    FILE *config = fopen(argv[1], "r");
    if (config == NULL) {
        perror("failed open config file");
        exit(-1);
    }

    //set seek to start file
    int seek = fseek(config, SEEK_SET, 0);

    int resultFile = open("result.csv", O_CREAT || O_APPEND || O_RDWR);
    if (resultFile < 0) {
        perror("failed open result file");
        exit(-1);
    }

    read(config, buff, SIZE);
    char *paths = strtok(buff, "\n");
    strcpy(usersDirPath, paths);
    paths = strtok(NULL, "\n");
    strcpy(inputSource, paths);
    paths = strtok(NULL, "\n");
    strcpy(outputSource, paths);

    startChecking(usersDirPath, inputSource, outputSource);
    close(config);
    return 0;
}

void startChecking(char *usersDirPath, char *inputSource, char *outputSource) {

    DIR *users = opendir(usersDirPath);
    if (users == NULL) {
        perror("failed open users directory");
        exit(-1);
    }

    //open input
    //open output
    //iterate over users

    //for every user:
    // find is exe
    //count the depth of is exe
    //compile an run it
    //check for timeout etc
    //compare output files with ex11
    //set is grade

    //close files
    //close dir

    //return to main
}