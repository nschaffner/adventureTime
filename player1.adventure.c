#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <pthread.h>

/* Global variables */
#define TOTAL_ROOMS_AVAIL 10
#define MAX_ROOMS_ALLOWED 7
#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6

/* I referenced the thread conversation in this link to enumerate a bool type: https://stackoverflow.com/questions/4159713/how-to-use-boolean-datatype-in-c */
typedef enum {false=0, true=1} bool;

/* Room names */
char* roomNames[TOTAL_ROOMS_AVAIL] = {"alpha", "betta", "gamma", "delta", "epsil", "zetta", "ettaa", "theta", "iotta", "kappa"};

/* Room types */
char* roomTypes[3] = {"START_ROOM", "END_ROOM", "MID_ROOM"};

/* Room struct */
struct room{
    char* rName;
    char* rType;
    int whatRoom;
    int numOutboundConnections;
    struct room* outboundConnections[MAX_CONNECTIONS];
};

/* Room path struct */
struct roomPath{
    int rNameIndex;
};

/* Function prototypes */
void newestDir(char* newDirN);
void getFileInfo(struct room *arrayOfR, char* newestDirN);
void playGame(struct room* arrOfR, struct roomPath* arrRP);
void makeThread();
void* writeTime();
void readTime();

/**************************************************************** 
* Name: newestDir()
* Description: This function will open up the current directory and then loop through each entry inside it. The function will search for directories with the
*               pathname "player1.rooms.". If a match is found, the function will get the last modified timestamp of it, and record the name and timestamp. 
*               It will then compare the timestamp with the previous entry and store the newer version between the two. This process will continue until all
*               the directories inside the current directory have been assessed. The newestDirName array that was passed into the function will point to the 
*               newest directory with the pathname "player1.rooms." when the current directory is closed at the end of the function call
****************************************************************/
void newestDir(char* newestDirName){
    int newestDirTime = -1;                                                     /* Variable to hold the time of the newest directory examined */
    char targetDirPrefix[32] = "player1.rooms.";                               /* Prefix function is searching for */

    memset(newestDirName, '\0', sizeof(newestDirName));                         /* Clear out the array before using it */

    DIR* dirToCheck;                                                            /* Directory pointer to hold directory we're starting in */
    struct dirent *fileInDir;                                                   /* Holds the current subdir of the starting directory */
    struct stat dirAttributes;                                                  /* Variable to hold the information we've gained about subdir */

    dirToCheck = opendir(".");                                                  /* Open up the directory this program was run in */

    if(dirToCheck > 0)                                                          /* Verify whether directory could be opened */
    {
        while((fileInDir = readdir(dirToCheck)) != NULL)                        /* Check through each entry in directory */
        {
            if(strstr(fileInDir->d_name, targetDirPrefix) != NULL)              /* If the entry has the prefix stated above */
            {
                stat(fileInDir->d_name, &dirAttributes);                        /* Get the attributes of the entry */

                if((int)dirAttributes.st_mtime > newestDirTime)                 /* If this time is bigger than the previously saved entry, replace that entry with the newer entry */
                {
                    newestDirTime = (int)dirAttributes.st_mtime;
                    memset(newestDirName, '\0', sizeof(newestDirName));         /* Clear out the array before copying in the new directory name */
                    strcpy(newestDirName, fileInDir->d_name);
                }
            }
        }
    }

    closedir(dirToCheck);                                                       /* Close the directory that was opened at the beginning of the function */
}

/**************************************************************** 
* Name: getFileInfo()
* Description: This function will open the most recent rooms directory that has been passed into the function. It will then open each file within that directory and
*               read each line of the selected file. The strncmp() function will be used and, depending on the line, the room at the current index of the arrayOfRooms
*               will be assigned the corresponding room name or room type. The function will close the directory once all of the files have been read. The function will
*               then repoen the most recent rooms directory that has been passed into the function to assign the connection names and number of connections for the room
*               at the current index of the arrayOfRooms room struct. The function will then close the directory once all of the files have been read
****************************************************************/
void getFileInfo(struct room *arrayOfRooms, char* newestDirName){
    DIR *correctDir;                                                            /* Directory pointer to hold the directory we're starting in */
    struct dirent *fileInDir;                                                   /* Holds the current file of the starting directory */
    char tempFile[100];                                                         /* Variable to hold the current file name */
    char tempString[100];                                                       /* Variable to hold the current string of charaters that is read from the file line */
    char *ret;                                                                  /* Variable to hold the pointer to a character in the current line of the current file being read */
    char ch = ':';                                                              /* Variable to hold the ":" character, which will be used when reading through a line of a file */
    FILE *myFile;                                                               /* File pointer */
    int arrayIndex = 0;                                                         /* Variable to hold the current index of the arrayOfRooms */
    int connectionIndex = 0;                                                    /* Variable to hold the current index of the connection of the applicable room of the arrayOfRooms */
    int i =0;

    correctDir = opendir(newestDirName);                                        /* Open up the newest room directory */
    if(correctDir){                                                             /* Verify whether the directory could be opened */
        while((fileInDir = readdir(correctDir)) != NULL){                       /* Check through each file in the directory */
            if(!strcmp(fileInDir->d_name, ".") || !strcmp(fileInDir->d_name, "..")){
                ;                                                               /* Don't do anything with that file because it is not a "player1.rooms." file */
            }
            else{
                memset(tempFile, '\0', sizeof(tempFile));                       /* Clear out the array before copying in the new file name */
                sprintf(tempFile, "%s/%s", newestDirName, fileInDir->d_name);   /* Print to tempFile "player1.rooms.<PROCESS ID>/<ROOM NAME> */
                myFile = fopen(tempFile, "r");                                  /* Open a file, with the name created in the previous line, for reading, and point myFile to the file */

                while(fgets(tempString, 100, myFile) != NULL){                  /* Read in a line from the myFile and store it in tempString. I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm */
                    ret = strchr(tempString, ch);                               /* Search for the ":" char, which ret will now point to. I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_strchr.htm */

                    if(strncmp(tempString, "ROOM N", 6) == 0){                  /* Assess if the first 6 characters of the line are "ROOM N". I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_strncmp.htm*/
                        for(i = 0; i < TOTAL_ROOMS_AVAIL; i++){                 /* For loop to search through all the room names to find the correct name that the room at this index should be assigned to */
                            if(strncmp(ret+2, roomNames[i], 2) == 0){           /* Progress the pointer to the character ":" 2 more characters to point to the first character of the name. Compare its first two characters */
                                arrayOfRooms[arrayIndex].rName = roomNames[i];  /* the first two characters of the room name at the given index. If they match, assign the room's name at this index to the appropriate room name */
                            }
                        }
                    }
                    else if(strncmp(tempString, "ROOM T", 6) == 0){             /* Assess if the first 6 characters of the line are "ROOM T." I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm */
                        for(i = 0; i < 3; i++){                                 /* For loop to search through all the room types to find the correct type that the room at this index should be assigned to */
                            if(strncmp(ret+2, roomTypes[i], 1) == 0){           /* Progress the pointer to the character ":" 2 more characters to point to the first character of the type. Compare its first character */
                                arrayOfRooms[arrayIndex].rType = roomTypes[i];  /* the first character of the room type at the given index. If they match, assign the room's type at this index to the appropriate room type */
                                if(i == 0){                                     /* If the index of the room types is 0, update the room's whatRoom at this index to 0 */
                                    arrayOfRooms[arrayIndex].whatRoom = 0;
                                }
                                else if(i == 1){                                /* If the index of the room types is 1, update the room's whatRoom at this index to 1 */
                                    arrayOfRooms[arrayIndex].whatRoom = 1;
                                }
                            }
                        }
                    }
                }

                fclose(myFile);                                                 /* Close the file that was opened at the beginning of the while loop */
                arrayIndex++;                                                   /* Increment the variable for the arrayOfRooms index */
            }
        }
    }
    closedir(correctDir);                                                       /* Close the directory that was opened */

    arrayIndex = 0;                                                             /* Reset the variable for the arrayOfRooms index */

    correctDir = opendir(newestDirName);                                        /* Open up the newest room directory, which was previously used above */                                    
    if(correctDir){                                                             /* Verify whether the directory could be opened */
        while((fileInDir = readdir(correctDir)) != NULL){                       /* Check through each file in the directory */
            if(!strcmp(fileInDir->d_name, ".") || !strcmp(fileInDir->d_name, "..")){
                ;                                                               /* Don't do anything with that file because it is not a "player1.rooms." file */
            }
            else{
                memset(tempFile, '\0', sizeof(tempFile));                       /* Clear out the array before copying in the new file name */
                sprintf(tempFile, "%s/%s", newestDirName, fileInDir->d_name);   /* Print to tempFile "player1.rooms.<PROCESS ID>/<ROOM NAME> */
                myFile = fopen(tempFile, "r");                                  /* Open a file, with the name created in the previous line, for reading, and point myFile to the file */

                while(fgets(tempString, 100, myFile) != NULL){                  /* Read in a line from the myFile and store it in tempString. I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm */
                    ret = strchr(tempString, ch);                               /* Search for the ":" char, which ret will now point to. I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_strchr.htm */
                    
                    if(strncmp(tempString, "C", 1) == 0){                       /* Assess if the first character of the line is "C". I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_strncmp.htm*/
                        for(i = 0; i < MAX_ROOMS_ALLOWED; i++){                 /* For loop to search through all the room names to find the correct name that the room at this index should be assigned to */
                            if(strncmp(ret+2, arrayOfRooms[i].rName, 2) == 0){  /* Progress the pointer to the character ":" 2 more characters to point to the first character of the name. Compare its first two characters */
                                arrayOfRooms[arrayIndex].outboundConnections[connectionIndex] = &arrayOfRooms[i];   /* Assign the connection at that index of the room of the given index to the appropriate room in arrayOfRooms */
                                arrayOfRooms[arrayIndex].numOutboundConnections = arrayOfRooms[arrayIndex].numOutboundConnections + 1;  /* Increment the number of connections for the room at this index */
                                connectionIndex++;                              /* Increment the variable holding the current index of the connection of the applicable room of the arrayOfRooms */
                            }
                        }
                    }
                }

                fclose(myFile);                                                 /* Close the file that was opened at the beginning of the while loop */
                arrayIndex++;                                                   /* Increment the variable for the arrayOfRooms index */
                connectionIndex = 0;                                            /* Reset the variable holding the current index of the connection of the applicable room of the arrayOfRooms */
            }
        }
    }
    closedir(correctDir);                                                       /* Close the directory that was opened */
}

/**************************************************************** 
* Name: playGame()
* Description: This function will find the starting room for this version fo the building of the rooms. The interface displayed to the user will list what room the user
*               is currently in as well as the list of possible connections that can be followed. It will also display a prompt for the user to enter a room name of one 
*               of the room connections displayed where the user would like to move to. If the user types in the exact name of a connection to another room correctly and 
*               then hits return, the program will write a new line and then continue running as before. If the user types anything but a valid room name from this location, 
*               the program will return an error line and repeat the current location, possible connections and prompt. The path history step count is only incremented when a correct 
*               room name is entered. Once the user reaches the "ending room", the program will indicate that the "ending room" has been reached. It will also print out the path the 
*               user has taken to get there, the number of steps taken, which is one less than the total number of rooms visited, and a congratulatory message
****************************************************************/
void playGame(struct room* arrayOfRooms, struct roomPath* arrayRoomsPath){
    int numSteps = 0;                                                           /* Variable to hold the number of steps taken by the user in the current game */
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    int tempIndex = -1;                                                         /* Variable to hold the value of the current rNameIndex of the arrayRoomsPath */
    int foundRoom = 0;                                                          /* Variable to indicate if the user typed in the correct name of a room */
    int currentRoom;                                                            /* Variable to hold the current room's index */                   
    int notEndRoom = 1;                                                         /* Variable to indicate if the user has reached the "END_ROOM" */

    for(i = 0; i < MAX_ROOMS_ALLOWED; i++){                                     /* For loop to find the starting room for this version of the building of the roooms */
        if(arrayOfRooms[i].whatRoom == 0){                                      /* If the room's whatRoom has been assigned to the value 0, it is the starting room */
            currentRoom = i;                                                    /* Update currentRoom */
        }
    }

    printf("CURRENT LOCATION: %s\n", arrayOfRooms[currentRoom].rName);          /* Print out the starting room name */
    printf("POSSIBLE CONNECTIONS: ");                                           
    for(i = 0; i < arrayOfRooms[currentRoom].numOutboundConnections; i++){      /* For loop to print out all of the starting room's connections*/
        if(i == (arrayOfRooms[currentRoom].numOutboundConnections - 1)){        /* If the last connection has been reached in the current room, print out the connection name followed by a period and new line character */
            printf("%s.\n", arrayOfRooms[currentRoom].outboundConnections[i]->rName);
        }
        else{                                                                   /* Else print out the connection name followed by a comma and space */
        printf("%s, ", arrayOfRooms[currentRoom].outboundConnections[i]->rName);
        }
    }
    
    /* For the following section, I referenced page 102 of the "All Block 2 Lectures in One PDF.pdf" titled "Getting Input From the User: userinput.c" */
    int numCharsEntered = -5;                                                   /* Variable to hold how many chars are entered by the user */                                                                                                
    int currChar = -5;                                                          /* Variable to track where you are when you print out each char */
    size_t bufferSize = 0;                                                      /* Variable to hold how large the allocated buffer is */
    char* lineEntered = NULL;                                                   /* Variable to point to a buffer allocated by getline() that holds the user's entered string + \n + \0 */

    while(notEndRoom)                                                           /* While loop to until the user has reached the "END ROOM" */
    {
        printf("WHERE TO? >");                                          
        numCharsEntered = getline(&lineEntered, &bufferSize, stdin);            /* Get a line from the user */

        if(strncmp(lineEntered, "time", 4) == 0){                               /* If the user enters "time", call the makeThread() function to create a second thread to write the time */
            makeThread();
            readTime();                                                         /* Call the readTime() function to print out the new current time in currentTime.txt */
            foundRoom = 1;                                                      /* Update foundRoom variable to 1 in order to bypass the subsequent if statement below */
        }

        if(numCharsEntered < 6){                                                /* If the number of chars entered by the user is less than 6, set the variable to 6 */
            numCharsEntered = 6;                                                
        }

        for(i = 0; i < arrayOfRooms[currentRoom].numOutboundConnections; i++){  /* For loop to loop through each connection of the room at the current index */
            if(strncmp(lineEntered, arrayOfRooms[currentRoom].outboundConnections[i]->rName, (numCharsEntered - 1)) == 0){  /* Compare first 5 chars of user input with connection name at that index of room at current index */
                for(j = 0; j < 7; j++){                                         /* For loop to loop through each room of arrayOfRooms */
                    if(strncmp(arrayOfRooms[j].rName, arrayOfRooms[currentRoom].outboundConnections[i]->rName, 5) == 0){    /* Compare first 5 chars of room name at that index with connection name at index i of room at current index*/
                        currentRoom = j;                                        /* Update currentRoom */

                        for(m = 0; m < 10; m++){                                /* For loop to look through each of the room names */
                            if(strncmp(roomNames[m], arrayOfRooms[currentRoom].rName, 5) == 0){     /* If the name of the new currentRoom matches the room name at the given index */
                                arrayRoomsPath[numSteps].rNameIndex = m;                            /* Assign the rNameIndex at the current index to index of the matching room name */
                            }    
                        }

                        numSteps++;                                             /* Increment the number of steps the user has taken */

                        if(arrayOfRooms[currentRoom].whatRoom == 1){            /* If the room's whatRoom variable at the current index has been assigned the value 1, the user has reached the "END ROOM" */
                            printf("\n");
                            printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
                            printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numSteps);
                            for(m = 0; m < numSteps; m++){                      /* For loop to print out each room name that was traveled to during this iteration of the game */
                                tempIndex = arrayRoomsPath[m].rNameIndex;
                                printf("%s\n", roomNames[tempIndex]);
                            }
                            return;                                             /* Return to int main() */
                        }

                        printf("\n");
                        printf("CURRENT LOCATION: %s\n", arrayOfRooms[currentRoom].rName);      /* Print the current room name */
                        printf("POSSIBLE CONNECTIONS: ");
                    
                        for(k = 0; k < arrayOfRooms[currentRoom].numOutboundConnections; k++){  /* For loop to print out all of the current room's connections */
                            if(k == (arrayOfRooms[currentRoom].numOutboundConnections - 1)){
                                printf("%s.\n", arrayOfRooms[currentRoom].outboundConnections[k]->rName);
                            }
                            else{
                                printf("%s, ", arrayOfRooms[currentRoom].outboundConnections[k]->rName);
                            }
                        }
                        foundRoom = 1;                                                          /* Update foundRoom variable to 1 in order to bypass the subsequent if statement */
                        break;
                    }
                }
            }
        }

        if(!foundRoom){                                                         /* If statement to print out the the statements indicating that the user did not enter a valid room name */
            printf("\n");
            printf("HUH? I DONT'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
            printf("CURRENT LOCATION: %s\n", arrayOfRooms[currentRoom].rName);  /* Print the current room name */
            printf("POSSIBLE CONNECTIONS: ");
                    
            for(i = 0; i < arrayOfRooms[currentRoom].numOutboundConnections; i++){  /* For loop to print out all of the current room's connections */
                if(i == (arrayOfRooms[currentRoom].numOutboundConnections - 1)){
                    printf("%s.\n", arrayOfRooms[currentRoom].outboundConnections[i]->rName);
                }
                else{
                    printf("%s, ", arrayOfRooms[currentRoom].outboundConnections[i]->rName);
                }
            }
        }

        free(lineEntered);                                                      /* Free the memory allocated by getline() */
        lineEntered = NULL;                                                     /* Reset the pointer */
        foundRoom = 0;                                                          /* Reset foundRoom to false */
    }
}

/**************************************************************** 
* Name: makeThread()
* Description: This function will create a mutex variable and thread variable. It will then lock the mutex variable and create a second thread that will call the writeTime() function.
*               Once the writeTime() function has returned, the mutex variable will be unlocked and the function will wait for second thread to terminate
****************************************************************/
void makeThread(){
    pthread_mutex_t firstThread = PTHREAD_MUTEX_INITIALIZER;                    /* Create and initialize the firstThread mutex variable */
    pthread_t secondThread;                                                     /* Create a thread called secondThread */

    pthread_mutex_lock(&firstThread);                                           /* Lock the firstThread mutex variable */

    pthread_create(&secondThread, NULL, writeTime, NULL);                       /* Create a new thread and call the writeTime() function to write the current time to "currentTime.txt". I used NULL for the 2nd and 4th arguments */
                                                                                /* as no arguments were needed. I referenced the following regarding the pthread_create() function: https://www.geeksforgeeks.org/multithreading-c-2/ */
    pthread_mutex_unlock(&firstThread);                                         /* Unlock the firstThread mutex variable */

    pthread_join(secondThread, NULL);                                           /* Wait for secondThread to terminate. I used NULL for 2nd arugment as no argument was needed. I referenced: https://www.geeksforgeeks.org/multithreading-c-2/ */                          
}                                                                               

/**************************************************************** 
* Name: writeTime()
* Description: This function will either a file called "currentTime.txt" and input the current time into the file. If the file already exists, the function will replace the old values with the current time
****************************************************************/
void* writeTime(){
    FILE* myFile;                                                               /* File pointer */
    char currentTime[100];                                                      /* Array to hold the current time */
    time_t uneditedTime;                                                        /* Variable to hold the system time value. I referenced: http://www.cplusplus.com/reference/ctime/localtime/ */    
    struct tm *tm;                                                              /* Pointer to a tm struct. I referenced: http://www.cplusplus.com/reference/ctime/localtime/ */

    memset(currentTime, '\0', sizeof(currentTime));                             /* Clear out the array before using it */

    time(&uneditedTime);                                                        /* Returned value points to an internal object. I referenced: http://www.cplusplus.com/reference/ctime/localtime/ */
    tm = localtime(&uneditedTime);                                              /* Get the current local time and point the tm struct point to this address. I referenced: http://www.cplusplus.com/reference/ctime/localtime/ */

    strftime(currentTime, 100, "%l:%M%P %A, %B %d, %Y", tm);                    /* For the arguments, I referenced: http://man7.org/linux/man-pages/man3/strftime.3.html and https://www.geeksforgeeks.org/strftime-function-in-c/ */
                                                                                
    myFile = fopen("currentTime.txt", "w");                                     /* Open a file called "currentTime.txt" for writing and point myFile to the file */
    fprintf(myFile, "%s\n", currentTime);                                       /* Print the current time to the file */
    fclose(myFile);                                                             /* Close the current file stream */
}

/**************************************************************** 
* Name: readTime()
* Description: This function will print out the current time stored in "currentTime.txt"
****************************************************************/
void readTime(){
    FILE* myFile;                                                               /* File pointer */
    char tempString[100];                                                       /* Variable to hold the current string of charaters that is read from the file line */

    myFile = fopen("currentTime.txt", "r");                                     /* Open a file called "currentTime.txt" for reading, and point myFile to the file */
    fgets(tempString, 100, myFile);                                             /* Read in the first line from the myFile and store it in tempString. I referenced: https://www.tutorialspoint.com/c_standard_library/c_function_fgets.htm */
    printf("\n%s\n", tempString);                                               /* Print out the current time stored in tempString */      
}

int main(){
    srand(time(NULL));

    char newestDirName[256];                                                    /* Array to hold the name of the newest directory that contains "player1.rooms.". I referenced "2.4 Manipulating Directories" */

    newestDir(newestDirName);                                                   /* Function to find the newest directory that contains "player1.rooms." */

    struct room arrayOfRooms[MAX_ROOMS_ALLOWED];                                /* Room struct of 7 rooms named arrayOfRooms */
    struct roomPath arrayRoomsPath[100];                                        /* RoomPath struct of 100 roomPaths name arrayRoomsPath */

    int i = 0;
    int j = 0;

    for(i = 0; i < 7; i++){                                                     /* For loop to iterate through each of the rooms of arrayOfRooms */
        arrayOfRooms[i].numOutboundConnections = 0;                             /* Set the number of connections of the room at the current index in the arrayOfRooms to 0 */
        arrayOfRooms[i].whatRoom = 2;                                           /* Set the whatRoom variable of the room at the current index in the arrayOfRooms to 0 */
    }

    getFileInfo(arrayOfRooms, newestDirName);                                   /* Function to get the information from each file in the newest directory */

    for(i = 0; i < 100; i++){                                                   /* For loop to iterate through each of the roomPaths of arrayRoomsPath */
        arrayRoomsPath[i].rNameIndex = 0;                                       /* Set the rNameIndex of the roomPath at the current index in the arrayRoomsPath to 0 */
    }

    playGame(arrayOfRooms, arrayRoomsPath);                                     /* Function to play the game */

    return 0;
}