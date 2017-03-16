
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

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
#define EXIT_ACK 16

struct lab3message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

// function declarations
char* packetToStr(struct lab3message packet);
struct lab3message parser(char recMsg[BUFLEN]);
void* antenna(void* sd);

bool socketExist;
pthread_t t;

int main(int argc, char** argv) {
    int n, bytes_to_read, sd;
    struct hostent *hp;
    struct sockaddr_in server;
    char *bp, rbuf[BUFLEN], sbuf[BUFLEN], rawInput[BUFLEN];

    // ===================================== DECIPHERING INPUT =================
    // declaring possible info from input
    char command[BUFLEN];
    char password[BUFLEN], serverIP[BUFLEN], clientID[BUFLEN];
    char badChar[BUFLEN], badCommand[BUFLEN];
    int serverPort, sessionID;
    int badInt[BUFLEN];
    socketExist = false;

    // declaring stuff to send and receive
    struct lab3message outPacket, received;

    while (1) { // complete various commands
        //printf("Give a command: ");
        fgets(rawInput, BUFLEN, stdin); // first take the input

        if ((sscanf(rawInput, "%s %s %s %s %d", badCommand, badChar, badChar, badChar, &badInt) == 5) &&
                strcmp(badCommand, "/login") == 0) {
            sscanf(rawInput, "%s %s %s %s %d", command, clientID, password, serverIP, &serverPort);
            sprintf(outPacket.data, "%s", password); // no need to send clientID
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = LOGIN;

            // connect to server with this information
            if (socketExist == false) {
                // create a stream socket
                if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                    fprintf(stderr, "Can't create a socket \n");
                    exit(1);
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
                
                socketExist = true;
                pthread_create(&t, NULL, antenna, &sd);
            }

            //send it out and let server add them to client list
            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN);
            free(temp);
        } else if ((sscanf(rawInput, "%s", badCommand) == 1) &&
                strcmp(badCommand, "/logout") == 0) {
            sscanf(rawInput, "%s", command);
            strcpy(outPacket.data, "dummy");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = EXIT;

            //send it out
            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN);
            free(temp);
        } else if ((sscanf(rawInput, "%s %d", badCommand, &badInt) == 2) &&
                strcmp(badCommand, "/joinsession") == 0) {
            sscanf(rawInput, "%s %d", command, &sessionID);
            sprintf(outPacket.data, "%d", sessionID);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = JOIN;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
        } else if ((sscanf(rawInput, "%s", badCommand) == 1) &&
                strcmp(badCommand, "/leavesession") == 0) {
            sscanf(rawInput, "%s", command);
            strcpy(outPacket.data, "dummy");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = LEAVE_SESS;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
        } else if ((sscanf(rawInput, "%s %d", badCommand, &badInt) == 2) &&
                strcmp(badCommand, "/createsession") == 0) {
            sscanf(rawInput, "%s %d", command, &sessionID);

            outPacket.type = NEW_SESS;
            sprintf(outPacket.data, "%d", sessionID);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
        } else if ((sscanf(rawInput, "%s", badCommand) == 1) &&
                strcmp(badCommand, "/list") == 0) {
            sscanf(rawInput, "%s", command);
            strcpy(outPacket.data, "dummy");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = QUERY;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
        } else if ((sscanf(rawInput, "%s", badCommand) == 1) &&
                strcmp(badCommand, "/quit") == 0) {
            sscanf(rawInput, "%s", command);

            strcpy(outPacket.data, "dummy");
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = EXIT;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
            break; //exit while loop
        } else { // send a message to the current conference session
            strcpy(outPacket.data, rawInput);
            outPacket.size = strlen(outPacket.data);
            strcpy(outPacket.source, clientID);
            outPacket.type = MESSAGE;

            char* temp = packetToStr(outPacket);
            write(sd, temp, BUFLEN); //send it out
            free(temp);
        }
    }

    //close(sd);
    return (EXIT_SUCCESS);
}

void* antenna(void* sd) {
    int sockfd = *((int*) sd);
    int n, bytes_to_read;
    char *bp, rbuf[BUFLEN];
    while (1) {
        bp = rbuf;
        bytes_to_read = BUFLEN;
        while ((n = read(sockfd, bp, bytes_to_read)) > 0) {
            bp += n;
            bytes_to_read -= n;
        }
        struct lab3message received = parser(rbuf); // complete message

        if (received.type == LO_NACK) {
            printf("==================== Error: login. %s \n", received.data);
        } else if (received.type == MESSAGE) {
            printf("==================== Incoming Message: %s \n", received.data);
        } else if (received.type == QU_ACK) {
            printf("==================== %s \n", received.data);
        }
        else if(received.type ==  GEN_ACK || received.type ==  LO_ACK || received.type ==  JN_ACK || received.type ==  NS_ACK) {
            printf("==================== Success: %s \n", received.data);
        }
        else if (received.type == GEN_NACK || received.type == JN_NACK) {
            printf("==================== Failed because: %s \n", received.data);
        } else if (received.type == EXIT_ACK) {
            printf("==================== Success: %s \n", received.data);
            socketExist = false;
            pthread_exit(t);
        } else
            continue;
    }
    return NULL;
}

char* packetToStr(struct lab3message packet) {
    char dummy[BUFLEN];
    sprintf(dummy, "%u:%u:%s:%s", packet.type, packet.size, packet.source, packet.data);
    char* output = malloc(strlen(dummy) * sizeof (char));
    strcpy(output, dummy);
    return output;
}

struct lab3message parser(char recMsg[]) {
    struct lab3message tempPack;

    //printf("PARSER INPUT: %s \n", recMsg);
    tempPack.type = atoi(strtok(recMsg, ":"));
    tempPack.size = atoi(strtok(NULL, ":"));
    strcpy(tempPack.source, strtok(NULL, ":"));
    strcpy(tempPack.data, strtok(NULL, ""));

    return tempPack;
};
