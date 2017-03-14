/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_TCP_PORT 3000
#define BUFLEN 256
#define LOGIN 1
#define LO_ACK 2
#define LO_NACK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NACK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13

/*
struct lab3message {
unsigned int type;
unsigned int size;
unsigned char source[MAX_NAME];
unsigned char data[MAX_DATA];
};
 */


int main(int argc, char** argv) {
    int n, bytes_to_read;
    int sd, port;
    struct hostent *hp;
    struct sockaddr_in server;
    char *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], rawInput[BUFLEN];

    host = "128.100.13.231";
    port = SERVER_TCP_PORT;
    fgets(rawInput, BUFLEN, stdin);

    // create a stream socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket \n");
    }

    bzero((char*) &server, sizeof (struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(port);
    server.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL) {
        fprintf(stderr, "Can't get server address \n");
        exit(1);
    }
    bcopy(hp->h_addr, (char*) &server.sin_addr, hp->h_length);

    //connecting to the server
    if ((connect(sd, (struct sockaddr*) &server, sizeof (server))) == -1) {
        perror("Can't connect \n");
        exit(1);
    }


    printf("Connected: server's address is %s \n", hp->h_name);
    printf("Transmit:\n");
    gets(sbuf); // get users text
    write(sd, sbuf, BUFLEN); //send it out

    // ===================================== DECIPHERING INPUT =================
    // declaring possible info from input
    char command[BUFLEN];
    char password[BUFLEN], serverIP[BUFLEN];
    int clientID, serverPort, sessionID;
    bool requestValid = true;
    bool completedRequest = false;

    while (1) { // complete various commands
        if ((sscanf(rawInput, "%s %d %s %s %d", command, clientID, password, serverIP, serverPort) == 5) &&
                strcmp(command, "/login") == 0) {
            /*
                                        // INCOMPLETE
                                        if(loggedIn)
                                            requestValid = false;
             */
            if (!requestValid) {
                printf("Error: %s", command);
                continue;
            }
            // add them to client list INCOMPLETE
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/logout") == 0) {
            /*
                    // INCOMPLETE
                    if(!loggedIn)
                        requestValid = false;
             */
            if (!requestValid) { // not logged in in the first case
                printf("Error: %s", command);
                continue;
            }
            // complete log out request
        } else if ((sscanf(rawInput, "%s %d", command, sessionID) == 2) &&
                strcmp(command, "/joinsession") == 0) {
            /*
                    // INCOMPLETE
                    if(!loggedIn || inSession)
                        requestValid = false;
             */
            if (!requestValid) { // not logged in
                printf("Error: %s", command);
                continue;
            }
            // complete request
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/leavesession") == 0) {
            /*
                // INCOMPLETE
                if (!loggedIn || !inSession)
                        requestValid = false;
             */
            if (!requestValid) {// not logged in
                printf("Error: %s", command);
                continue;
            }
            // complete request
        } else if ((sscanf(rawInput, "%s %d", command, sessionID) == 2) &&
                strcmp(command, "/createsession") == 0) {
            /*
                        // INCOMPLETE
                        if(!loggedIn || inSession)
                            requestValid = false;
             */
            if (!requestValid) { // not logged in or session already exists
                printf("Error: %s", command);
                continue;
            }
            // complete request
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/list") == 0) {
            /*
                        // INCOMPLETE
                        if(!loggedIn)
                            requestValid = false;
             */
            if (!requestValid) { // not logged in
                printf("Error: %s", command);
                continue;
            }
            // complete request
        }s else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/quit") == 0) {
            // complete request by logging out and terminating program
        }// send a message to the current conference session
        else {
            /*
                        // INCOMPLETE
                        if(!loggedIn || !inSession)
                            requestValid = false;
             */
            if (!requestValid) { // not logged in and/or not in session
                printf("Error: %s", command);
                continue;
            }
            // complete request by sending message in session
        }
    }


    // ===================================== END =======================



    printf("Receive:\n");
    bp = rbuf;
    bytes_to_read = BUFLEN;

    while ((n = read(sd, bp, bytes_to_read)) > 0) {
        bp += n;
        bytes_to_read -= n;
    }
    printf("%s\n", rbuf);


    close(sd);
    return (EXIT_SUCCESS);
}

