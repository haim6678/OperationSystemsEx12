#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include <wait.h>

#define COMPILE_ERROR -2
#define TIMEOUT_EXEPTION -3
#define NOT_ACCURATE_OUTPUT -4
#define WRONG_OUTPUT -5
#define NO_C_FILE -6
#define FORK_FAILED -7
#define CORE_DUMPED_EXEPTION -8
#define WAIT_ERROR
#define SIZE 512

void startChecking(char *usersDirPath, char *inputSource, char *outputSource, int resultDescriptor);

int main(int argc, char *argv[]) {

    //declare variables
    char buff[SIZE];
    char outputSource[SIZE];
    char inputSource[SIZE];
    char usersDirPath[SIZE];
    char initialProgPath[SIZE];
    int resultFile;
    char *paths;
    DIR *users;
    ssize_t readNum;
    //check if we got the args
    if (argc < 2) {
        perror("wrong arguments");
        exit(-1);
    }

    if ((getcwd(initialProgPath, SIZE) == NULL)) {
        //todo handle error
    }
    //open the config file to read
    int config = open(argv[1], O_RDONLY);
    if (config == NULL) {
        perror("failed open config file");
        exit(-1);
    }

    //set the reading to br from start
    off_t seek = lseek(config, 0, SEEK_SET);
    if (seek == -2) {
        //todo handle error
    }


    //create result file
    resultFile = open("result.csv", O_CREAT || O_APPEND || O_RDWR);
    if (resultFile < 0) {
        perror("failed open result file");
        exit(-1);
    }

    //read the all file
    readNum = read(config, buff, SIZE);
    if (readNum < 0) {
        //todo handle error
    }

    //cut every line of it
    //start with users directory
    paths = strtok(buff, "\n");
    strcpy(usersDirPath, paths);

    //initialize the input output files pathes
    paths = strtok(NULL, "\n");
    strcpy(inputSource, paths);
    paths = strtok(NULL, "\n");
    strcpy(outputSource, paths);

    //run check on every student
    startChecking(usersDirPath, inputSource, outputSource, resultFile);

    //go back to initial directory
    chdir(initialProgPath);

    close(config);
    return 0;
}

void startChecking(char *usersDirPath, char *inputSource, char *outputSource, int resultDescriptor) {

    struct dirent *pDirent;
    DIR *users;
    int inputDescriptor;
    int outputDescriptor;
    char *studentName;
    int depth;
    int grade;
    //open users directory
    users = opendir(usersDirPath);
    if (users == NULL) {
        perror("failed open users directory");
        exit(-1);
    }

    //go to the students directory
    chdir(usersDirPath);

    //open input source
    inputDescriptor = open(inputSource, O_RDONLY);
    if (inputDescriptor < 0) {
        perror("failed open input source");
        exit(-1);
    }

    //open output source
    outputDescriptor = open(outputSource, O_RDONLY);
    if (inputDescriptor < 0) {
        perror("failed open output source");
        exit(-1);
    }

    //iterate over users
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
    char *outName;
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

        //check compile errors
        if (WIFEXITED (status)) {
            if (WEXITSTATUS(status) == 1) {
                //todo handle error compiling
            }
        } else if (WEXITSTATUS(status) == 0) {
            x
            return executeUserProg(outName, inputFile, outputFile);
        }


    }
}

int executeUserProg(char *name, int inputFile, char *outputFile, int outputFileDescriptor, char *compareProgPath) {

    //declare variables
    __pid_t pid;
    int status;
    int result;
    int success;
    char userProgName[SIZE];
    char userOutputName[SIZE];

    //create outpu file
    int userOutput = open("studetOutput", O_RDWR | O_APPEND | O_EXCL, 0666);

    //create program out path
    strtok(userProgName, name);
    strtok(name, "/");
    strtok(name, "student.out");

    //create user output path
    strtok(userOutputName, name);
    strtok(name, "/");
    strtok(name, "studetOutput");

    //use dup for input
    success = dup2(inputFile, 0);
    if (success == -1) {
        //todo handle dup fail
    }

    //ude dup for output
    success = dup2(outputFileDescriptor, 1);
    if (success == -1) {
        //todo handle dup fail
    }

    //create sun
    pid = fork();
    if (pid < 0) {
        //todo handle error
    }

    //we are son process
    if (pid == 0) {
        //run program
        execlp(userProgName, userOutputName, NULL);


        //check for timeout

        //we are father process
    } else {

        //wait for son
        if (waitpid(pid, &status, 0) == -1) {
            //todo handle wait error
        }
        //run my compare prog
        result = runCompare(userOutputName, outputFile, compareProgPath);
        //chdir(getenv("HOME"));
        chdir(name);
        unlink(userProgName);
        unlink(userOutputName);
        return result;
        //return result
    }

    //todo delete a.out file

}

int runCompare(char *userOutput, char *outputFile, char *progDirPath) {
    int status;
    __pid_t pid;

    chdir(getenv("HOME"));

    DIR *progDir = opendir(progDirPath);
    chdir(progDir);
    //fork a son

    if ((pid = fork()) < 0) {
        //todo handle error
    }

    if (pid == 0) {
        //run my ex11 on it
        execlp("ex11", "ex11", userOutput, outputFile, NULL);
        //check if fail
    } else {

        if (waitpid(pid, &status, 0) == -1) {
            // todo handle wait error
        }


        lseek(outputFile, 0, SEEK_SET);
    }


    //delete user output

    //return grade
}

int is_C_file(char *pDirent) {
    size_t suffix = strlen(pDirent) - 1;
    if ((suffix > 2) && (pDirent[suffix] == 'c') && (pDirent[suffix - 1] == '.')) { //todo >2 or >=2
        return 1;
    }
    return 0;
}
