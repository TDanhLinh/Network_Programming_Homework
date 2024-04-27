#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

// Signal handler function for child process termination
void signalHandler(int signo)
{
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

// Function to check the command and format received from the client
int checkCmd(char *buf, char *format)
{
    char cmd[16];
    int n = sscanf(buf, "%s %s", cmd, format);

    if (n == 2)
    {
        if (!strcmp(cmd, "GET_TIME"))
        {
            if (!strcmp(format, "dd/mm/yyyy") || !strcmp(format, "dd/mm/yy") || !strcmp(format, "mm/dd/yyyy") || !strcmp(format, "mm/dd/yy"))
            {
                return 1;
            }
        }
    }
    return 0;
}

int main()
{
    // Create a socket for listening to client connections
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Set up the address structure for the socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Bind the socket to the address and port
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Start listening for client connections
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    // Set up a signal handler for child process termination
    signal(SIGCHLD, signalHandler);

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client connected, client = %d\n", client);

        // Fork a new process for handling the client connection
        if (fork() == 0)
        {
            close(listener);

            char buf[256];
            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    continue;

                buf[ret] = 0;
                printf("Received: %s\n", buf);
                char format[16];

                // Check if the command and format are valid
                if (checkCmd(buf, format))
                {
                    printf("format: %s\n", format);

                    char timeBuffer[32];
                    time_t now = time(NULL);
                    struct tm *tm = localtime(&now);

                    // Format the current time based on the client's request
                    if (!strcmp(format, "dd/mm/yyyy"))
                        strftime(timeBuffer, sizeof(timeBuffer), "%d/%m/%Y\n", tm);
                    else if (!strcmp(format, "dd/mm/yy"))
                        strftime(timeBuffer, sizeof(timeBuffer), "%d/%m/%y\n", tm);
                    else if (!strcmp(format, "mm/dd/yyyy"))
                        strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%Y\n", tm);
                    else if (!strcmp(format, "mm/dd/yy"))
                        strftime(timeBuffer, sizeof(timeBuffer), "%m/%d/%y\n", tm);

                    printf("time: %s\n", timeBuffer);
                    send(client, timeBuffer, strlen(timeBuffer), 0);
                }
                else
                {
                    char *msg = "Invalid command\n";
                    send(client, msg, strlen(msg), 0);
                }
            }
            exit(0);
        }
        close(client);
    }
    close(listener);
    return 0;
}