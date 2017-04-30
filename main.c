#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include <wait.h>

#define SIZE 512

void startChecking(char *usersDirPath, char *inputSource, char *outputSource, int resultDescriptor);

int main(int argc, char *argv[]) {

    //declare variables
    char buff[SIZE];
    char outputSource[SIZE];
    char inputSource[SIZE];
    char usersDirPath[SIZE];

    //check if we got the args
    if (argc < 2) {
        perror("wrong arguments");
        exit(-1);
    }

    //open the config file to read
    int config = open(argv[1], O_RDONLY);
    if (config == NULL) {
        perror("failed open config file");
        exit(-1);
    }

    //set the reading to br from start
    off_t seek = lseek(config, 0, SEEK_SET);
    //todo check succeed


    //create result file
    int resultFile = open("result.csv", O_CREAT || O_APPEND || O_RDWR);
    if (resultFile < 0) {
        perror("failed open result file");
        exit(-1);
    }

    //read the all file
    ssize_t n = read(config, buff, SIZE);
    //cut every line of it
    char *paths = strtok(buff, "\n");
    strcpy(usersDirPath, paths);

    //go to the students directory
    chdir(usersDirPath);

    //initialize the input output files
    paths = strtok(NULL, "\n");
    strcpy(inputSource, paths);
    paths = strtok(NULL, "\n");
    strcpy(outputSource, paths);

    startChecking(usersDirPath, inputSource, outputSource, resultFile);

    //todo go back to initial path

    close(config);
    return 0;
}

void startChecking(char *usersDirPath, char *inputSource, char *outputSource, int resultDescriptor) {

    struct dirent *pDirent;

    //open users directory
    DIR *users = opendir(usersDirPath);
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

        if (pDirent->d_type == DT_DIR) { //TODO will work or need stat?
            depth = 0;
            // find is exe
            //count the depth of is exe

            depth = checkExecutableAndRun(usersDirPath, pDirent->d_name, &depth, inputDescriptor, outputDescriptor);
            //has an executable
            setGrade()
            //todo what to do?
        }

    }

}


//close files
//close dir

//return to main
//finish
}

int checkExecutableAndRun(char *dir, char *studentDirName, int depth, int inputFile, int outputFile,) {

    //declare vars
    char *temp = dir;
    struct dirent *pDirent;
    ++(depth);
    DIR *studentDir;


    strcat(dir, "/");
    strcat(dir, studentDirName);
    studentDir = opendir(dir);
    if (studentDir == NULL) {

        //todo HANDLE
    }

    chdir(dir);
    //search for c file
    if (((pDirent = readdir(studentDir)) == NULL)) {
        return -1;
    } else if (is_C_file(pDirent->d_name) == 1) {
        //compile and run

        return runFile(dir, pDirent->d_name, inputFile, outputFile, depth);
    }

    return checkExecutableAndRun(dir, pDirent->d_name, depth, inputFile, outputFile);
}


int runFile(char *dir, char *fileName, int inputFile, int outputFile, int depth) {

    chdir(dir);
    int status;
    strcat(dir, "/");
    strcat(dir, fileName);

    int pid = fork();
    //son process
    if (pid == 0) {

        execlp("gcc", "gcc", "-o", "student.out", fileName, NULL);

        //todo handle exec fail
    } else {

        //wait for sun
        //check wait
        pid = waitpid(pid, &status, 0);
        if (pid == -1) {
            //todo handle wait failed
        };


        if (WIFEXITED (status)) {

        } else {
            //todo handle error compiling
        }


        //check compile error

        //run the exe file
        return executeUserProg();
    }
}

int executeUserProg(char *name, int resultFile, int inputFile, int outputFile) {

    //use dup for input and output

    //create sun

    //run prog into a file withe the input

    //wait for sun

    runCompare(userOutput, outputFile);
}

int runCompare(int userOutput, int outputFile) {

    //fork a son

    //run my ex11 on it

    //check if fail

    //return grade
}

int is_C_file(char *pDirent) {
    size_t suffix = strlen(pDirent) - 1;
    if ((suffix > 2) && (pDirent[suffix] == 'c') && (pDirent[suffix - 1] == '.')) { //todo >2 or >=2
        return 1;
    }
    return 0;
}
