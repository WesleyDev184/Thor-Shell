#ifndef EXECUTETHORCOMMANDS_C
#define EXECUTETHORCOMMANDS_C

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include "../utils.h"

char *command_background_list[MAX_COMMANDS_list];
char *pipe_command[MAX_ARGS];
char *token;
int background_processes[MAX_COMMANDS_list];
int bg_process_count = 0;
int pid;

// declaration of functions
void executeThorCommand();
void setupCommandList();
void executeCommand();
void automaticCompilation(char *program_name);
int fileExists(const char *directory, const char *prefixo);
void showHistory();
void helpCommand();
void addCommand(Command **list, char *name, char *description, CommandFunctions function);

/**
 * The function executes a command by searching for it in a linked list and calling its corresponding
 * function.
 *
 * @return nothing (void).
 */
void executeThorCommand()
{
  Command *currentCommand = commandList;
  while (currentCommand != NULL)
  {
    if (strcmp(currentCommand->name, argv[0]) == 0)
    {
      currentCommand->function();
      return;
    }
    currentCommand = currentCommand->next;
  }

  if (fileExists(dir, cmd) == 1)
  {
    automaticCompilation(cmd);
  }
  else
  {
    printf("Command not found\n");
  }
}

/**
 * The function "setupCommandList" adds commands to a command list with their descriptions and
 * corresponding functions.
 */
void setupCommandList()
{
  addCommand(&commandList, "help", "Show all commands", helpCommand);
  addCommand(&commandList, "history", "see the history of commands already used", showHistory);
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

      if (strcmp(argv[0], "cd") == 0)
      {
        if (chdir(argv[1]) != 0)
        {
          printf("Caminho inválido!\n");
        }
      }
      else
      {
        pid = fork();
        if (pid == 0)
        {
          // son

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
          perror("execvp");
          exit(EXIT_FAILURE);
        }
        else
        {
          // father
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
    }

    if (background)
    {
      // Add the PID of the last background process to the list.
      background_processes[bg_process_count++] = pid;
    }
  }
}

/**
 * The function `automaticCompilation` compiles a C source code file into a binary file if it does not
 * exist or if the source code is newer than the binary file.
 *
 * @param name The parameter "name" is a pointer to a character array that represents the name of the C
 * source code file.
 *
 * @return In this code snippet, if the C source code file does not exist, the function will return
 * without performing any further actions.
 */
void automaticCompilation(char *name)
{
  // Verify that the C source code file exists.
  char prefix_name[MAX_CMD_SIZE];
  snprintf(prefix_name, sizeof(prefix_name), "%s.c", name);

  struct stat source;
  if (stat(prefix_name, &source) == 0)
  {
    // Check if the binary file does not exist or if the source code is newer.
    struct stat binary_stat;
    if (stat(name, &binary_stat) != 0 || difftime(source.st_mtime, binary_stat.st_mtime) > 0)
    {
      // Command to compile the C program (assuming gcc).
      if (fork() == 0)
      {
        execlp("gcc", "gcc", prefix_name, "-o", name, NULL);
        perror("gcc");
        exit(EXIT_FAILURE);
      }
      else
      {
        wait(NULL);
      }
    }
  }
  else
  {
    return;
  }

  char path[MAX_PATH_SIZE];
  snprintf(path, sizeof(path), "./%s", argv[0]);
  execvp(path, argv);
  perror("execvp");
  exit(EXIT_FAILURE);
}

/**
 * The function "fileExists" checks if a .c file with a given prefix exists in a specified directory.
 *
 * @param directory The `directory` parameter is a string that represents the directory path where you
 * want to search for files.
 * @param prefixo The parameter "prefixo" is a string that represents the prefix that the file name
 * should start with.
 *
 * @return The function `fileExists` returns an integer value. It returns 1 if a file with the given
 * prefix and a .c extension is found in the specified directory. It returns 0 if no such file is found
 * or if there is an error opening the directory.
 */
int fileExists(const char *directory, const char *prefixo)
{
  DIR *dp;
  struct dirent *entry;

  // Open the directory
  dp = opendir(directory);
  if (dp == NULL)
  {
    perror("Erro ao abrir o diretório");
    return 0; // Returns false if the directory cannot be opened
  }

  // Cycle through the files in the directory
  while ((entry = readdir(dp)))
  {
    // Checks if the file name starts with the prefix
    if (strncmp(prefixo, entry->d_name, strlen(prefixo)) == 0)
    {
      // Checks if the file has a .c extension
      size_t len = strlen(entry->d_name);
      if (len >= 2 && strcmp(entry->d_name + len - 2, ".c") == 0)
      {
        closedir(dp);
        return 1; // Returns true if a .c file with the prefix is ​​found
      }
    }
  }

  // Closes the directory and returns false if no .c file with the prefix is ​​found
  closedir(dp);
  return 0;
}

/**
 * The function "showHistory" prints the command history, including the index and the command itself.
 */
void showHistory()
{
  printf("command history:\n");
  for (int i = 0; i < history_count; i++)
  {
    printf("%d: %s\n", i + 1, history[i]);
  }
  exit(EXIT_SUCCESS);
}

/**
 * The function "helpCommand" prints a list of available commands along with their descriptions.
 */
void helpCommand()
{
  printf("Commands:\n\n");
  Command *currentCommand = commandList;
  while (currentCommand != NULL)
  {
    printf("%s - %s\n", currentCommand->name, currentCommand->description);
    currentCommand = currentCommand->next;
  }
  printf("\n");
  exit(EXIT_SUCCESS);
}

/**
 * The function adds a new command to a linked list of commands.
 *
 * @param list A pointer to a pointer of type Command. This is used to keep track of the head of the
 * linked list of commands.
 * @param name A string representing the name of the command.
 * @param description The "description" parameter is a string that describes the purpose or
 * functionality of the command. It provides additional information about what the command does.
 * @param function The "function" parameter is a pointer to a function that takes no arguments and
 * returns void. This function will be associated with the command being added to the list.
 */
void addCommand(Command **list, char *name, char *description, CommandFunctions function)
{
  Command *newCommand = (Command *)malloc(sizeof(Command));
  newCommand->name = strdup(name);
  newCommand->description = strdup(description);
  newCommand->function = function;
  newCommand->next = *list;
  *list = newCommand;
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

#endif