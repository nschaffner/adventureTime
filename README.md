## adventureTime
This program is a text-based adventure game written in C that allows a user to navigate through a pre-defined set of rooms with the objective of reaching the last room in order to “win”. The game is split up into two C programs: player1.buildrooms.c and player1.adventure.c. The player1.buildrooms.c creates a series of files that hold descriptions of the in-game rooms and how the rooms are connected. The player1.adventure.c program provides an interface for playing the game using the most recently generated rooms. In the game, the player will begin in the "starting room" and will win the game automatically upon entering the "ending room", which causes the game to exit, displaying the path taken by the player. During the game, the player can also enter a command that returns the current time, which utilizes mutexes and multithreading:
- Each time the player1.buildrooms.c program is run, a new directory called "player1.rooms.PROCESS ID" will be created and 7 different room files will be generated in the directory, which will each contain one room per file
- The player1.buildrooms.c program contains 10 hard-coded room names and 3 room types. Each time the program is run, 7 of the 10 names will be randomly assigned to one of each of the 7 rooms generated. The room types will also be randomly assigned each time the program is run, with the "START_ROOM" and "END_ROOM" rooms assigned one time each per run and the "MID_ROOM" assigned to the remaining rooms. Each room must have between 3 and 6 outbound connections, inclusive. The outbound connections will also be randomly assigned each time the program is run and each outbound connection to a room must have a matching connection coming back. An outbound connection cannot point to the room where it is coming from and a room cannot have more than one outbound connection to the same room
- The player1.adventure.c program contains the interface displayed to the user will list what room the user is currently in as well as the list of possible connections that can be followed. It will also display a prompt for the user to enter a room name of one of the room connections displayed where the user would like to move to. If the user types in the exact name of a connection to another room correctly and then hits return, the program will write a new line and then continue running as before. If the user types anything but a valid room name from this location, the program will return an error line and repeat the current location, possible connections and prompt. The path history step count is only incremented when a correct room name is entered. Once the user reaches the "ending room", the program will indicate that the "ending room" has been reached. It will also print out the path the user has taken to get there, the number of steps taken, which is one less than the total number of rooms visited, and a congratulatory message. The exit status code will be set to 0 before the program exits and the rooms directory will not be deleted. During the game, the user can also enter a command that returns the current time

### Deployment
After cloning the repository, please follow the steps below to run the player1.buildrooms.c and player1.adventure.c programs:
1) Log into flip1 and run the following 2 commands in the directory containing the program files:\
    gcc –o player1.adventure player1.adventure.c -lpthread\
    gcc –o player1.buildrooms player1.buildrooms.c

2) Run the following command to create a new instance of the rooms:\
    player1.buildrooms

3) Run the following command to run the game:\
    player1.adventure

4) Follow the prompts presented in the terminal until you are informed that you have won. You can also input the command "time" at any point after starting gameplay to check the current date and time

    - Please note that a file called currentTime.txt will be created once you run the "time" command and will contain a time stamp

### Notes
- The player1.buildrooms.c and player1.adventure.c program was tested on my university's UNIX servers called os1
