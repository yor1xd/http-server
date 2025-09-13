#define BUFFER_SIZE 8192

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/sendfile.h>

#include "http.h"

int main(int argc, char* argv[]){
    int conn_fd;

    char headerbuf[BUFFER_SIZE];
    char path[BUFFER_SIZE];

    ssize_t bytes;

    int socketfd = beginServer();
    if(socketfd == -1){
        fprintf(stderr, "Could not retrieve socket file descriptor.\n");
        return -1;
    }

    for(;;){

        //Continously accept requests
        conn_fd = accept(socketfd, NULL, NULL);
        if(conn_fd == -1){
            fprintf(stderr, "Could not listen on address.\nERROR: %d\n", errno);
            break;
        }
    
        recv(conn_fd, headerbuf, BUFFER_SIZE, 0);
        request* req = parseRequest(headerbuf);

        printf("Requested path: %s\n", req->path);

        char page_html[BUFFER_SIZE] = "";
        char parsed_html[BUFFER_SIZE] = "";

        DIR* dir = opendir(req->path);
        
        if(dir != NULL){

            struct dirent* dirent;
            while((dirent = readdir(dir)) != NULL){
                //Build path for href
                strcpy(path, req->path);
                strcat(path, dirent->d_name);

                if(dirent->d_type == DT_DIR
                && path[strlen(path) - 1] != '/')
                    strcat(path, "/");

                sprintf(parsed_html, "<a href=\"%s\">%s</a><br>\n", path, dirent->d_name);

                //Add parsed html to page
                strcat(page_html, parsed_html);

            }

            //Send to client
            struct response response = {
                200,
                "text/html",
                NULL,
                page_html,
                0
            };

            sendResponse(conn_fd, response);

            //Clear page buffer and Close connection
            memset(page_html, '\0', BUFFER_SIZE);
            close(conn_fd);

        }

        if(dir == NULL){
            //Not directory, assume and try to open file.
            int file_fd = openat(0, req->path, O_RDONLY);

            if(file_fd == -1){
                fprintf(stderr, "Could not open file at %s.\nERROR: %d\n", req->path, errno);

                struct response not_found = {
                    404,
                    "text/plain",
                    NULL,
                    "Could not open file or directory.",
                    0
                };

                sendResponse(conn_fd, not_found);
                close(conn_fd);
                continue;

            }

            //Get file size for download
            struct stat filestat = {0};
            fstat(file_fd, &filestat);

            char* len_content = malloc(256 * sizeof(char));
            sprintf(len_content, "Content-Length: %lu", filestat.st_size);

            struct response response = {
                200,
                "application/octet-stream",
                &len_content,
                "",
                1
            };

            sendResponse(conn_fd, response);
            off_t offset = 0;
            ssize_t bytes_sent = 0;
            while((bytes_sent = sendfile(conn_fd, file_fd, &offset, filestat.st_size)) > 0){
                printf("Sending...\n");
            }

            printf("Done sending file.\nBytes sent: %lu", bytes_sent);

            free(len_content);
            close(conn_fd);
            continue;

        }

    }

    printf("Exiting...\n");
    close(socketfd);

    return 0;
}
