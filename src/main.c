#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "thorCommands/executeThorCommads.c"
#include "utils.h"

Command *commandList = NULL; // start command list
char cmd[MAX_CMD_SIZE];
char dir[MAX_PATH_SIZE];
char *argv[MAX_ARGS];
char history[HISTORY_SIZE][MAX_CMD_SIZE];
int history_count = 0;
int pid;

void welcomeScreen()
{
  printf("      ------------------     \n");
  printf("     |                  |    \n");
  printf("     |    Thor Shell    |    \n");
  printf("     |                  |    \n");
  printf("      ------------------     \n");
  printf("             |  |            \n");
  printf("             |  |            \n");
  printf("             |  |            \n");
  printf("             |  |            \n");
  printf("             |  |            \n");
  printf("              --             \n");
  printf("    Licensed under GPLv3:    \n");
  printf("\n");
}

/**
 * The function executes a command by either changing the current directory or creating a child process
 * to execute the command.
 */
void executeCommand()
{

  argv[0] = strtok(cmd, " ");
  argv[1] = strtok(NULL, " ");
  argv[2] = NULL;

  if (strcmp(argv[0], "cd") == 0)
  {
    if (chdir(argv[1]) != 0)
      printf("Caminho inválido!\n");
  }
  else
  {
    pid = fork();
    if (pid == 0)
    {
      if (execvp(argv[0], argv) == -1)
      {
        executeThorCommand();
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      wait(NULL);
    }
  }
}

/**
 * The function getUsername retrieves the username of the current user.
 *
 * @return The function `getUsername()` returns a pointer to a character array (string) containing the
 * username of the current user.
 */
char *getUsername()
{
  struct passwd *pw;
  uid_t uid;

  uid = geteuid();
  pw = getpwuid(uid);

  if (pw != NULL)
    return pw->pw_name;
  else
    return "";
}

/**
 * The function getCurrentDirectory returns the current directory path as a string, or "Unknown" if the
 * path cannot be determined.
 *
 * @return The function getCurrentDirectory() returns a pointer to a character array representing the
 * current directory. If the current directory can be successfully retrieved using the getcwd()
 * function, the function returns the directory path. Otherwise, if the getcwd() function fails, the
 * function returns the string "Unknown".
 */
char *getCurrentDirectory()
{

  if (getcwd(dir, sizeof(dir)) != NULL)
    return dir;
  else
    return "Unknown";
}

/**
 * The main function sets up a command line interface, prompts the user for commands, stores them in
 * history, and executes them.
 *
 * @return The main function is returning an integer value of 0.
 */
int main(void)
{
  setupCommandList();
  system("clear");
  welcomeScreen();
  while (1)
  {
    // Prompt the user for a command
    char *username = getUsername();
    char *currentDirectory = getCurrentDirectory();
    printf("\x1b[35m%s@Thor\x1b[0m:\x1b[36m%s\x1b[0m$ ", username, currentDirectory);
    fgets(cmd, 511, stdin);
    cmd[strlen(cmd) - 1] = 0;

    // Check if the user wants to exit
    if (strcmp(cmd, "exit") == 0)
    {
      exit(EXIT_SUCCESS);
    }
    else
    {
      // Store the command in history
      if (history_count < HISTORY_SIZE)
      {
        strcpy(history[history_count], cmd);
        history_count++;
      }
      else
      {
        // If history is full, remove the oldest command
        for (int i = 0; i < HISTORY_SIZE - 1; i++)
        {
          strcpy(history[i], history[i + 1]);
        }
        strcpy(history[HISTORY_SIZE - 1], cmd);
      }

      // Execute the command
      executeCommand();
    }
  }
  return 0;
}
