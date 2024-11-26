#include <stdio.h>
#include <string.h>			//for string functions
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()


char** parseInput(char* line)
// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
{
	char* list;
	char **lists = malloc(64*sizeof(char*));
	int position = 0;
	while((list = strsep(&line," ")) != NULL)
	{
		lists[position] = list;
		position++;
	}
	lists[position] = NULL;	
	return lists;
}

	
void executeCommand(char** cmd)   
// This function will fork a new process to execute a command
{

	if(strcmp(cmd[0],"cd") == 0)
	{
		chdir(cmd[1]); // changing directory
		return;
	}
	else
	{
	  int rc = fork();
		if (rc < 0)
		{
			exit(0); // Error in forking
	  	}
	  		else if (rc == 0)
			{
	    		// Inside child process
	    		if (execvp(cmd[0], cmd) < 0)
				{
	      			printf("Shell: Incorrect command\n");
					exit(1);
	   	 		}
	  		}
				else
				{
	    			// Parent process will wait 
	    			int rc_wait = wait(NULL);
	  			}
	}
  return;
	
}

void executeParallelCommands(char** cmd)
{
    int i=0;
    int bef = i;
    //while loop because of multiple && commands
    while(cmd[i] != NULL)
    {
        while(cmd[i]!=NULL && strcmp(cmd[i],"&&") != 0)
        {
            i++;
        }
        cmd[i] = NULL;
        if(strcmp(cmd[bef],"cd")==0) //for cd command
            {
                chdir(cmd[bef+1]); //chdir - changing directory
            }
        else
            {
                //creating a child by dork
                int rc= fork();
                if(rc < 0)
                {
                    exit(1); // error creating child process
                }
                else if(rc == 0) // inside child process
                {
                    if(execvp(cmd[bef],&cmd[bef]) < 0)
                    {
                        printf("Shell:Incorrect command\n");
                        exit(1); // error executing the process
                    }
                }
            }
        i++;
        bef = i;
    }
    while(wait(NULL)>0);
}


void executeSequentialCommands(char** cmd)
{
    int i=0;
    int bef = i;
    //while loop because of multiple ## commands
    while(cmd[i] != NULL)
    {
        while(cmd[i]!=NULL && strcmp(cmd[i],"##") != 0)
        {
            i++;
        }
        cmd[i] = NULL;
        if(strcmp(cmd[bef],"cd")==0) //for cd command
            {
                chdir(cmd[bef+1]); //chdir - changing directory
            }
        else
            {
                //creating a child by fork
                int rc= fork();
                if(rc < 0)
                {
                    exit(1); // error creating child process
                }
                else if(rc == 0) // inside child process
                {
                    if(execvp(cmd[bef],&cmd[bef]) < 0)
                    {
                        printf("Shell:Incorrect command\n");
                        exit(1); // error executing the process
                    }
                }
                else
                {
                    wait(NULL);
                }
            }
        i++;
        bef = i;
    }
}

void executeCommandRedirection(char** cmd)
{
	// This function will run a single command with output redirected to an output file specificed by user
	int i = 0;
	int bef = i;
		while(cmd[i] != NULL && strcmp(cmd[i],">") != 0)
		{
			i++;
		}
		cmd[i] = NULL;
		int rc = fork();
		if(rc < 0)  // In case fork failed, so it exits
		{
			exit(1);
		}
		else if(rc == 0)
		{
			if(cmd[i+1] == NULL)
				return;
			close(STDOUT_FILENO);
			open(cmd[i+1], O_CREAT | O_WRONLY | O_APPEND);
			if(execvp(cmd[0],cmd)<0) // In case if we give wrong command
			{
				printf("Shell: Incorrect command");
				exit(1);
			}
		}
		else
		{
			int rc_wait = wait(NULL); //Here  parent process will wait for the Child
		}

}

void print()
	{
		char path[1000];
		getcwd(path,sizeof(path)); //getcwd  goes to the current working directory
		printf("%s$",path);
	}

int main()
{

	// Initial declarations
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	long unsigned int size = 10;
	char* line=NULL;
	line = (char *)malloc(size);
	char ** cmd=NULL;
	int i, command = 0;
	while(1)	// This loop will keep your shell running until user use exit command.
	{
		
		//  format of currentWorkingDirectory$
		print();
		// It accepts input with 'getline()'
		getline(&line,&size,stdin);		
		for(i=0;line[i] != '\0';i++)
                {
                    if(line[i] == EOF || line[i] == '\n')
                    {
                        line[i] = '\0';
                    }
                }
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
		cmd = parseInput(line);
		if(strcmp(cmd[0],"exit") == 0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}
		i = 0;
		while(cmd[i] != NULL && command==0)
		{
			
			if(strcmp(cmd[i],"&&") == 0)
			{
				command = 1;
			}
			else if(strcmp(cmd[i],"##") == 0)
			{
				command = 2;
			}
			else if(strcmp(cmd[i],">") == 0)
			{
				command = 3;
			}
			i++;
		}

	 	if(command == 1)
			executeParallelCommands(cmd);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(command == 2)
			executeSequentialCommands(cmd);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(command == 3)
			executeCommandRedirection(cmd);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(cmd);		// This function is invoked when user wants to run a single commands

	}

	return 0;
}

