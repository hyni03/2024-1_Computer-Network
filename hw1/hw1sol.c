// Student ID : 20223060
// Name : Eunji Kim

#include <stdio.h> 
#include <stdlib.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <unistd.h>

#define PROMPT() {printf("\n> ");fflush(stdout);}
#define GETCMD "down"
#define QUITCMD "quit"


int main(int argc, char *argv[]) {
    
	int socktoserver = -1;
	char buf[BUFSIZ];

	printf("Student ID : 20223060\n");
	printf("Name : Eunji Kim\n");

	PROMPT(); 

	for (;;) {
		if (!fgets(buf, BUFSIZ - 1, stdin)) {
			if (ferror(stdin)) {
				perror("stdin");
				exit(1);
			}
			exit(0);
		}

		char *cmd = strtok(buf, " \t\n\r");

		if((cmd == NULL) || (strcmp(cmd, "") == 0)) {
			PROMPT(); 
			continue;
		} else if(strcasecmp(cmd, QUITCMD) == 0) {
				exit(0);
		}

		if(!strcasecmp(cmd, GETCMD) == 0) {
			printf("Wrong command %s\n", cmd);
			PROMPT(); 
			continue;
		}

		// connect to a server
		struct sockaddr_in server;
		int sock;

        // Create a socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            PROMPT();
            continue;
        }

        // Extract URL from command
		char *url = strtok(NULL, " \t\n\r"); 
		if (url == NULL) {
			printf("Invalid URL\n");
			PROMPT();
			continue;
		}

        // Parse url
        char *protocol = strtok(url, "://");
        char *hostname = strtok(NULL, "/");
        int port = 80; // Default port is 80

        // If port number is included, extract it separately
        char *im_ptr = strchr(hostname, ':');
        if (im_ptr != NULL) {
            *im_ptr = '\0'; 
            port = atoi(im_ptr + 1); 
        }

        // Error 1 - if protocol is not 'http'
		if (protocol == NULL || strcmp(protocol, "http") != 0) {
			printf("Only support http, not %s\n", protocol);
			PROMPT();
			continue;
		}

        // Error 2 - if unable to connect to the server
		struct hostent *host = gethostbyname(hostname);
		if (host == NULL) {
			printf("%s: unknown host\ncannot connect to server %s %d\n", hostname, hostname, port);
            PROMPT();
            continue;
        }

		// Set up server information
        memset(&server, 0, sizeof(server));
        server.sin_family = AF_INET;
        server.sin_port = htons(port); // HTTP port
        memcpy(&server.sin_addr, host->h_addr, host->h_length);

        // Connect to server
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
			(void) close(sock);
            PROMPT();
            continue;
        }


	    char buf[BUFSIZ];
	    char recv_buf[BUFSIZ];
		char *path = strtok(NULL, "\n");
		char *filename = strrchr(path, '/');
		int total_size, current, download, percent;

        // Parse filename
        if (filename == NULL) 
            filename = path;
        else filename += 1;

        // Generate request message
		snprintf(buf, BUFSIZ, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-agent: HW1/1.0\r\nConnection: close\r\n\r\n", path, hostname);
        printf("%s\n", buf);

        // Send request message
        if (write(sock, buf, strlen(buf)) < 0) {
            perror("write");
            PROMPT();
            continue;
        }

        // Receive response
        ssize_t bytes_received = recv(sock, recv_buf, BUFSIZ, 0);
        recv_buf[bytes_received] = '\0';

        // Parse response
        char *header_end = strstr(recv_buf, "\r\n\r\n");
        char *content_length_start = strstr(recv_buf, "Content-Length:");

        // Extract status code
        char *status_code = strtok(recv_buf, " ");
        status_code = strtok(NULL, " ");

        // Error 3 - if response code is not 200, print error code and message
        if (strcmp(status_code, "200") != 0) {
            char *status_message = strtok(NULL, "\r\n");
            printf("%s %s\n\n", status_code, status_message);
            close(sock);
            PROMPT();
            continue;
        }

        // Extract total size
        sscanf(content_length_start + strlen("Content-Length:"), "%d", &total_size);
        printf("Total Size %d bytes\n", total_size);

        // Open file for writing
        FILE *file = fopen(filename, "wb");

        // Write response body to file
        fwrite(header_end + 4, 1, bytes_received - (header_end - recv_buf + 4), file);
        current = bytes_received - (header_end - recv_buf + 4);

        // Download remaining data
        percent = 10;
        while (current < total_size) {
            bytes_received = recv(sock, recv_buf, BUFSIZ, 0);
            if (bytes_received <= 0)
                break;
            fwrite(recv_buf, 1, bytes_received, file);
            current += bytes_received;

            // Print download progress
            download = (current * 100) / total_size;
            if (download >= percent) {
                printf("Current Downloading %d/%d (bytes) %d%%\n", current, total_size, download);
                percent += 10;
            }
        }

        printf("Download Complete: %s, %d/%d\n", filename, total_size, total_size);

        // Close file and socket
        fclose(file);
        close(sock);
        
        PROMPT();
    }
		
} 
