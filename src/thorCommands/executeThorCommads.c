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
#include <ctype.h>
#include "../utils.h"

char *process_background_list[MAX_COMMANDS_list];
char *token_precess;
int background_processes_exec[MAX_COMMANDS_list];
int bg_process_count = 0;
int pid;

// declaration of functions
void executeThorCommand();
void setupCommandList();
void executeCommand();
void executeSingleCommand(char *command);
void checkAndRemoveTrailingSpace(char *command, int *background);
void splitCommand(char *command, char *command_list[], int *command_count);
void processCommand(char *command_list[], int command_count, int background);
void handleCdCommand(char *argv[]);
void executeChildProcess(char *argv[], int prev_pipe, int pipes[], int j, int command_count);
void automaticCompilation(char *program_name);
int fileExists(const char *directory, const char *prefix);
void showHistory();
void helpCommand();
void addCommand(Command **list, char *name, char *description, CommandFunctions function);
void sig_child_handler(int sig);

/**
 * The function executes a command by searching for it in a linked list and calling its corresponding
 * function.
 *
 * @return nothing (void).
 */
void executeThorCommand()
{
  Command *currentCommand = thorCommandList;
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
  addCommand(&thorCommandList, "help", "Show all commands", helpCommand);
  addCommand(&thorCommandList, "history", "see the history of commands already used", showHistory);
}

/**
 * The function executeCommand() separates a string of commands by '&' and processes each command
 * individually.
 */
void executeCommand()
{
  int commands_count = 0;
  char *command = strtok(cmd, "&");

  // Loop to separate commands by '&'
  while (command != NULL)
  {
    process_background_list[commands_count++] = command;
    command = strtok(NULL, "&");
  }

  // Loop to process each command
  for (int i = 0; i < commands_count; i++)
  {
    executeSingleCommand(process_background_list[i]);
  }
}

/**
 * The function executes a single command, checking for background execution and handling pipes if
 * necessary.
 *
 * @param command A pointer to a character array representing the command to be executed.
 */
void executeSingleCommand(char *command)
{
  int background = 0;
  checkAndRemoveTrailingSpace(command, &background);

  int command_count = 0;
  char *command_list[command_count + 1];

  splitCommand(command, command_list, &command_count);

  processCommand(command_list, command_count, background);
}

/**
 * The function checks if the last character of a given command is a space, and if so, removes it and
 * sets a background flag.
 *
 * @param command A pointer to a character array representing a command.
 * @param background A pointer to an integer variable that indicates whether the command should be
 * executed in the background or not. If the value of *background is 1, it means the command should be
 * executed in the background. If the value is 0, it means the command should be executed in the
 * foreground.
 */
void checkAndRemoveTrailingSpace(char *command, int *background)
{
  if (command[strlen(command) - 1] == ' ')
  {
    *background = 1;
    command[strlen(command) - 1] = '\0'; // Remove space.
  }
}

/**
 * The function splits a command string by the pipe character "|" and stores the individual commands in
 * an array.
 *
 * @param command A string containing the command to be split by pipe characters (|).
 * @param command_list An array of strings where each element will store a command separated by a pipe
 * symbol (|).
 * @param command_count The `command_count` parameter is a pointer to an integer variable that will store the
 * number of pipe commands found in the `command` string.
 */
void splitCommand(char *command, char *command_list[], int *command_count)
{
  char *token_precess;
  token_precess = strtok(command, "|");
  *command_count = 0;
  while (token_precess != NULL)
  {
    command_list[(*command_count)++] = token_precess;
    token_precess = strtok(NULL, "|");
  }
  command_list[*command_count] = NULL;
}

/**
 * The function `processCommand` executes a series of commands separated by pipes, creating child
 * processes for each command and handling the `cd` command separately.
 *
 * @param command_list An array of strings representing the commands to be executed in the pipeline.
 * Each string represents a single command with its arguments separated by spaces.
 * @param command_count The `command_count` parameter represents the number of pipes in the command. It
 * indicates how many separate commands are being piped together.
 * @param background An integer variable indicating whether the command should be executed in the
 * background or not. If background is 1, the command should be executed in the background. If
 * background is 0, the command should be executed in the foreground.
 */
void processCommand(char *command_list[], int command_count, int background)
{
  int pipes[2];
  int prev_pipe = -1;
  int pid;

  for (int j = 0; j < command_count; j++)
  {
    if (pipe(pipes) == -1)
    {
      perror("pipe");
      exit(EXIT_FAILURE);
    }

    argv[0] = strtok(command_list[j], " ");
    argv[1] = strtok(NULL, " ");
    argv[2] = NULL;

    if (strcmp(argv[0], "cd") == 0)
    {
      handleCdCommand(argv);
    }
    else
    {
      pid = fork();
      if (pid == 0)
      {
        executeChildProcess(argv, prev_pipe, pipes, j, command_count);
      }
      else
      {
        if (prev_pipe != -1)
        {
          close(prev_pipe);
        }

        prev_pipe = pipes[0];
        close(pipes[1]);

        if (!background && j == command_count - 1)
        {
          wait(NULL);
        }
      }
    }
  }

  if (background)
  {
    // Adicione o PID do último processo em segundo plano à lista.
    background_processes_exec[bg_process_count++] = pid;
  }
}

/**
 * The function handles the "cd" command by changing the current directory to the specified path.
 *
 * @param argv An array of strings, where each string represents a command-line argument. The first
 * element (argv[0]) is typically the name of the program being executed, and the subsequent elements
 * (argv[1], argv[2], etc.) are the arguments passed to the program. In this case, argv
 */
void handleCdCommand(char *argv[])
{
  if (chdir(argv[1]) != 0)
  {
    printf("Caminho inválido!\n");
  }
}

/**
 * The function executes a child process with the given arguments and handles input/output redirection
 * using pipes.
 *
 * @param argv An array of strings representing the command and its arguments to be executed in the
 * child process.
 * @param prev_pipe The "prev_pipe" parameter is the file descriptor of the previous pipe. It is used
 * to redirect the standard input of the child process to the output of the previous command in the
 * pipeline. If there is no previous pipe, the value of "prev_pipe" will be -1.
 * @param pipes The "pipes" parameter is an array of file descriptors used for inter-process
 * communication. It is used to pass data between the parent process and its child processes. The array
 * contains two file descriptors: pipes[0] is used for reading from the pipe, and pipes[1] is used for
 * writing
 * @param j The parameter "j" represents the index of the current child process in the array of child
 * processes.
 * @param command_count The `command_count` parameter represents the total number of pipes in the pipeline.
 */
void executeChildProcess(char *argv[], int prev_pipe, int pipes[], int j, int command_count)
{
  if (prev_pipe != -1)
  {
    dup2(prev_pipe, 0);
    close(prev_pipe);
  }

  if (j < command_count - 1)
  {
    dup2(pipes[1], 1);
  }

  close(pipes[0]);

  // Tokenize the path variable to get individual paths
  char *token = strtok(commandPath, ":");
  while (token != NULL)
  {
    char path[MAX_PATH_SIZE]; // Adjust the size as needed
    snprintf(path, sizeof(path), "%s/%s", token, argv[0]);

    // Attempt to execute the command with the full path
    execv(path, argv);

    // If execv fails, try the next path in the list
    token = strtok(NULL, ":");
  }

  // If we reach here, execv has failed for all paths
  executeThorCommand(); // Execute an alternative command or handle the failure as needed
  perror("execv");
  exit(EXIT_FAILURE);
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
 * @param prefix The parameter "prefix" is a string that represents the prefix that the file name
 * should start with.
 *
 * @return The function `fileExists` returns an integer value. It returns 1 if a file with the given
 * prefix and a .c extension is found in the specified directory. It returns 0 if no such file is found
 * or if there is an error opening the directory.
 */
int fileExists(const char *directory, const char *prefix)
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
    if (strncmp(prefix, entry->d_name, strlen(prefix)) == 0)
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
  Command *currentCommand = thorCommandList;
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
      if (background_processes_exec[i] == child_pid)
      {
        background_processes_exec[i] = 0;
        break;
      }
    }
  }
}

#endif