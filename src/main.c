#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include "thorCommands/executeThorCommads.c"
#include "utils.h"

Command *commandList = NULL; // start command list
char cmd[MAX_CMD_SIZE];
char dir[MAX_PATH_SIZE];
char *argv[MAX_ARGS];
char history[HISTORY_SIZE][MAX_CMD_SIZE];
char *command_background_list[MAX_COMMANDS_list];
char *pipe_command[MAX_ARGS];
char *token;
int background_processes[MAX_COMMANDS_list];
int history_count = 0;
int bg_process_count = 0;
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
 * The function `executeCommand` executes a series of commands, separated by "&" and "|", and handles
 * input/output redirection and background execution.
 */
void executeCommand()
{
  int commands_count = 0;
  char *command = strtok(cmd, "&");
  while (command != NULL)
  {
    command_background_list[commands_count++] = command;
    command = strtok(NULL, "&");
  }

  for (int i = 0; i < commands_count; i++)
  {
    int background = 0;

    // Check if the command ends with a space.
    if (command_background_list[i][strlen(command_background_list[i]) - 1] == ' ')
    {
      background = 1;
      command_background_list[i][strlen(command_background_list[i]) - 1] = '\0'; // Remove the space.
    }

    // Check if the command is a built-in command.
    token = strtok(command_background_list[i], "|");
    int pipe_count = 0;
    while (token != NULL)
    {
      pipe_command[pipe_count++] = token;
      token = strtok(NULL, "|");
    }
    pipe_command[pipe_count] = NULL;

    int pipes[2];
    int prev_pipe = -1;

    for (int j = 0; j < pipe_count; j++)
    {
      if (pipe(pipes) == -1)
      {
        perror("pipe");
        exit(EXIT_FAILURE);
      }

      argv[0] = strtok(pipe_command[j], " ");
      argv[1] = strtok(NULL, " ");
      argv[2] = NULL;

      pid = fork();
      if (pid == 0)
      {
        // Filho

        if (prev_pipe != -1)
        {
          // Redirect standard input to the previous pipe.
          dup2(prev_pipe, 0);
          close(prev_pipe);
        }

        if (j < pipe_count - 1)
        {
          // Redirect standard output to pipe.
          dup2(pipes[1], 1);
        }

        close(pipes[0]);
        execvp(argv[0], argv);
        executeThorCommand();
        // perror("execvp");
        exit(EXIT_FAILURE);
      }
      else
      {
        // Pai
        if (prev_pipe != -1)
        {
          close(prev_pipe);
        }

        prev_pipe = pipes[0];
        close(pipes[1]);

        if (!background && j == pipe_count - 1)
        {
          // If not in the background and for the last command in the pipe,
          // wait for the child process.
          wait(NULL);
        }
      }
    }

    if (background)
    {
      // Add the PID of the last background process to the list.
      background_processes[bg_process_count++] = pid;
    }
  }
}

/**
 * The sig_child_handler function handles the SIGCHLD signal by waiting for child processes to
 * terminate and updating the list of background processes.
 *
 * @param sig The parameter "sig" is the signal number that triggered the signal handler. In this case,
 * the signal handler is specifically designed to handle the SIGCHLD signal, which is sent to the
 * parent process when a child process terminates or stops.
 */
void sig_child_handler(int sig)
{
  int status;
  pid_t child_pid;
  while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    for (int i = 0; i < bg_process_count; i++)
    {
      if (background_processes[i] == child_pid)
      {
        background_processes[i] = 0;
        break;
      }
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
    return "Unknow";
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
  signal(SIGCHLD, sig_child_handler);
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
