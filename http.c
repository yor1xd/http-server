#include "http.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>



request* parseRequest(char* buf){

    request* request = malloc(sizeof(struct request));

    sscanf(buf, "%s %s %s %s", 
           request->method, 
           request->path, 
           request->version, 
           request->host);

    return request;
}

int beginServer(){
    int res, sfd;
    struct addrinfo hints, *list, *result;
    request req;

    memset(&hints, 0, sizeof(hints));
    memset(&req, 0, sizeof(struct request));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    res = getaddrinfo(NULL, "4444", &hints, &list);
    if(res){
        fprintf(stderr, "Could not get address info.\n ERROR: %s\n", gai_strerror(res));
        return res;
    }

    for(result = list; result != NULL; result = result->ai_next){
        sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(sfd == -1){
            continue;
        }

        res = bind(sfd, result->ai_addr, result->ai_addrlen);
        if(res == 0) break;

        close(sfd);
    }

    freeaddrinfo(list);

    if(result == NULL){
        fprintf(stderr, "Could not bind address.\n");
        return -1;
    }

    res = listen(sfd, 5);
    if(res == -1){
        fprintf(stderr, "Could not listen on address.\nERROR: %d\n", errno);
        return -1;
    }

    return sfd;
}


ssize_t createResponse(struct response resp, char* string){
    sprintf(string, 
            "HTTP/1.1 %d\r\nContent-Type: %s\r\n", 
            resp.code,
            resp.content_type);

    for(int i = 0; i < resp.len_headers; i++){
        strcat(string, resp.headers[i]);
        strcat(string, "\r\n");
    }

    strcat(string, "\r\n");

    if(!strcmp(resp.content_type, "text/html")){
        strcat(string, "<body>");
        strcat(string, resp.body);
        strcat(string, "</body>");
    } else {
        strcat(string, resp.body);
    }

    return strlen(string);
}

ssize_t sendResponse(int fd, struct response response){
    char message[8192];
    ssize_t length = createResponse(response, message);
    
    return send(fd, message, length, 0);
}
