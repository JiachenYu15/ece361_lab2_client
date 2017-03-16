/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: xuminjia
 *
 * Created on March 12, 2017, 4:01 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_TCP_PORT 3001
#define BUFLEN 1000

int main(int argc, char** argv) {
    int n, bytes_to_read;
    int sd, port;
    struct hostent *hp;
    struct sockaddr_in server;
    char *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN];

    /*
        switch (argc) {
            case 2:
                host = argv[1];
                port = SERVER_TCP_PORT;
                break;
            case 3:
                host = argv[1];
                port = atoi(argv[2]);
                break;
            default:
                fprintf(stderr, "COMMAND WRONG \n");
                exit(1);
        }
     */

    host = "128.100.13.179";
    port = SERVER_TCP_PORT;

    // create a stream socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket \n");
    }

    bzero((char*) &server, sizeof (struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(host)) == NULL) {
        fprintf(stderr, "Can't get server address \n");
        exit(1);
    }
    bcopy(hp->h_addr, (char*) &server.sin_addr, hp->h_length);
    
    int result = (connect(sd, (struct sockaddr*) &server, sizeof (server)));
    printf("result: %d\n", result);

    //connecting to the server
    if (result == -1) {
        perror("Can't connect \n");
        exit(1);
    }
    printf("Connected: server's address is %s \n", hp->h_name);
    printf("Transmit:\n");
    while(1){
    gets(sbuf); // get users text
    write(sd, sbuf, BUFLEN); //send it out

    printf("Receive:\n");
    bp = rbuf;
    bytes_to_read = BUFLEN;

    while ((n = read(sd, bp, bytes_to_read)) > 0) {
        bp += n;
        bytes_to_read -= n;
    }
    printf("%s\n", rbuf);
    }
    close(sd);
    return (EXIT_SUCCESS);
}

