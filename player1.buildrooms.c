#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <dirent.h>

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
    int numOutboundConnections;
    struct room* outboundConnections[MAX_CONNECTIONS];
};

/* Function prototypes */
bool IsGraphFull(struct room* arrayOfR);
void AddRandomConnection(struct room* arrayOfR);
struct room* GetRandomRoom(struct room* arrayOfR);
bool CanAddConnectionFrom(struct room* passedR);
bool IsSameRoom(struct room* xR, struct room* yR);
bool ConnectionAlreadyExists(struct room* xR, struct room* yR);
void ConnectRoom(struct room* xR, struct room* yR);
void createRoomFiles(struct room* arrayOfR);

/**************************************************************** 
* Name: IsGraphFull()
* Description: This function will assess whether all rooms have between 3 and 6 outbound connections. If so, it will return true. If not, it will return false
****************************************************************/
bool IsGraphFull(struct room* arrayOfRooms){
    int i = 0;

    for(i = 0; i < MAX_ROOMS_ALLOWED; i++){
        if(arrayOfRooms[i].numOutboundConnections < MIN_CONNECTIONS){           /* I had originally had this as "...numOutboundConnections < MAX_CONNECTIONS", however, each room was then populated with 6 connections. */          
            return false;                                                       /* That line was ensuring that the number of connections was always filled to 6. In order to randomize this to be values between 3-6 inclusive, */          
        }                                                                       /* I set this value to return true as long as it was greater than 3. The AddRandomConnection then adds more connections up to 6 but will not */
    }                                                                           /* fill all of the room connections to be 6 automatically */
    return true;
}

/**************************************************************** 
* Name: AddRandomConnection()
* Description: This function will add a random, valid outbound connection from one room to another room
****************************************************************/
void AddRandomConnection(struct room* arrayOfRooms){
    struct room* A;                                                             /* Struct pointer for first room */
    struct room* B;                                                             /* Struct pointer for second room */

    while(true){                                                                /* While loop to get a random room, from the arrayOfRooms passed in, and verify if an outbound connection can be made */
        A = GetRandomRoom(arrayOfRooms);

        if (CanAddConnectionFrom(A) == true){
            break;
        }
    }

    do{                                                                         /* Do-while loop to get a second room, from the arrayOfRooms passed in */
        B = GetRandomRoom(arrayOfRooms);                                        /* Assess if you can add a connection to second room, second room != first room and they aren't already connected. If false, find new room */
    }
    while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

    ConnectRoom(A, B);                                                          /* Add outbound connection from first room to second */
    ConnectRoom(B, A);                                                          /* Add outbound connectino from second room to first */
}

/**************************************************************** 
* Name: GetRandomRoom()
* Description: This function returns a random room but doesn't validate if a connection can be added
****************************************************************/
struct room* GetRandomRoom(struct room* arrayOfRooms){
    int index = 0;                                                              /* Create a variable to hold the random index */
    index = rand() % MAX_ROOMS_ALLOWED;                                         /* Assign randomized number between 0-6 to the random index variable */

    return &arrayOfRooms[index];                                                /* Return the address of the randomly chosen arrayOfRooms room */
}

/**************************************************************** 
* Name: CanAddConnectionFrom()
* Description: This function returns true if a connection can be added from the passed in room, meaning that it currently has less than 6 outbound connections. Otherwise,
*               this function will return false
****************************************************************/
bool CanAddConnectionFrom(struct room* passedRoom){
    if(passedRoom->numOutboundConnections < MAX_CONNECTIONS){                   /* If the number of connections of the passed in room is 6 or less, return true */
        return true;
    }
    else{                                                                       /* Else return false because no more connections can be added to this room */
        return false;
    }
}

/**************************************************************** 
* Name: IsSameRoom()
* Description: This function returns true if rooms x and y are the same room. Otherwise, it returns false
****************************************************************/
bool IsSameRoom(struct room* x, struct room* y){
    if(strcmp(x->rName, y->rName) == 0){                                        /* Compare the names of the 2 passed in rooms to verify if they are the same name */
        return true;                                                            /* If so, return true */
    }
    else{                                                                       /* Else return false because the names of the 2 passed in rooms are not the same */
        return false;
    }
}

/**************************************************************** 
* Name: ConnectionAlreadyExists()
* Description: This function returns true if a connection from room x to room y already exists. Otherwise, it returns false
****************************************************************/
bool ConnectionAlreadyExists(struct room* x, struct room* y){
    int i = 0;

    for(i = 0; i < x->numOutboundConnections; i++){                             /* For loop to loop through all of the connections of the first passed in room */
        if(strcmp(x->outboundConnections[i]->rName, y->rName) == 0){            /* Compare the name of the connection, at that specific index from the first room, to the name of the passed in second room */
            return true;                                                        /* If they are the same, return true because a connection exists so these rooms should not be connected again */
        }
    }
    return false;                                                               /* Else return false because a connection does not exists between the two passed in rooms */
}

/**************************************************************** 
* Name: ConnectRoom()
* Description: This function connects rooms x and y together, however, it does not check if this connection is valid
****************************************************************/
void ConnectRoom(struct room* x, struct room* y){
    int tempIndex;                                                              /* Variable to hold the current number of connections of the first passed in room */
    tempIndex = x->numOutboundConnections;

    x->outboundConnections[tempIndex] = y;                                      /* Set the first passed in room's connection at the current index, to the second passed in room */
    x->numOutboundConnections = x->numOutboundConnections + 1;                  /* Increment the first passed in room's number of connections. This is incremented after as it is used for the index above */
}

/**************************************************************** 
* Name: createRoomFiles()
* Description: This function first creates a directory called "player1.rooms.<PROCESS ID>. It then creates 7 different room files, which will contain 1 room per file,
*               in the directory that was previously created
****************************************************************/
void createRoomFiles(struct room* arrayOfRooms){
    FILE* myFile;                                                               /* File pointer */
    int i;
    int j;

    char dirName[50];                                                           /* Array to hold the name of the directory */
    memset(dirName, '\0', sizeof(dirName));                                     /* Clear out the array before using it */
    sprintf(dirName, "player1.rooms.%d", getpid());                            /* Print to dirName "player1.rooms.<PROCESS ID>" */
    
    int result = mkdir(dirName, 0755);                                          /* Full permissions for the owner and read and execute permissions for others */

    if(result != 0){                                                            /* If an error occurred with making the directory, print out an error message */
        printf("Error in making directory");
    }

    char tempFile[100];                                                         /* Array to hold the room file name for each of the 7 files that are created */

    for(i = 0; i < MAX_ROOMS_ALLOWED; i++){                                     /* For loop to loop through each room */
        memset(tempFile, '\0', sizeof(tempFile));                               /* Clear out the array before using it */
        sprintf(tempFile, "%s/%s_room", dirName, arrayOfRooms[i].rName);        /* Print to tempFile "player1.rooms.<PROCESS ID>/<ROOM NAME> */
        myFile = fopen(tempFile, "w");                                          /* Open a file, with the name created in the previous line, for writing and point myFile to the file */

        fprintf(myFile, "ROOM NAME: %s\n", arrayOfRooms[i].rName);              /* Print the room name, at the current index, to the file */

        for(j = 0; j < arrayOfRooms[i].numOutboundConnections; j++){            /* For loop to print all of the outbound connection room names, at the current index, to the file */
            int k = j + 1;                                                      /* Variable to display the number of connections in the subsequent line. It must be increased by 1 as the "CONNECTION" starts at 1 */
            fprintf(myFile, "CONNECTION %d: %s\n", k, arrayOfRooms[i].outboundConnections[j]->rName);
        }
            
        fprintf(myFile, "ROOM TYPE: %s\n", arrayOfRooms[i].rType);              /* Print the room type, at the current index, to the file */
        fclose(myFile);                                                         /* Close the current file stream */
    }
}

int main(){
    srand(time(NULL));

    struct room arrayOfRooms[MAX_ROOMS_ALLOWED];                                /* Room struct of 7 rooms named arrayOfRooms */

    int index = 0;                                                              /* Variable to hold the randomized number created for assigning the random room names */
    int i = 0;
    int j = 0;
    int alreadyUsed = 0;                                                        /* Variable to indicate if a random number between 0-9 has already been used */
    int tempStart;                                                              /* Variable to hold the randomized number created for assigning the starting room */
    int tempEnd;                                                                /* Variable to hold the randomized number created for assigning the ending room */


    for(i = 0; i < 7; i++){                                                     /* For loop to assign each room a random room name */
        alreadyUsed = 1;                                                        /* Set alreadyUsed = 1 to enter the while loop */
        while(alreadyUsed){                                                     /* While loop to create a random number and verify if that random number has already been used in a previous iteration */
            alreadyUsed = 0;                                                    /* Set alreadyUsed = 0 */
            index = rand() % TOTAL_ROOMS_AVAIL;                                 /* Assign a random number between 0-9 to the index variable */                              
            
            for(j = 0; j <= i; j++){                                            /* For loop to verify if the name at the current random index has already been assigned */
                if(arrayOfRooms[j].rName == roomNames[index]){
                    alreadyUsed = 1;                                            /* If the name at the current random index has already been used, set alreadyUsed = 1 and create a new random number */
                }
            }
        }
        arrayOfRooms[i].rName = roomNames[index];                               /* Set the name of the room at the current index in the arrayOfRooms to the room name at the random index */
        arrayOfRooms[i].numOutboundConnections = 0;                             /* Set the number of connections of the room at the current index in the arrayOfRooms to 0 */
    }

    tempStart = rand() % MAX_ROOMS_ALLOWED;                                     /* Assign a random number between 0-6 to the tempStart variable */                            
    tempEnd = rand() % MAX_ROOMS_ALLOWED;                                       /* Assign a random number between 0-6 to the tempEnd variable */
    
    while(tempStart == tempEnd){                                                /* While loop to verify if that tempStart and tempEnd values are equal */
        tempEnd = rand() % MAX_ROOMS_ALLOWED;                                   /* If so, assign a new random number between 0-6 to the tempStart variable */
    }

    for(i = 0; i < 7; i++){                                                     /* For loop to assign each room a random room type */
        if(tempStart == i){                                                     /* If the tempStart value equals the current index, set the type of the room at the current index in the arrayOfRooms to "START_ROOM */
            arrayOfRooms[i].rType = roomTypes[0];                           
        }
        else if(tempEnd == i){                                                  /* If the tempEnd value equals the current index, set the type of the room at the current index in the arrayOfRooms to "END_ROOM */
            arrayOfRooms[i].rType = roomTypes[1];
        }
        else{                                                                   /* Else set the type of the type of the room at the current index in the arrayOfRooms to "MID_ROOM" */
            arrayOfRooms[i].rType = roomTypes[2];
        }
    }

    while (IsGraphFull(arrayOfRooms) == false){                                 /* Function to asses whether each room in the arrayOfRooms has between 3-6 connections */
        AddRandomConnection(arrayOfRooms);                                      /* Function to add a new connection to a room in the arrayOfRooms */
    }

    createRoomFiles(arrayOfRooms);                                              /* Function to create a directory and 7 room files within that directory */

    return 0;
}