#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

int main()
{
    // Create a socket for listening
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == -1)
    {
        // Handle error if socket creation fails
        perror("socket() failed");
        return 1;
    }

    // Define the address structure for binding
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    // Set address to INADDR_ANY to listen on all available network interfaces
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Set port number to 9000
    addr.sin_port = htons(9000);

    // Bind the socket to the specified address and port
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        // Handle error if binding fails
        perror("bind() failed");
        return 1;
    }

    // Listen for incoming connections with a backlog of 5
    if (listen(listener, 5) == -1)
    {
        // Handle error if listening fails
        perror("listen() failed");
        return 1;
    }

    // Buffer to store received data
    char buf[256];

    // Create 8 child processes to handle incoming connections
    for (int i = 0; i < 8; i++)
    {
        if (fork() == 0)
        {
            // Child process loop to accept and handle incoming connections
            while (1)
            {
                // Accept an incoming connection
                int client = accept(listener, NULL, NULL);
                printf("New client accepted: %d\n", client);

                // Receive data from the client
                int ret = recv(client, buf, sizeof(buf), 0);

                if (ret <= 0)
                    continue;

                buf[ret] = 0;

                // Print the received data
                printf("Received from %d: %s\n", client, buf);

                // Send a response back to the client
                char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
                send(client, msg, strlen(msg), 0);

                // Close the client socket
                close(client);
            }
            // Exit the child process
            exit(0);
        }
    }

    // Wait for user input before terminating the parent process
    getchar();

    // Kill all child processes
    killpg(0, SIGKILL);

    // Close the listener socket
    close(listener);

    return 0;
}