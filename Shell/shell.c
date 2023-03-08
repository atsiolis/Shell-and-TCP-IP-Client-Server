#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CMD_LEN 256
#define MAX_CMD_ARGS 32

int main(int argc, char *argv[]) 
{
    char cmd[MAX_CMD_LEN];
    char *cmd_args[MAX_CMD_ARGS];
    int cmd_args_len;
    int redirect_in, redirect_out;
    int i;

    while (1) 
    {
        // Print the command prompt
        printf("$ ");

        // Read the command from the standard input
        fgets(cmd, sizeof(cmd), stdin);

        // Remove the trailing newline character from the command
        cmd[strcspn(cmd, "\n")] = 0;

        // Split the command into arguments
        cmd_args[0] = strtok(cmd, " ");
        cmd_args_len = 1;
        while ((cmd_args[cmd_args_len] = strtok(NULL, " ")) != NULL) 
        {
            cmd_args_len++;
        }

        // Exit the shell if the command is "exit"
        if (strcmp(cmd_args[0], "exit") == 0) 
        {
            break;
        }

        // Check if the command contains a pipe
        int pipe_index = -1;
        for (int i = 0; i < cmd_args_len; i++) 
        {
            if (strcmp(cmd_args[i], "|") == 0) 
            {
                pipe_index = i;
                break;
            }
        }

        redirect_in = redirect_out = -1;
        

        if (pipe_index >= 0) 
        {
        // The command contains a pipe
        // Create a pipe
            int pipefd[2];
            if (pipe(pipefd) < 0) 
            {
                perror("pipe failed");
                continue;
            }

        // Split the command into two parts: the part before the pipe and the part after the pipe
            char *cmd1_args[MAX_CMD_ARGS];
            int cmd1_args_len = pipe_index;
            for (int i = 0; i < cmd1_args_len; i++) 
            {
                cmd1_args[i] = cmd_args[i];
            }

            char *cmd2_args[MAX_CMD_ARGS];
            int cmd2_args_len = cmd_args_len - (pipe_index + 1);
            for (int i = 0; i < cmd2_args_len; i++) 
            {
                cmd2_args[i] = cmd_args[i + pipe_index + 1];
            }

            // Create a new process to execute the first part of the command
            pid_t pid1 = fork();

            if (pid1 == 0)
            {
                // This is the child process

                // Close the read end of the pipe
                close(pipefd[0]);

                // Redirect the standard output to the write end of the pipe
                dup2(pipefd[1], STDOUT_FILENO);

                // Execute the first part of the command
                execvp(cmd1_args[0], cmd1_args);

                // If execvp returns, it must have failed
                perror("execvp failed");
                exit(1);
            } 
            else if (pid1 < 0) 
            {
                // The fork failed
                perror("fork failed");
            } 
            else 
            {
                // This is the parent process
                // Create a new process to execute the second part of the command
                pid_t pid2 = fork();

                if (pid2 == 0) 
                {
                    // This is the child process
                    // Close the write end of the pipe
                    close(pipefd[1]);

                    // Redirect the standard input to the read end of the pipe
                    dup2(pipefd[0], STDIN_FILENO);

                    // Execute the second part of the command
                    execvp(cmd2_args[0], cmd2_args);

                    // If execvp returns, it must have failed
                    perror("execvp failed");
                    exit(1);
                } 
                else if (pid2 < 0) 
                {
                    // The fork failed
                    perror("fork failed");
                } 
                else 
                {
                    // This is the parent process
                    // Close the read and write ends of the pipe
                    close(pipefd[0]);
                    close(pipefd[1]);

                    // Wait for the child processes to finish
                    waitpid(pid1, NULL, 0);
                    waitpid(pid2, NULL, 0);
                }
            }
        } 
        else
        {
            // The command does not contain a pipe
            // Create a new process to execute the command
            pid_t pid = fork();
        
            if (pid == 0) 
            {
                // This is the child process
                // Check if the command contains a redirection
                for (int i = 0; i < cmd_args_len; i++) 
                {
                    if (strcmp(cmd_args[i], "<") == 0) 
                    {
                        redirect_in = i;
                    } 
                    else if (strcmp(cmd_args[i], ">") == 0 || strcmp(cmd_args[i], ">>") == 0) 
                    {
                        redirect_out = i;
                    }
                }
                // Redirect output
                if (redirect_out != -1)
                {
                    int fd2;
                    if (strcmp(cmd_args[redirect_out], ">") == 0)
                    {
                        // Open the file for writing, truncating the file if it already exists
                        fd2 = open(cmd_args[redirect_out + 1], O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    }
                    else if (strcmp(cmd_args[redirect_out], ">>") == 0)
                    {
                        // Open the file for writing, appending to the file if it already exists
                        fd2 = open(cmd_args[redirect_out + 1], O_WRONLY | O_CREAT | O_APPEND, 0666);
                    }
                    // Redirect output
                    if (fd2 < 0)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (dup2(fd2, STDOUT_FILENO) < 0)
                    {
                        perror("dup2");
                        exit(1);
                    }
                    if (close(fd2) < 0)
                    {
                        perror("close");
                        exit(1);
                    }
                    // Remove the redirection argument and the file name from the command
                    for(i=redirect_out; i<cmd_args_len-2; i++)
                    {
                        cmd_args[i] = cmd_args[i+2];
                    }
                    cmd_args[cmd_args_len-2] = NULL;
                    cmd_args_len -= 2;
                }

                // Redirect input
                if (redirect_in != -1)
                {
                    // Open the file for reading
                    int fd = open(cmd_args[redirect_in + 1], O_RDONLY);
                    // Redirect input
                    if (fd < 0)
                    {
                        perror("open failed");
                        exit(1);
                    }
                    if (dup2(fd, STDIN_FILENO) < 0)
                    {
                        perror("dup2 failed");
                        exit(1);
                    }
                    if(close(fd) < 0)
                    {
                        perror("close failed");
                        exit(1);
                    }

                    // Remove the redirection argument and the file name from the command
                    for(i=redirect_in; i<cmd_args_len-2; i++)
                    {
                        cmd_args[i] = cmd_args[i+2];
                    }

                    cmd_args[cmd_args_len-2] = NULL;
                    cmd_args_len -= 2;
                }

                // Execute the command
                execvp(cmd_args[0], cmd_args);
        
                // If execvp returns, it must have failed
                perror("execvp failed");
                exit(1);
            } 
            else if (pid < 0) 
            {
                // The fork failed
                perror("fork failed");
            } 
            else 
            {
                // This is the parent process
        
                // Wait for the child process to finish
                waitpid(pid, NULL, 0);

                //change stdin back to keyboard
                if (dup2(0, STDIN_FILENO) < 0)
                {
                    perror("dup2 failed");
                    continue;
                }
                //change stdout back to screen
                if (dup2(1, STDOUT_FILENO) < 0)
                {
                    perror("dup2 failed");
                    continue;
                }   
            }
        }
    }
}