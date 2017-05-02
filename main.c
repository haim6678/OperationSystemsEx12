#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include <wait.h>

#define SIZE 512
#define ‫‪COMPILATION_ERROR‬‬ -2
#define ‫‪TIMEOUT‬‬ -3
#define MULTIPLE_DIRECTORIES -5
#define NO_C_FILE -6
#define FORK_FAILED -7
#define CORE_DUMPED_EXEPTION -8
#define WAIT_ERROR -9


//the function deceleration
void startChecking(char *usersDirPath, char *inputSource,
                   char *outputSource, char *compareProgPath, int resultFile);

void checkExecutableAndRun(char *dir, char *studentDirName, int depth,
                           int inputFile, int outputFile, char *compareProgPath,
                           int resultFile, char *name);

void compileFile(char *dir, char *fileName, int inputFile, char *outputFile, int depth,
                 char *initialPath, char *studentName, int resultFile);

void executeUserProg(char *dirName, int inputFileDescriptor,
                     char *compareProgPath, char *givenOutputFile,
                     int depth, char *studentName, int resultFile);

void runCompare(char *userOutput, char *outputFile, char *progDirPath, int depth,
                char *studentName, int resultFile);

int calcGrade(int grade, int depth);

void getStringGrade(char *buff, int grade, int depth);

void setGrade(int grade, int depth, int resultFile, char *studentName);

int is_C_file(char *pDirent);

int checkForManyFOlders(char *temp);


/**
 * the main function.
 * opens the config file,then opens the users directory
 * runs every user program and grade him.
 * @param argc - number of args
 * @param argv- the args for the program
 * @return - exit value
 */
int main(int argc, char *argv[]) {

    //declare variables
    char buff[SIZE];
    char outputSource[SIZE];
    char inputSource[SIZE];
    char usersDirPath[SIZE];
    char initialProgPath[SIZE];
    int resultFile;
    char *paths;
    ssize_t readNum;

    //check if we got the args
    if (argc < 2) {
        perror("wrong arguments");
        exit(-1);
    }

    //get the current working directory
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
    startChecking(usersDirPath, inputSource, outputSource, initialProgPath, resultFile);

    //go back to initial directory
    chdir(initialProgPath);

    close(config);
    return 0;
}

/**
 * get the users directory path and
 * start running on each student
 * fins is c file and grade him.
 * @param usersDirPath - the users directory
 * @param inputSource - the input for the user program
 * @param outputSource - the output for the user program
 * @param resultDescriptor -the file where to write is result
 */
void startChecking(char *usersDirPath, char *inputSource, char *outputSource,
                   char *compareProgPath, int resultFile) {

    //declare variables
    struct dirent *pDirent;
    DIR *users;
    int inputDescriptor;
    int outputDescriptor;
    int depth;
    int temp;
    char tempPath[SIZE];

    //open users directory
    users = opendir(usersDirPath);
    if (users == NULL) {
        perror("failed open users directory");
        exit(-1);
    }

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

        //reset working directory ro the users directory
        chdir(usersDirPath);
        //reset the given input and output files
        lseek(inputDescriptor, 0, SEEK_SET);
        lseek(outputDescriptor, 0, SEEK_SET);

        //check for more then one directory
        strtok(tempPath, usersDirPath);
        strtok(tempPath, "/");
        strtok(tempPath, pDirent->d_name);
        temp = checkForManyFOlders(tempPath);

        //if there is more then one
        if (temp > 1) {
            setGrade(MULTIPLE_DIRECTORIES, 0, resultFile, pDirent->d_name);
            continue;
        }
        //iterate and find file
        if (pDirent->d_type == DT_DIR) { //TODO will work or need stat?
            depth = 0;

            // find the exe compile an run is
            //then compare to the expected files
            checkExecutableAndRun(usersDirPath, pDirent->d_name, depth, inputDescriptor, outputDescriptor,
                                  compareProgPath, resultFile, pDirent->d_name);

            //todo what to do?
        }
    }
}


//todo close dir

/**
 * check in a given student directory if there is a c file
 * if there is then compile an run it.
 * @param dir - the curr directory
 * @param studentDirName -students directory
 * @param depth - the depth of the c file in the students directory
 * @param inputFile - the input for the user program
 * @param outputFile  - the output for the user program
 * @param compareProgPath - the path to the compare program
 * @param resultFile - the file where to write is grade
 */
void checkExecutableAndRun(char *dir, char *studentDirName, int depth, int inputFile, int outputFile,
                           char *compareProgPath, int resultFile, char *name) {

    //declare vars
    char *temp = dir;
    struct dirent *pDirent;
    ++depth;
    DIR *studentDir;

    //create path to students directory and open it
    strtok(dir, "/");
    strtok(dir, studentDirName);
    studentDir = opendir(dir);
    if (studentDir == NULL) {
        //todo HANDLE
    }

    chdir(dir);
    //search for c file
    if (((pDirent = readdir(studentDir)) == NULL)) {
        setGrade(NO_C_FILE, 0, resultFile, name);
        return;
    } else if (is_C_file(pDirent->d_name) == 1) {
        //compile and run
        compileFile(dir, pDirent->d_name, inputFile, outputFile, depth,
                    compareProgPath, studentDirName, resultFile);
        return;
    }

    //recursive call to search  for c file
    checkExecutableAndRun(dir, pDirent->d_name, depth, inputFile, outputFile, studentDirName, resultFile, name);
}

/**
 * gets a path to a c file
 * opens a new son process and runs this
 * file on the new process. then compare the
 * result to a given file.
 * @param dir - the directory where the file is
 * @param fileName  - the file name
 * @param inputFile - the input for the user program
 * @param outputFile  - the output for the user program
 * @param depth - the depth the file is in
 * @param initialPath - the place where the compare program is
 * @param studentName - the student's name
 * @param resultFile - the file where to write is grade
 */
void compileFile(char *dir, char *fileName, int inputFile, char *outputFile, int depth,
                 char *initialPath, char *studentName, int resultFile) {

    //declare variables
    char exeFilePath[SIZE];
    int status;
    int pid;

    //create exe path
    strtok(exeFilePath, dir);
    strtok(exeFilePath, "/");
    strtok(exeFilePath, fileName);

    //create son process
    pid = fork();

    //we are in the son process
    if (pid == 0) {

        execlp("gcc", "gcc", "-o", "student.out", fileName, NULL);

        //todo handle exec fail

        //we are in the father process
    } else {

        //wait for sun
        pid = waitpid(pid, &status, 0);
        //check wait succeed
        if (pid == -1) {
            //todo handle wait failed
        };

        //check compile errors
        if (WIFEXITED (status)) {
            //if there was a compile problem
            if (WEXITSTATUS(status) == 1) {
                setGrade(COMPILATION_ERROR‬‬, 0, resultFile, studentName);
                return;
                //everything was fine
            } else if (WEXITSTATUS(status) == 0) {
                executeUserProg(dir, inputFile, initialPath, outputFile, depth, studentName, resultFile);
            }
        }
    }
}

/**
 * runs a given path to an exe file
 * read the input from a file.
 * also write the output to a file.
 * then compare the output the the wanted result
 * @param dirName - the directory where the file is
 * @param inputFileDescriptor - the input descriptor
 * @param compareProgPath  - the path to compare program
 * @param givenOutputFile - the wanted output.
 * @param depth - the depth of the file's directory
 * @param studentName - the name
 * @param resultFile - the file where to write is grade
 */
void executeUserProg(char *dirName, int inputFileDescriptor,
                     char *compareProgPath, char *givenOutputFile, int depth, char *studentName, int resultFile) {

    //declare variables
    __pid_t pid;
    int status;
    int result;
    int success;
    int userOutputDescriptor;
    char userProgName[SIZE];
    char userOutputName[SIZE];

    //create output file
    userOutputDescriptor = open("studentOutput", O_RDWR | O_APPEND | O_EXCL, 0666);

    //create program out path
    strtok(userProgName, dirName);
    strtok(userProgName, "/");
    strtok(userProgName, "student.out");

    //create user output path
    strtok(userOutputName, dirName);
    strtok(userOutputName, "/");
    strtok(userOutputName, "studentOutput");

    //use dup for input
    success = dup2(inputFileDescriptor, 0);
    if (success == -1) {
        //todo handle dup fail
    }

    //ude dup for output
    success = dup2(userOutputDescriptor, 1);
    if (success == -1) {
        //todo handle dup fail
    }

    //create sun to run user program
    pid = fork();
    if (pid < 0) {
        //todo handle error
    }

    //we are son process
    if (pid == 0) {
        //run program
        execlp(dirName, userProgName, NULL);

        //todo check for timeout or exeptiom and set grade

        //we are father process
    } else {
        //wait 5 seconds for son
        sleep(5);
        //check if sin finished correctly
        if ((success = waitpid(pid, &status, WNOHANG)) == 0) {
            setGrade(TIMEOUT‬‬, 0, resultFile, studentName);
        } else if (success == -1) {
            //todo handle error waiting
            //todo: exit? or set grade to 0?
        } else {
            //run my compare program
            runCompare(userOutputName, givenOutputFile, compareProgPath, depth, studentName, resultFile);
        }

        //restore position
        chdir(getenv("HOME"));
        chdir(dirName);
        unlink(userProgName);
        unlink(userOutputName);
        return;

    }
}

/**
 * the function that runs the compare program on
 * the result.
 * @param userOutput - the wanted output.
 * @param outputFile - the wanted input.
 * @param progDirPath - the path to compare program
 * @param depth - the depth the file is in
 * @param studentName - the student's name
 * @param resultFile - the file where to write is grade
 * @return
 */
void runCompare(char *userOutput, char *outputFile, char *progDirPath, int depth, char *studentName, int resultFile) {

    //declare variables
    int status;
    __pid_t pid;
    DIR *progDir;
    int compareResult;

    //go to the compare program path
    chdir(getenv("HOME"));
    progDir = opendir(progDirPath);
    if (progDir == NULL) {

        //todo handle
    }
    chdir(progDirPath);

    //fork a son
    if ((pid = fork()) < 0) {
        //todo handle error
    }

    //if we are the son process
    if (pid == 0) {
        //run my ex11 on it
        execlp(progDirPath, "ex11", userOutput, outputFile, NULL);
        //check if fail
    } else {

        if (waitpid(pid, &status, 0) == -1) {
            // todo handle wait error
        }

        //if exited normally
        if (WIFEXITED(status)) {
            //then get the compare result given by the exit command of the compare program
            compareResult = WEXITSTATUS(status);
            //set the students grade
            setGrade(compareResult, depth, resultFile, studentName);
        } else {
            //todo handle
        }

    }
}

/**
 * calculate the numeric grade that the student needs to get.
 * @param grade - an indicator of the grade.
 * @param depth - the depth pendelty
 * @return - the final grade
 */
int calcGrade(int grade, int depth) {
    int temp;
    //calc the grade
    switch (grade) {
        case 1:
            temp = (100) - (10 * depth);
            if (temp > 0) {
                return temp;
            }
            return 0;
        case 2:
            temp = (70) - (10 * depth);
            if (temp > 0) {
                return temp;
            }
            return 0;
        case 3:
            return 0;
        default:
            return 0;
    }
}

/**
 * get the student's grade.
 * with a faw parameters.
 * calc is grade according to the parameters and write is
 * grade to the file.
 * @param grade - the indicator for the grade
 * @param depth - the depth of is c file
 * @param resultFile - the file to write to.
 * @param studentName - his name.
 */
void setGrade(int grade, int depth, int resultFile, char *studentName) {

    //declare variables
    char finalDetails[512];
    char gradeDescription[128];
    char gradeString[128];
    int temp;
    ssize_t writen;


    //convert grade to string
    temp = calcGrade(grade, depth);
    writen = snprintf(gradeString, 128, "%d", temp);
    if (writen < 0) {
        //todo handle this
    }

    //gets the string describes his grade
    getStringGrade(gradeDescription, grade, depth);

    //create one long string
    strtok(finalDetails, studentName);
    strtok(finalDetails, ",");
    strtok(finalDetails, gradeString);
    strtok(finalDetails, ",");
    strtok(finalDetails, gradeDescription);

    //write this string
    writen = write(resultFile, studentName, strlen(finalDetails));
    if (writen < 0) {
        //todo handle error
    }

    return;
}

/**
 * check if a given path is a path to a c file
 * @param pDirent - the path
 * @return  -if it is a c file
 */
int is_C_file(char *pDirent) { //todo will work or need to use lstat?
    size_t suffix = strlen(pDirent) - 1;
    if ((suffix > 2) && (pDirent[suffix] == 'c') && (pDirent[suffix - 1] == '.')) { //todo >2 or >=2
        return 1;
    }
    return 0;
}

/**
 * check if a given directory has more then one
 * subdirectory in a given directory
 * @param temp - the given directory
 * @return - if more then one sub-directory
 */
int checkForManyFOlders(char *temp) {

    struct dirent *pDirnet;
    DIR *dir;
    int num = 0;
    dir = opendir(temp);
    if (dir == NULL) {

        //TODO HANDLE!
    }

    //count the sub directories
    while ((pDirnet = readdir(dir)) != NULL) {
        if (pDirnet->d_type == DT_DIR) {
            num++;
        }
    }
    return num;
}

/**
 * gets a string that gives a feedback on
 * a students grade.
 * @param buff - where to enter the string
 * @param grade - the grade
 * @param depth  - the depth of the c file.
 */
void getStringGrade(char *buff, int grade, int depth) {

    memset(buff, '\0', sizeof(buff));
    switch (grade) {
        case 3:
            if (depth > 0) { //todo 0 or 1?
                strcpy(buff, "WRONG_DIRECTORY");
                return;
            }
            strcpy(buff, "GREAT_JOB");
            return;
        case 2:
            strcpy(buff, "SIMILAR_OUTPUT");
            return;
        case 1:
            strcpy(buff, "WRONG_OUTPUT");
            return;
        case -2:
            strcpy(buff, "COMPILATION_ERROR");
            return;
        case -3:
            strcpy(buff, "TIMEOUT");
            return;
        case -4:
            strcpy(buff, "NO_C_FILE");
            return;
        case -5:
            strcpy(buff, "MULTIPLE_DIRECTORIES");
            return;
        default:
            break;
    }
}