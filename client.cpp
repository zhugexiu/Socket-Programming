#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <tuple>

using namespace std;

#define LOCALHOST "127.0.0.1"
#define SERVER_TCP_PORT 34389
#define BUFLENGTH 1024
#define ERROR_FLAG -1

int client_sockfd;
int server_sockfd;
struct sockaddr_in server_tcp_addr;
char clientInput[BUFLENGTH];
char serverOutput[BUFLENGTH];
string location;

void create_tcp();

void send_to_server();

void rec_from_server();

int main(int argc, char *argv[]){
    printf("The client is up and running\n");
    create_tcp();       
    location = argv[1];
    send_to_server();
    rec_from_server();
    close(client_sockfd);
    return 0;
}

// from beej's tutorial
void create_tcp(){
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);//create a socket  
    server_tcp_addr.sin_family = AF_INET;
    server_tcp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_tcp_addr.sin_port = htons(SERVER_TCP_PORT);
    if((connect(server_sockfd,(struct sockaddr *)&server_tcp_addr, sizeof(struct sockaddr))) == ERROR_FLAG){
        perror("[Error]connect error");
        exit(1);
    }
}

void send_to_server(){ 
    // from beej's tutorial  
    memset(clientInput,'\0', sizeof(clientInput));
    strncpy(clientInput, location.c_str(),BUFLENGTH);  
    //send data 
    if(send(server_sockfd, clientInput, sizeof(clientInput), 0) == ERROR_FLAG){
        perror("[Error]Error sending");
        close(client_sockfd);
        exit(1);
    }
    printf("The client has sent query to Scheduler using TCP: client location %s \n", location.c_str());
}

void rec_from_server(){ 
    //get result
    //from beej's tutorial
    memset(serverOutput,'\0', sizeof(serverOutput));
    if(recv(server_sockfd, serverOutput, sizeof(serverOutput),0) == ERROR_FLAG) {
        perror("[Error] client: fail to receive output from scheduler");
        close(client_sockfd);
        exit(1);
    }
    string res = serverOutput;
    if(res == "NotFound"){
        cout << "The client has received results from the Scheduler: assigned to Hospital None" << endl;
        cout << "Location " << location << " not found" << endl;
    }else if(res == "ScoreNone"){
        cout << "The client has received results from the Scheduler: assigned to Hospital None" << endl;
        cout << "Score = None, No assignment" << endl;
    }else{
        cout << "The client has received results from the Scheduler: assigned to Hospital " << serverOutput;
        cout << endl;
    }
}
