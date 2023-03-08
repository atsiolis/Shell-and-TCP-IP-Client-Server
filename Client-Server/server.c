#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
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

void init_match_array(match_array *match_array)
{   
    int i;

    //random seed
    time_t t;
    srand((unsigned)time(&t));

    //read from file
    FILE *fp;
    fp = fopen("matches.txt", "r");
    if (fp == NULL)
    {
        printf("Error opening file");
        exit(1);
    }
    match_array->size = 10;
    match_array->match = (match*)malloc(match_array->size * sizeof(match));

    //read line by line
    char line[100];
    char *token;
    
    for (i = 0; i<10; i++)
    {
        match_array->match[i].team1 = (char*)malloc(100 * sizeof(char));
        match_array->match[i].team2 = (char*)malloc(100 * sizeof(char));

        match_array->match[i].id = i;
        fgets(line, 100, fp);                   
        token = strtok(line, "\n\0");
        strcpy(match_array->match[i].team1, token);
        token = NULL;
        fgets(line, 100, fp);
        token = strtok(line, "\n\0");
        strcpy(match_array->match[i].team2, token);
        token = NULL;

        match_array->match[i].num_of_tickets = 1+rand() % 5;
        match_array->match[i].price = 10+rand() % 10;
    }
    
    fclose(fp);
}

int main(int argc, char *argv[])
{
    char buff[MAX];
    int n,i;
    match_array match_array;
    init_match_array(&match_array);

    int sockfd, connfd, len; 
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verify
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    int pid;
    int id;        
    int num_of_tickets;
    while(1)
    {
        // Accept the data packet from client and verification
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server accept failed...\n");
            exit(0);
        }
        else
            printf("server accept the client...\n");
        
        pid = fork();
        if (pid == 0)
        {   
            while (1) {}
        }
        else
        {
            //send matches array
            for (i=0; i<match_array.size; i++)
            {
                sprintf(buff, "id: %d %s vs %s, %d tickets, %d euros/ticket\n", match_array.match[i].id, match_array.match[i].team1, match_array.match[i].team2, match_array.match[i].num_of_tickets, match_array.match[i].price);
                write(connfd, buff, sizeof(buff));
                bzero(buff, MAX);
            }
            //read id and number of tickets
            read(connfd, buff, sizeof(buff));
            id = atoi(buff);
            bzero(buff, MAX);

            read(connfd, buff, sizeof(buff));
            num_of_tickets = atoi(buff);
            bzero(buff, MAX);

            //check if there are enough tickets
            if (num_of_tickets > match_array.match[id].num_of_tickets)
            {
                sprintf(buff, "Not enough tickets available\n");
                write(connfd, buff, sizeof(buff));
                bzero(buff, MAX);
            }
            else
            {
                //change number of tickets, calculate price and send it to client
                match_array.match[id].num_of_tickets -= num_of_tickets;
                sprintf(buff, "You bought %d tickets for %d euros\n", num_of_tickets, num_of_tickets*match_array.match[id].price);
                write(connfd, buff, sizeof(buff));
                bzero(buff, MAX);
            }
            close(connfd);

        }
    }    
    //deallocation of memory
    for (i=0; i<match_array.size; i++)
    {
        free(match_array.match[i].team1);
        free(match_array.match[i].team2);
    }
    free(match_array.match);
}


    