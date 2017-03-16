
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

#define BUFLEN 1000
#define MAX_NAME 1000
#define MAX_DATA 1000
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
#define GEN_ACK 14
#define GEN_NACK 15

struct lab3message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

// function declarations
char* packetToStr(struct lab3message packet);
struct lab3message parser(char recMsg[BUFLEN]);

int main(int argc, char** argv) {
    int n, bytes_to_read, sd;
    struct hostent *hp;
    struct sockaddr_in server;
    char *bp, rbuf[BUFLEN], sbuf[BUFLEN], rawInput[BUFLEN];

    // ===================================== DECIPHERING INPUT =================
    // declaring possible info from input
    char command[BUFLEN];
    char password[BUFLEN], serverIP[BUFLEN], clientID[BUFLEN];
    int serverPort, sessionID;
    bool completedRequest = false;

    // declaring stuff to send and receive
    struct lab3message outPacket, received;

    while (1) { // complete various commands
        fgets(rawInput, BUFLEN, stdin); // first take the input
        
        if ((sscanf(rawInput, "%s %s %s %s %d", command, clientID, password, serverIP, serverPort) == 5) &&
                strcmp(command, "/login") == 0) {
            sprintf(outPacket.data, "%s", password); // no need to send clientID
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = LOGIN;

            // connect to server with this information
            // create a stream socket
            if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                fprintf(stderr, "Can't create a socket \n");
            }

            bzero((char*) &server, sizeof (struct sockaddr_in));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = htonl(serverPort);
            server.sin_port = htons(serverPort);

            if ((hp = gethostbyname(serverIP)) == NULL) {
                fprintf(stderr, "Can't get server address \n");
                exit(1);
            }
            bcopy(hp->h_addr, (char*) &server.sin_addr, hp->h_length);

            //connecting to the server
            if ((connect(sd, (struct sockaddr*) &server, sizeof (server))) == -1) {
                perror("Can't connect \n");
                exit(1);
            }

            //send it out and let server add them to client list
            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN);
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf);

            //see whether this was successful
            if (received.type == LO_ACK)
                completedRequest = true;
            else if (received.type == LO_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // already logged in
                continue;
            }
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/logout") == 0) {
            strcpy(outPacket.data, "");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = EXIT;

            //send it out
            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN);
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // complete log out request

            //see whether this was successful
            if (received.type == GEN_ACK)
                completedRequest = true;
            else if (received.type == GEN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // // not logged in in the first case
                continue;
            }

        } else if ((sscanf(rawInput, "%s %d", command, sessionID) == 2) &&
                strcmp(command, "/joinsession") == 0) {
            sprintf(outPacket.data, "%d", sessionID);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = JOIN;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // complete join session request

            //see whether this was successful
            if (received.type == JN_ACK)
                completedRequest = true;
            else if (received.type == JN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // (!loggedIn || inSession)
                continue;
            }

        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/leavesession") == 0) {

            strcpy(outPacket.data, "");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = LEAVE_SESS;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // complete leave session request

            //see whether this was successful
            if (received.type == GEN_ACK)
                completedRequest = true;
            else if (received.type == GEN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // (!loggedIn || !inSession)
                continue;
            }
        } else if ((sscanf(rawInput, "%s %d", command, sessionID) == 2) &&
                strcmp(command, "/createsession") == 0) {
            sprintf(outPacket.data, "%d", sessionID);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = NEW_SESS;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);

            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // complete create session request

            //see whether this was successful
            if (received.type == NS_ACK)
                completedRequest = true;
            else if (received.type == GEN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // ((!loggedIn || inSession)
                continue;
            }
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/list") == 0) {
            strcpy(outPacket.data, "");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = QUERY;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf);

            //see whether this was successful
            if (received.type == QU_ACK) { // PRINT OUT STUFF HERE: complete request
                completedRequest = true;
                // first see how many sessions there are
                int numUser;
                bp = rbuf;
                bytes_to_read = BUFLEN;
                while ((n = read(sd, bp, bytes_to_read)) > 0) {
                    bp += n;
                    bytes_to_read -= n;
                }
                received = parser(rbuf);
                numUser = atoi(received.data);

                // for each session receive a packet and print
                int i;
                for (i = 0; i < numUser; i++) {
                    bp = rbuf;
                    bytes_to_read = BUFLEN;
                    while ((n = read(sd, bp, bytes_to_read)) > 0) {
                        bp += n;
                        bytes_to_read -= n;
                    }
                    received = parser(rbuf);
                    printf("%s \n", received.data); // format: clientID, session numbers
                }
            } else if (received.type == GEN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // (!loggedIn)
                continue;
            }
        } else if ((sscanf(rawInput, "%s", command) == 1) &&
                strcmp(command, "/quit") == 0) {

            strcpy(outPacket.data, "");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = EXIT;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);

            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // leave session and log out

            //see whether this was successful
            if (received.type == GEN_ACK) {
                completedRequest = true;
                break; // terminate client program
            } else if (received.type == GEN_NACK) {
                printf("Error: %s \n", command);
                printf("Reason: %s \n", received.data); // (!loggedIn || !inSession)
                continue;
            }
        } else { // send a message to the current conference session
            strcpy(outPacket.data, rawInput);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = QUERY;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
            bp = rbuf;
            bytes_to_read = BUFLEN;
            while ((n = read(sd, bp, bytes_to_read)) > 0) {
                bp += n;
                bytes_to_read -= n;
            }
            received = parser(rbuf); // complete message

            //see whether this was successful: always will be
            if (received.type == GEN_ACK)
                completedRequest = true;
        }
    }

    close(sd);
    return (EXIT_SUCCESS);
}

char* packetToStr(struct lab3message packet) {
    char dummy[BUFLEN];
    sprintf(dummy, "%u:%u:%s:%s", packet.type, packet.size, packet.source, packet.data);
    char* output = malloc(strlen(dummy) * sizeof (char));
    strcpy(output, dummy);
    return output;
} 

struct lab3message parser(char recMsg[BUFLEN]) {
    struct lab3message tempPack;

    tempPack.type = atoi(strtok(recMsg, ":")); 
    tempPack.size = atoi(strtok(NULL, ":"));
    strcpy(tempPack.source, strtok(NULL, ":"));
    strcpy(tempPack.data, strtok(NULL, ""));

    return tempPack;
};
