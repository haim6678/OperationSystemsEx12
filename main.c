#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>

#define SIZE 512

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

void startChecking(char *usersDirPath, char *inputSource, char *outputSource, int resultDescriptor) {

    //open users directory
    DIR *users = opendir(usersDirPath);
    struct dirent *pDirent;
    if (users == NULL) {
        perror("failed open users directory");
        exit(-1);
    }

    //open input source
    int inputDescriptor = open(inputSource, O_RDONLY);
    if (inputDescriptor < 0) {
        perror("failed open input source");
        exit(-1);
    }

    //open output source
    int outputDescriptor = open(outputSource, O_RDONLY);
    if (inputDescriptor < 0) {
        perror("failed open output source");
        exit(-1);
    }

    //iterate over users
    char *studentName;
    int depth;
    int grade;
    //for every user run test
    while ((pDirent = readdir(users)) != NULL) {
        studentName = pDirent->d_name;

        if (pDirent->d_type == DT_DIR) { //TODO will worl or need lstat?

            // find is exe
            //count the depth of is exe
            depth = hasExecutable();
            //has an executable
            if (depth != -1) {
                //compile an run it
                //check for timeout etc
                //compare output files with ex11
                grade = runCheck();
                //set is grade
                setGrade(depth, grade, resultDescriptor);
            } else {
                //didn't fount exe
                //todo what to do?
            }

        }

    }


    //close files
    //close dir

    //return to main
    //finish
}