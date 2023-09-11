#ifndef EXECUTETHORCOMMANDS_C
#define EXECUTETHORCOMMANDS_C

#include <stdio.h>
#include <string.h>
#include "../utils.h"

// declaration of functions
void executeThorCommand();
void showHistory();
void helpCommand();
void setupCommandList();
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
    if (strcmp(currentCommand->name, cmd) == 0)
    {
      currentCommand->function();
      return;
    }
    currentCommand = currentCommand->next;
  }
  printf("Comando não encontrado!\n");
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

#endif