/******************************************
*
Student name: Haim Rubinstein
*
Student ID: 203405386
*
Course Exercise Group:01
*
Exercise name:Ex12
******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <memory.h>
#include <wait.h>

#define SIZE 512
#define COMPILATION_ERROR -2
#define TIMEOUT -3
#define MULTIPLE_DIRECTORIES -5
#define NO_C_FILE -6
#define FORK_FAILED "failed forking"
#define CORE_DUMPED_EXEPTION -8
#define WAIT_ERROR "FAILED WAITING"
#define DUP_FAIL "dup action failed"
#define OPEN_FILE_FAIL "filed to open file"
#define SEEK_FAIL "failed seek file"
#define READ_FAIL "failed reading"
#define DIR_OPEN_FAIL "failed to open dir"
#define WRITE_ERROR "failed write"
#define EXEC_FAIL "failed exec a son program"
#define STUDENT_PROGRAM_OUTPUT_FILE "studentOutput"
#define STUDENT_COMPILED_FILE_NAME "student.out"
#define COMPARE_PROGRAM_NAME "comp.out"
#define CLOSE_FAILED "failed to close file"
#define UNLINK_FAILED "failed to unlink a file"
#define CHANGE_DIR_FAIL "failed chainging dir"
#define FAIL_GET_CURRENT_DIRECTORY "failed getting current dir"
#define COMPILE_PROCESS_FAIL "compile processs crashed"

//the function deceleration
void StartChecking(char *usersDirPath, char *inputSource,
                   char *outputSource, char *compareProgPath, int resultFile);

void checkExecutableAndRun(char *dir, char *studentDirName, int *depth,
                           int *reslutSearch, char *fileName);

void CompileFile(char *dir, char *fileName, int inputFile, char *outputFile,
                 int depth, char *initialPath, char *studentName, int resultFile);

void ExecuteUserProg(char *dirName, int inputFileDescriptor,
                     char *compareProgPath, char *givenOutputFile,
                     int depth, char *studentName, int resultFile);

void RunCompare(char *userOutput, char *outputFile, char *progDirPath, int depth,
                char *studentName, int resultFile);

int CalcGrade(int grade, int depth);

void GetStringGrade(char *buff, int grade, int depth);

void SetGrade(int grade, int depth, int resultFile, char *studentName);

int IsCFile(char *pDirent);


/********************************************
 * the main function.
 * opens the config file,then opens the users directory
 * runs every user program and grade him.
 * @param argc - number of args
 * @param argv- the args for the program
 * @return - exit value
 *******************************************/
int main(int argc, char *argv[]) {

    //declare variables
    char buff[SIZE];
    char outputSource[SIZE];
    char inputSource[SIZE];
    char usersDirPath[SIZE];
    char initialProgPath[SIZE];
    int resultFile;
    int config;
    char *paths;
    ssize_t readNum;

    //check if we got the args
    if (argc < 2) {
        perror("wrong arguments for program");
        exit(-1);
    }

    //get the current working directory
    if ((getcwd(initialProgPath, SIZE) == NULL)) {
        write(2, FAIL_GET_CURRENT_DIRECTORY, strlen(FAIL_GET_CURRENT_DIRECTORY));
        exit(-1);
    }

    //open the config file to read
    config = open(argv[1], O_RDONLY);
    if (config < 0) {
        write(2, OPEN_FILE_FAIL, strlen(OPEN_FILE_FAIL));
        exit(-1);
    }

    //set the reading to br from start
    off_t seek = lseek(config, 0, SEEK_SET);
    if (seek < 0) {
        write(2, SEEK_FAIL, strlen(SEEK_FAIL));
        exit(-1);
    }

    //create result file
    resultFile = open("result.csv", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (resultFile < 0) {
        write(2, OPEN_FILE_FAIL, strlen(OPEN_FILE_FAIL));
        exit(-1);
    }

    //read the all file
    readNum = read(config, buff, SIZE);
    if (readNum < 0) {
        write(2, READ_FAIL, strlen(READ_FAIL));
        exit(-1);
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
    startChecking(usersDirPath, inputSource, outputSource,
                  initialProgPath, resultFile);

    //go back to initial directory
    if (chdir(initialProgPath) < 0) {
        write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
        exit(-1);
    }

    if (close(config) < 0) {
        write(2, CLOSE_FAILED, strlen(CLOSE_FAILED));
        exit(-1);
    }
    if (close(resultFile) < 0) {
        write(2, CLOSE_FAILED, strlen(CLOSE_FAILED));
        exit(-1);
    }
    return 0;
}

/********************************************
 * get the users directory path and
 * start running on each student
 * fins is c file and grade him.
 * @param usersDirPath - the users directory
 * @param inputSource - the input for the user program
 * @param outputSource - the output for the user program
 * @param resultDescriptor -the file where to write is result
 *******************************************/
void startChecking(char *usersDirPath, char *inputSource, char *outputSource,
                   char *compareProgPath, int resultFile) {

    //declare variables
    int inputDescriptor;
    int outputDescriptor;
    int depth;
    int temp;
    struct dirent *pDirent;
    DIR *users;
    char usersPathBackup[SIZE];
    char stuentNameBackup[SIZE];
    char fileName[SIZE];

    //save the name of the students directory
    strcpy(usersPathBackup, usersDirPath);
    //open users directory
    users = opendir(usersDirPath);
    if (users == NULL) {
        write(2, DIR_OPEN_FAIL, strlen(DIR_OPEN_FAIL));
        exit(-1);
    }

    //iterate over users
    //for every user run test
    while ((pDirent = readdir(users)) != NULL) {

        //check for . or .. folders
        if ((strcmp(pDirent->d_name, ".") == 0)
            || (strcmp(pDirent->d_name, "..") == 0)) {
            continue;
        }

        //reset working directory ro the users directory
        strcpy(usersDirPath, usersPathBackup);
        if (chdir(usersDirPath) < 0) {
            write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
            exit(-1);
        }



        //iterate and find file
        if (pDirent->d_type == DT_DIR) {
            depth = 0;
            temp = 0;
            strcpy(stuentNameBackup, pDirent->d_name);
            // find the exe compile an run is
            //then compare to the expected files
            checkExecutableAndRun(usersDirPath, pDirent->d_name, &depth,
                                  &temp, fileName);
            //check result
            if (temp == 1) {
                inputDescriptor = open(inputSource, O_RDONLY);
                if (inputDescriptor < 0) {
                    write(2, OPEN_FILE_FAIL, strlen(OPEN_FILE_FAIL));
                    exit(-1);
                }

                //compile and run
                compileFile(usersDirPath, fileName, inputDescriptor, outputSource,
                            depth, compareProgPath, stuentNameBackup, resultFile);

                close(inputDescriptor);
            } else if (temp == NO_C_FILE) {
                //no c file
                setGrade(NO_C_FILE, 0, resultFile, stuentNameBackup);
                continue;
            } else if (temp == MULTIPLE_DIRECTORIES) {
                //multiple directories
                setGrade(MULTIPLE_DIRECTORIES, 0, resultFile, stuentNameBackup);
                continue;
            }
        }
    }

    //close input and output files
    if (close(inputDescriptor) < 0) {
        write(2, CLOSE_FAILED, strlen(CLOSE_FAILED));
        exit(-1);
    }
    if (close(outputDescriptor) < 0) {
        write(2, CLOSE_FAILED, strlen(CLOSE_FAILED));
        exit(-1);
    }
}

/********************************************
 * check in a given student directory if there is a c file
 * if there is then compile an run it.
 * @param dir - the curr directory
 * @param studentDirName -students directory
 * @param depth - the depth of the c file in the students directory
 * @param inputFile - the input for the user program
 * @param outputFile  - the output for the user program
 * @param compareProgPath - the path to the compare program
 * @param resultFile - the file where to write is grade
 *******************************************/
void checkExecutableAndRun(char *dir, char *studentDirName, int *depth,
                           int *reslutSearch, char *fileName) {

    //declare var
    struct dirent *pDirent;
    DIR *studentDir;
    int numDirectories = 0;
    int foundDirecoty = 0;
    char tempDir[SIZE];


    //create path to students directory and open it
    strcat(dir, "/");
    strcat(dir, studentDirName);
    studentDir = opendir(dir);
    //open it
    if (studentDir == NULL) {
        write(2, DIR_OPEN_FAIL, strlen(DIR_OPEN_FAIL));
        exit(-1);
    }
    if (chdir(dir) < 0) {
        write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
        exit(-1);
    }

    //get next entry in directory check if it's . or .. or just a file
    pDirent = readdir(studentDir);
    while ((pDirent != NULL)) {
        //it's father's directory
        if ((strcmp(pDirent->d_name, ".") == 0) ||
            (strcmp(pDirent->d_name, "..") == 0)) {
            pDirent = readdir(studentDir);
            continue;
        }
        //it's a c file
        if ((!(pDirent->d_type == DT_DIR) && (is_C_file(pDirent->d_name) == 1))) {
            strcpy(fileName, pDirent->d_name);
            *reslutSearch = 1;
            return;
        }
        //it's a directory
        if (pDirent->d_type == DT_DIR) {
            numDirectories++;
            foundDirecoty = 1;
            memset(tempDir, 0, SIZE);
            strcpy(tempDir, pDirent->d_name);
        }
        pDirent = readdir(studentDir);
    }

    //found no c file and there is multiple directories
    if (numDirectories > 1) {
        *reslutSearch = MULTIPLE_DIRECTORIES;
        return;
    }
    //we found nothing
    if (foundDirecoty == 0) {
        *reslutSearch = NO_C_FILE;
        return;
    }

    //search the next directory
    (*depth)++;
    //recursive call to search for c file
    checkExecutableAndRun(dir, tempDir, depth, reslutSearch, fileName);
}

/********************************************
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
 *******************************************/
void compileFile(char *dir, char *fileName, int inputFile, char *outputFile,
                 int depth, char *initialPath, char *studentName, int resultFile) {

    //declare variables
    int status;
    int pid;

    //create son process
    pid = fork();
    if (pid < 0) {
        write(2, FORK_FAILED, strlen(FORK_FAILED));
        exit(-1);
    }
    //we are in the son process
    if (pid == 0) {

        execlp("gcc", "gcc", "-o", STUDENT_COMPILED_FILE_NAME, fileName, NULL);
        //if an error accrued
        write(2, EXEC_FAIL, strlen(EXEC_FAIL));
        exit(-1);

        //we are in the father process
    } else {
        //wait for sun
        pid = waitpid(pid, &status, 0);
        //check wait succeed
        if (pid == -1) {
            write(2, WAIT_ERROR, strlen(WAIT_ERROR));
            exit(-1);
        };

        //check compile errors
        if (WIFEXITED(status)) {
            //if there was a compile problem
            if (WEXITSTATUS(status) == 1) {
                setGrade(COMPILATION_ERROR, 0, resultFile, studentName);
                return;
                //everything was fine
            } else if (WEXITSTATUS(status) == 0) {
                executeUserProg(dir, inputFile, initialPath, outputFile,
                                depth, studentName, resultFile);
            }
        } else {
            perror(COMPILE_PROCESS_FAIL);
            exit(-1);
        }
    }
}

/********************************************
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
 *******************************************/
void executeUserProg(char *dirName, int inputFileDescriptor,
                     char *compareProgPath, char *givenOutputFile, int depth,
                     char *studentName, int resultFile) {

    //declare variables
    __pid_t pid;
    int status;
    int success;
    int userOutputDescriptor;
    char userOutputName[SIZE];
    char compiledPath[SIZE];

    //create output file path
    memset(userOutputName, 0, SIZE);

    //create user output path
    strcpy(userOutputName, dirName);
    strcat(userOutputName, "/");
    strcat(userOutputName, STUDENT_PROGRAM_OUTPUT_FILE);

    //create sun to run user program
    pid = fork();
    if (pid < 0) {
        write(2, FORK_FAILED, strlen(FORK_FAILED));
        exit(-1);
    }

    //we are son process
    if (pid == 0) {
        //run program
        userOutputDescriptor = open(STUDENT_PROGRAM_OUTPUT_FILE,
                                    O_CREAT | O_RDWR, 0666);
        if (userOutputDescriptor < 0) {
            write(2, OPEN_FILE_FAIL, strlen(OPEN_FILE_FAIL));
            exit(-1);
        }
        if (chdir(dirName) < 0) {
            write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
            exit(-1);
        }
        //use dup for input
        success = dup2(inputFileDescriptor, 0);
        if (success == -1) {
            write(2, DUP_FAIL, strlen(DUP_FAIL));
            exit(-1);
        }

        //ude dup for output
        success = dup2(userOutputDescriptor, 1);
        if (success == -1) {
            write(2, DUP_FAIL, strlen(DUP_FAIL));
            exit(-1);
        }

        memset(compiledPath, 0, SIZE);
        strcpy(compiledPath, "./");
        strcat(compiledPath, STUDENT_COMPILED_FILE_NAME);

        execlp(compiledPath, STUDENT_COMPILED_FILE_NAME, NULL);


        write(2, EXEC_FAIL, strlen(EXEC_FAIL));
        exit(-1);
        //we are father process
    } else {
        //wait 5 seconds for son
        sleep(5);
        //check if son finished correctly
        if ((success = waitpid(pid, &status, WNOHANG)) == 0) {
            setGrade(TIMEOUT, 0, resultFile, studentName);
            //wait failed
        } else if (success == -1) {
            write(2, WAIT_ERROR, strlen(WAIT_ERROR));
            exit(-1);
        } else {
            kill(pid,SIGSTOP);
            //run my compare program
            runCompare(userOutputName, givenOutputFile, compareProgPath,
                       depth, studentName, resultFile);
        }

        //restore position
        if (chdir(getenv("HOME")) < 0) {
            write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
            exit(-1);
        }
        if (chdir(dirName) < 0) {

        }

        if (unlink(STUDENT_COMPILED_FILE_NAME) < 0) {
            write(2, UNLINK_FAILED, strlen(UNLINK_FAILED));
            exit(-1);
        }
        if (unlink(STUDENT_PROGRAM_OUTPUT_FILE) < 0) {
            write(2, UNLINK_FAILED, strlen(UNLINK_FAILED));
            exit(-1);
        }
        return;
    }
}

/********************************************
 * the function that runs the compare program on
 * the result.
 * @param userOutput - the wanted output.
 * @param outputFile - the wanted input.
 * @param progDirPath - the path to compare program
 * @param depth - the depth the file is in
 * @param studentName - the student's name
 * @param resultFile - the file where to write is grade
 *******************************************/
void runCompare(char *userOutput, char *outputFile, char *progDirPath,
                int depth, char *studentName, int resultFile) {

    //declare variables
    int status;
    __pid_t pid;
    DIR *progDir;
    int compareResult;
    char comparePath[SIZE];

    //go to the compare program path
    if (chdir(getenv("HOME")) < 0) {
        write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
        exit(-1);
    }
    progDir = opendir(progDirPath);
    if (progDir == NULL) {
        write(2, DIR_OPEN_FAIL, strlen(DIR_OPEN_FAIL));
        exit(-1);
    }
    if (chdir(progDirPath) < 0) {
        write(2, CHANGE_DIR_FAIL, strlen(CHANGE_DIR_FAIL));
        exit(-1);
    }

    //fork a son
    if ((pid = fork()) < 0) {
        write(2, FORK_FAILED, strlen(FORK_FAILED));
        exit(-1);
    }

    //if we are the son process
    if (pid == 0) {
        memset(comparePath, 0, SIZE);
        strcpy(comparePath, "./");
        strcat(comparePath, COMPARE_PROGRAM_NAME);
        //run my ex11 on it
        execlp(comparePath, COMPARE_PROGRAM_NAME, userOutput, outputFile, NULL);
        //check if fail
        write(2, EXEC_FAIL, strlen(EXEC_FAIL));
        exit(-1);

    } else if (waitpid(pid, &status, 0) == -1) {
        write(2, WAIT_ERROR, strlen(WAIT_ERROR));
        exit(-1);
    }

    //if exited normally
    if (WIFEXITED(status)) {
        //then get the compare result, given by the exit command of the compare program
        compareResult = WEXITSTATUS(status);
        //set the students grade
        setGrade(compareResult, depth, resultFile, studentName);
    } else {
        perror("compare process failed");
        exit(-1);
    }
}

/********************************************
 * calculate the numeric grade that the student needs to get.
 * @param grade - an indicator of the grade.
 * @param depth - the depth pendelty
 * @return - the final grade
 *******************************************/
int calcGrade(int grade, int depth) {

    //declare variables
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

/********************************************
 * get the student's grade.
 * with a faw parameters.
 * calc is grade according to the parameters and write is
 * grade to the file.
 * @param grade - the indicator for the grade
 * @param depth - the depth of is c file
 * @param resultFile - the file to write to.
 * @param studentName - his name.
 *******************************************/
void setGrade(int grade, int depth, int resultFile, char *studentName) {

    //declare variables
    char finalDetails[512];
    char gradeDescription[128];
    char gradeInString[128];
    int temp;
    ssize_t writen;

    memset(gradeInString, 0, 128);
    //convert grade to string
    temp = calcGrade(grade, depth);
    writen = snprintf(gradeInString, 128, "%d", temp);
    if (writen < 0) {
        perror("failed converting grade to string");
    }

    //gets the string describes his grade
    getStringGrade(gradeDescription, grade, depth);



    //create one long string
    memset(finalDetails, 0, 512);
    strcpy(finalDetails, studentName);
    strcat(finalDetails, ",");
    strcat(finalDetails, gradeInString);
    strcat(finalDetails, ",");
    strcat(finalDetails, gradeDescription);
    strcat(finalDetails, "\n");

    //write this string
    writen = write(resultFile, finalDetails, strlen(finalDetails));
    if (writen < 0) {
        write(2, WRITE_ERROR, strlen(WRITE_ERROR));
        exit(-1);
    }
    return;
}

/********************************************
 * check if a given path is a path to a c file
 * @param pDirent - the path
 * @return  -if it is a c file
 *******************************************/
int is_C_file(char *pDirent) {
    size_t suffix = strlen(pDirent) - 1;
    if ((suffix > 2) && (pDirent[suffix] == 'c') &&
        (pDirent[suffix - 1] == '.')) {
        return 1;
    }
    return 0;
}

/********************************************
 * gets a string that gives a feedback on
 * a students grade.
 * @param buff - where to enter the string
 * @param grade - the grade
 * @param depth  - the depth of the c file.
 *******************************************/
void getStringGrade(char *buff, int grade, int depth) {

    //convert according to the case
    memset(buff, 0, sizeof(buff));
    switch (grade) {
        case 1:
            if (depth > 0) {
                strcpy(buff, "WRONG_DIRECTORY");
                strcat(buff, ",");
                strcat(buff, "GREAT_JOB");
                return;
            }
            strcpy(buff, "GREAT_JOB");
            return;
        case 2:
            if (depth > 0) {
                strcpy(buff, "WRONG_DIRECTORY");
                strcat(buff, ",");
                strcat(buff, "SIMILAR_OUTPUT");
                return;
            }
            strcpy(buff, "SIMILAR_OUTPUT");

            return;
        case 3:
            if (depth > 0) {
                strcpy(buff, "WRONG_DIRECTORY");
                strcat(buff, ",");
                strcat(buff, "WRONG_OUTPUT");
                return;
            }
            strcpy(buff, "WRONG_OUTPUT");
            return;
        case -2:
            strcpy(buff, "COMPILATION_ERROR");
            if (depth > 0) {
                strcat(buff, ",");
                strcat(buff, "WRONG_DIRECTORY");
                return;
            }
            return;
        case -3:
            strcpy(buff, "TIMEOUT");
            if (depth > 0) {
                strcat(buff, ",");
                strcat(buff, "WRONG_DIRECTORY");
                return;
            }
            return;
        case -6:
            strcpy(buff, "NO_C_FILE");
            return;
        case -5:
            strcpy(buff, "MULTIPLE_DIRECTORIES");
            return;
        default:
            break;
    }
}