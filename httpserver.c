#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

int main()
{
    // creating socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // assinging port and address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // binding
    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // listening
    if (listen(socket_fd, 1) == -1)
    {
        perror("listen");
        exit(1);
    }
    printf("Listening to port 8080\n");
    while (1)
    {
        char buffer[1024];
        char *tokens[50];
        char file_content[4096];
        // accept new client
        int client_fd = accept(socket_fd, NULL, NULL);
        if (client_fd == -1)
        {
            perror("accept");
            exit(1);
        }

        // recieving request
        int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0)
        {
            buffer[n] = '\0';

            int index = 0;
            char *token = strtok(buffer, " ");
            

            while (token != NULL)
            {
                tokens[index++] = token;
                token = strtok(NULL, " ");
                
            }

            // reading specified file in request
            char *path = tokens[1];
            size_t bytes_read;
            FILE *fp = fopen(path + 1, "r");
            char header[2048];
            if (fp != NULL)
            {
                bytes_read = fread(file_content, sizeof(char), sizeof(file_content) - 1, fp);
                // file_content[bytes_read] = '\0'; we don't need because we are giving buffer size in send function externally.
                snprintf(header, sizeof(header),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                bytes_read
                );

                // sending to client
                send(client_fd, header, strlen(header), 0);
                send(client_fd, file_content, bytes_read, 0);
                fclose(fp);
                close(client_fd);
            } else {
                
                char *body = "Not Found";
                snprintf(header, sizeof(header),
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                strlen(body)
                );
                
                send(client_fd, header, strlen(header), 0);
                send(client_fd, body, strlen(body), 0);
                close(client_fd);
            }
            
        }
        else if(n == -1) {
            perror("recv");
            close(client_fd);
        } else {
            close(client_fd);
        }
    }

    close(socket_fd);

    return 0;
}