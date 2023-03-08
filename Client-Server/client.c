#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX 250
#define PORT 8080
#define SA struct sockaddr

typedef struct match{
    int id;
    char *team1;
    char *team2;
    int num_of_tickets;
    int price;
}match;

typedef struct match_array{
    match *match;
    int size;
}match_array;


int main()
{
    int i;
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
 
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
 
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
 

    while(1)
    {
        char buff[MAX];
        int n;
        bzero(buff, sizeof(buff));
        
        //read line by line the matches
        for(i=0;i<10;i++)
        {
            read(sockfd, buff, sizeof(buff));
            printf("%s", buff);
        }
        
        //send the id of the match and the number of tickets
        printf("Enter the id of the match you want to buy tickets for: ");
        scanf("%s", buff);
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        printf("Enter the number of tickets you want to buy: ");
        scanf("%s", buff);
        write(sockfd, buff, sizeof(buff));
        bzero(buff, sizeof(buff));
        //reiceive the price
        read(sockfd, buff, sizeof(buff));
        printf("%s", buff);
        bzero(buff, sizeof(buff));
        printf("\n");



        return 0;
    }
}