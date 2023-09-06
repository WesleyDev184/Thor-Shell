#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "thorCommands/executeThorCommads.c"

char cmd[512];
char dir[4096];
char *argv[3];
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
        executeThorCommand(cmd);
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      wait(NULL);
    }
  }
}

char *getUsername()
{
  struct passwd *pw;
  uid_t uid;

  uid = geteuid();
  pw = getpwuid(uid);

  if (pw != NULL)
    return pw->pw_name;
  else
    return "Desconhecido";
}

char *getCurrentDirectory()
{
  if (getcwd(dir, sizeof(dir)) != NULL)
    return dir;
  else
    return "Desconhecido";
}

int main(void)
{
  system("clear");
  welcomeScreen();
  while (1)
  {
    char *username = getUsername();
    char *currentDirectory = getCurrentDirectory();
    printf("\x1b[35m%s@Thor\x1b[0m:\x1b[36m%s\x1b[0m$ ", username, currentDirectory);
    fgets(cmd, 511, stdin);
    cmd[strlen(cmd) - 1] = 0;

    if (strcmp(cmd, "exit") == 0)
    {
      exit(EXIT_SUCCESS);
    }
    else
    {
      executeCommand();
    }
  }
  return 0;
}
