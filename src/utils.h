#ifndef UTILS_H
#define UTILS_H

#define HISTORY_SIZE 100
#define MAX_ARGS 3
#define MAX_CMD_SIZE 512
#define MAX_PATH_SIZE 4096

// structs
typedef void (*CommandFunctions)(void); // Function pointer type definition

typedef struct command
{
  char *name;
  char *description;
  CommandFunctions function;
  struct command *next;
} Command; // Command list struct

// global variables
extern char cmd[MAX_CMD_SIZE];                   // Command entered by user
extern char dir[MAX_PATH_SIZE];                  // Current directory path
extern char commandPath[MAX_PATH_SIZE];          // Command path
extern char *argv[MAX_ARGS];                     // Vector to store command arguments
extern char history[HISTORY_SIZE][MAX_CMD_SIZE]; // Matrix to store historical commands
extern int history_count;                        // Historical command counter
extern Command *commandList;                     // Command list

#endif