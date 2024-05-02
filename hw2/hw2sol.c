#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
    struct sockaddr_in server, remote;
    int request_sock, new_sock;
    int bytesread;
    socklen_t addrlen;
    char buf[BUFSIZ];

    if (argc != 2) {
        (void) fprintf(stderr,"usage: %s portnum \n",argv[0]);
        exit(1);
    }

    if ((request_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    
    printf("Student ID : 20223060\n");
    printf("Name : Eunji Kim\n");

    // Create a Server Socket
    memset((void *) &server, 0, sizeof (server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((u_short)atoi(argv[1]));

    // 소켓과 서버 주소 바인딩
    if (bind(request_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");      // 에러 출력
        exit(1);
    }

    if (listen(request_sock, SOMAXCONN) < 0) {
        perror("listen");
        exit(1);
    }

    while(1) {
        /* a new connection is available on the connetion socket */
        addrlen = sizeof(remote);
        new_sock = accept(request_sock,
            (struct sockaddr *)&remote, &addrlen);
        if (new_sock < 0) {
            perror("accept");
            exit(1);
        }

        while(1){

            printf("Connection : Host IP %s, Port %d, socket %d\n",
            inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), new_sock);

        
            bytesread = read(new_sock, buf, sizeof (buf) - 1);
            if (bytesread<=0) {
                if (close(new_sock)) 
                    perror("close");
                break;
            }
            
            buf[bytesread] = '\0';
            
            char *request_msg = strtok(buf, "\r\n");
            char *user_agent;
            char *header;
            int header_count = 0;

            while((header = strtok(NULL, "\r\n")) != NULL){
                if(strstr(header, "User-Agent: ") != NULL){
                    user_agent = strstr(header, "User-Agent: ") + strlen("User-Agetn: ");
                    char *end_of_user_agent = strchr(user_agent, ' ');
                    if (end_of_user_agent != NULL)
                        *end_of_user_agent = '\0';
                }   
                header_count++;
            }
            
            if(user_agent == NULL)
                user_agent = "(Unknown)";

            // 정보 출력
            printf("%s\nUser-Agent: %s\n%d headers\n", request_msg, user_agent, header_count);
            fflush(stdout); // 표준 출력 버퍼 비우기
            
            char *filename = strtok(request_msg + 5, " ");
            char response[BUFSIZ];

            // 요청 파일 읽기
            FILE *file = fopen(filename, "rb");
            if (!file) {
                printf("Server Error : No such file ./%s!\n", filename);
                
                sprintf(response, "HTTP/1.0 404 NOT FOUND\r\nConnection: close\r\nContent-Length: 0\r\nContent-Type: text/html\r\n\r\n");
                if (write(new_sock, response, strlen(response)) != strlen(response)) {
                    perror("write");
                }

                continue;
            }

            // 파일 크기 확인
            fseek(file, 0, SEEK_END);
            int file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            // MIME 타입 결정
            const char *mime_type;
            if (strstr(filename, ".jpg") || strstr(filename, ".jpeg")) {
                mime_type = "image/jpeg";
            } else if (strstr(filename, ".html")) {
                mime_type = "text/html";
            }

            // HTTP 응답 메시지 생성
            sprintf(response, "HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Length: %d\r\nContent-Type: %s\r\n\r\n", file_size, mime_type);


            // 클라이언트에게 응답 전송
            if (write(new_sock, response, strlen(response)) != strlen(response)) {
                perror("write");
            }

            // 파일 내용 전송
            char file_buffer[BUFSIZ];
            size_t total_bytes_sent = 0;

            while (1) {
                size_t bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file);
                if (bytes_read <= 0) break;
                if (write(new_sock, file_buffer, bytes_read) != bytes_read) {
                    perror("write");
                    break;
                }
                total_bytes_sent += bytes_read;
            }
            
            fclose(file);

            printf("finish %d %zu\n", file_size, total_bytes_sent);
            fflush(stdout); // 표준 출력 버퍼 비우기

        }  
    }
}
