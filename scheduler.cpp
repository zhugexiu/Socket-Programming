#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <float.h>//浮点数的极大值与极小值在float.h

using namespace std;

#define LOCALHOST "127.0.0.1"
#define SERVER_TCP_PORT 34389
#define A_UDP_PORT 30389
#define B_UDP_PORT 31389
#define C_UDP_PORT 32389
#define SERVER_UDP_PORT 33389
#define BACKLOG 100 // 有多少个特定的连接队列（pending connections queue）
#define ERROR_FLAG -1
#define BUFLENGTH 1024

int sockfdClient;
int sockfdServer;
int sockfdUDP;

struct sockaddr_in client_tcp_addr;
struct sockaddr_in server_tcp_addr;
struct sockaddr_in server_udp_addr;
struct sockaddr_in hospitalA_udp_addr;
struct sockaddr_in hospitalB_udp_addr;
struct sockaddr_in hospitalC_udp_addr;
socklen_t client_tcp_addr_size;

char clientInput[BUFLENGTH]; // input data from client
char AResult[BUFLENGTH]; // result returned from server A
char BResult[BUFLENGTH]; // result returned from server B
char CResult[BUFLENGTH]; // result returned from server C
char serverResult[BUFLENGTH]; // final result from server to client

char AInput[BUFLENGTH];//input from hospitalA
char BInput[BUFLENGTH];//input from hospitalB
char CInput[BUFLENGTH];//input from hospitalC

string res_send_to_client;//final message sent to scheduler

void handler(int sig);

void create_bind_tcp_socket();

void createUDPSocket();

void connectA();

void connectB();

void connectC();

void recv_from_A();

void recv_from_B();

void recv_from_C();

void recv_from_A_result();

void recv_from_B_result();

void recv_from_C_result();

void receive_from_client();

void query_to_hospitalA();

void query_to_hospitalB();

void query_to_hospitalC();

void send_to_client();

void updateToA();

void updateToB();

void updateToC();

int main(){
    struct sigaction act;
    act.sa_handler=handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags=0;
    sigaction(SIGCHLD, &act, 0);
    
    createUDPSocket();
    create_bind_tcp_socket();
    printf("The Scheduler is up and running \n");

    recv_from_A();
    recv_from_B();
    recv_from_C();

    if (sigaction(SIGCHLD, &act, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    while(true){
        //from beej's tutorial
        socklen_t clientAddrSize = sizeof(client_tcp_addr);
        sockfdClient = ::accept(sockfdServer, (struct sockaddr *) &client_tcp_addr, &clientAddrSize);
        if (sockfdClient == ERROR_FLAG) {
            perror("[ERROR] mainserver: fail to accept connection with client");
            exit(1);
        }
        //receive client location
        receive_from_client();
        //send location to hospitals
        query_to_hospitalA();
        recv_from_A_result();
        query_to_hospitalB();
        recv_from_B_result();
        query_to_hospitalC();
        recv_from_C_result();

        //decide which hospital to send
        string res_A = AResult;
        double score_A = DBL_MIN;
        double distance_A = DBL_MAX;
        string res_B = BResult;
        double score_B = DBL_MIN;
        double distance_B = DBL_MAX;
        string res_C = CResult;
        double score_C = DBL_MIN;
        double distance_C = DBL_MAX;
        
        if(res_A == "NotFoundNone" || res_B == "NotFoundNone" || res_C == "NotFoundNone" || res_A == "SameNone" || res_B == "SameNone" || res_C == "SameNone") {
            //地图上没有ｃｌｉｅｎｔ
            cout << "The Scheduler has received map information from Hospital A, the score = None and the distance = None" << endl;
            cout << "The Scheduler has received map information from Hospital B, the score = None and the distance = None" << endl;
            cout << "The Scheduler has received map information from Hospital C, the score = None and the distance = None" << endl;
            res_send_to_client = "NotFound";
        }else{
            //A
            if(res_A.substr(0,5) == "ANone"){
                string dis_ava_null = res_A.substr(5);
                cout << "The Scheduler has received map information from Hospital A, the score = None ";
                cout << "and the distance = " << dis_ava_null << endl;
            }else{
                char *splitStr = strtok(AResult,"|");
                string score_A_str = splitStr;
                score_A = atof(score_A_str.c_str());
                splitStr = strtok(NULL,"|");
                string distance_A_str = splitStr;
                distance_A = atof(distance_A_str.c_str());
                cout << "The Scheduler has received map information from Hospital A, the score = " << score_A_str; 
                cout << " and the distance = " << distance_A_str << endl;
            }
            //B
            if(res_B.substr(0,5) == "ANone"){
                string dis_ava_null = res_B.substr(5);
                cout << "The Scheduler has received map information from Hospital B, the score = None ";
                cout << "and the distance = " << dis_ava_null << endl;
            }else{
                char *splitStr = strtok(BResult,"|");
                string score_B_str = splitStr;
                score_B = atof(score_B_str.c_str());
                splitStr = strtok(NULL,"|");
                string distance_B_str = splitStr;
                distance_B = atof(distance_B_str.c_str());
                cout << "The Scheduler has received map information from Hospital B, the score = " << score_B_str; 
                cout << " and the distance = " << distance_B_str << endl;
            }
            //C
            if(res_C.substr(0,5) == "ANone"){
                string dis_ava_null = res_C.substr(5);
                cout << "The Scheduler has received map information from Hospital C, the score = None ";
                cout << "and the distance = " << dis_ava_null << endl;
            }else{
                char *splitStr = strtok(CResult,"|");
                string score_C_str = splitStr;
                score_C = atof(score_C_str.c_str());
                splitStr = strtok(NULL,"|");
                string distance_C_str = splitStr;
                distance_C = atof(distance_C_str.c_str());
                cout << "The Scheduler has received map information from Hospital C, the score = " << score_C_str; 
                cout << " and the distance = " << distance_C_str << endl;
            }
            if(score_A == DBL_MIN && score_B == DBL_MIN && score_C == DBL_MIN){
                res_send_to_client = "ScoreNone ";
            }else{
                if(score_A > score_B && score_A > score_C){
                    //assigned to A
                    cout << "The Scheduler has assigned Hospital A to the client" << endl;
                    res_send_to_client = "A";
                    updateToA(); 
                }else if(score_B > score_C && score_B > score_A){
                    //assigned to B
                    cout << "The Scheduler has assigned Hospital B to the client" << endl;
                    res_send_to_client = "B";
                    updateToB();
                }else if(score_C > score_A && score_C > score_B){
                    //assigned to C
                    cout << "The Scheduler has assigned Hospital C to the client" << endl;
                    res_send_to_client = "C";
                    updateToC();
                }else{
                    if(distance_A <= distance_B && distance_A <= distance_C){
                        cout << "The Scheduler has assigned Hospital A to the client" << endl;
                        res_send_to_client = "A";
                        updateToA(); 
                    }else if(distance_B < distance_A && distance_B < distance_C) {
                        cout << "The Scheduler has assigned Hospital B to the client" << endl;
                        res_send_to_client = "B";
                        updateToB();
                    }else if(distance_C < distance_A && distance_C < distance_B) {
                        cout << "The Scheduler has assigned Hospital C to the client" << endl;
                        res_send_to_client = "C";
                        updateToC();
                    }
                }
            }
        }
        send_to_client();
    }
    close(sockfdUDP); 
    close(sockfdClient);
    close(sockfdServer);
    return 0;

}

void handler(int sig){
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//create TCP socket for client and scheduler connection
void create_bind_tcp_socket(){   
    if((sockfdServer = socket(AF_INET, SOCK_STREAM, 0)) == ERROR_FLAG){
        perror("[Error]scheduler tcp socket error");
        exit(1);
    }

    // from beej's tutorial
    memset(&server_tcp_addr, 0, sizeof(server_tcp_addr));
    server_tcp_addr.sin_family = AF_INET;
    server_tcp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_tcp_addr.sin_port = htons(SERVER_TCP_PORT);

     // bind socket for client with IP address and port number for client
    if (::bind(sockfdServer, (struct sockaddr *) &server_tcp_addr, sizeof(server_tcp_addr)) == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to bind client socket");
        exit(1);
    }

    // listen to connections from clients
    if (::listen(sockfdServer, BACKLOG) == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to listen for client socket");
        exit(1);
    }
}

void createUDPSocket(){
   
    sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfdUDP == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to create UDP socket");
        exit(1);
    }

    // from beej's tutorial
    memset(&server_udp_addr, 0, sizeof(server_udp_addr));
    server_udp_addr.sin_family = AF_INET;
    server_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    server_udp_addr.sin_port = htons(SERVER_UDP_PORT);

    // bind socket
    if (::bind(sockfdUDP, (struct sockaddr *) &server_udp_addr, sizeof(server_udp_addr)) == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to bind UDP socket");
        exit(1);
    }
}

void connectA(){
    // from beej's tutorial
    memset(&hospitalA_udp_addr, 0, sizeof(hospitalA_udp_addr));
    hospitalA_udp_addr.sin_family = AF_INET;
    hospitalA_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    hospitalA_udp_addr.sin_port = htons(A_UDP_PORT);
}

void connectB(){
    // from beej's tutorial
    memset(&hospitalB_udp_addr, 0, sizeof(hospitalB_udp_addr));
    hospitalB_udp_addr.sin_family = AF_INET;
    hospitalB_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    hospitalB_udp_addr.sin_port = htons(B_UDP_PORT);
}

void connectC(){
    // from beej's tutorial
    memset(&hospitalC_udp_addr, 0, sizeof(hospitalC_udp_addr));
    hospitalC_udp_addr.sin_family = AF_INET;
    hospitalC_udp_addr.sin_addr.s_addr = inet_addr(LOCALHOST);
    hospitalC_udp_addr.sin_port = htons(C_UDP_PORT);
}   

void recv_from_A(){
    memset(AInput,'\0',sizeof(AInput));
    socklen_t serverAUDPLen = sizeof(hospitalA_udp_addr);
    if (::recvfrom(sockfdUDP, AInput, BUFLENGTH, 0, (struct sockaddr *) &hospitalA_udp_addr, &serverAUDPLen) == ERROR_FLAG) {
        perror("serverA receive failed");
        exit(1);
    }
    string MSG = AInput;
    //cout << MSG.substr(0,6) << endl;
    if(MSG.substr(0,6) == "Bootup") {
        char *splitStr = strtok(AInput,"|");
        string capacityPlusMSG = splitStr;
        string capacity = capacityPlusMSG.substr(6);
        splitStr = strtok(NULL,"|");
        string occupancy = splitStr;
        cout << "The scheduler has received information from Hospital A: total capacity is " << capacity;
        cout << " and initial occupancy is " << occupancy << endl;
    }else{
       cout << "Error" << endl;
    }

    char Feedback[BUFLENGTH];
    memset(Feedback,'\0',sizeof(Feedback));
    string str = "recvOK";
    strncpy(Feedback, str.c_str(), strlen(str.c_str()));
    if (::sendto(sockfdUDP, Feedback, sizeof(Feedback), 0, (const struct sockaddr *) &hospitalA_udp_addr,
                 sizeof(hospitalA_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input bootup message to server A");
        exit(1);
    }
}

void recv_from_B(){
    memset(BInput,'\0',sizeof(BInput));
    socklen_t serverBUDPLen = sizeof(hospitalB_udp_addr);
    if (::recvfrom(sockfdUDP, BInput, BUFLENGTH, 0, (struct sockaddr *) &hospitalB_udp_addr, &serverBUDPLen) == ERROR_FLAG) {
        perror("serverB receive failed");
        exit(1);
    }
    string MSG = BInput;
    //cout << MSG.substr(0,6) << endl;
    if(MSG.substr(0,6) == "Bootup") {
        char *splitStr = strtok(BInput,"|");
        string capacityPlusMSG = splitStr;
        string capacity = capacityPlusMSG.substr(6);
        splitStr = strtok(NULL,"|");
        string occupancy = splitStr;
        cout << "The scheduler has received information from Hospital B: total capacity is " << capacity;
        cout << " and initial occupancy is " << occupancy << endl;
    }else{
       cout << "Error" << endl;
    }

    char Feedback[BUFLENGTH];
    memset(Feedback,'\0',sizeof(Feedback));
    string str = "recvOK";
    strncpy(Feedback, str.c_str(), strlen(str.c_str()));
    if (::sendto(sockfdUDP, Feedback, sizeof(Feedback), 0, (const struct sockaddr *) &hospitalB_udp_addr,
                 sizeof(hospitalB_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input bootup message to server B");
        exit(1);
    }
}

void recv_from_C(){
    memset(CInput,'\0',sizeof(CInput));
    socklen_t serverCUDPLen = sizeof(hospitalC_udp_addr);
    if (::recvfrom(sockfdUDP, CInput, BUFLENGTH, 0, (struct sockaddr *) &hospitalC_udp_addr, &serverCUDPLen) == ERROR_FLAG) {
        perror("serverC receive failed");
        exit(1);
    }
    string MSG = CInput;
    //cout << MSG.substr(0,6) << endl;
    if(MSG.substr(0,6) == "Bootup") {
        char *splitStr = strtok(CInput,"|");
        string capacityPlusMSG = splitStr;
        string capacity = capacityPlusMSG.substr(6);
        splitStr = strtok(NULL,"|");
        string occupancy = splitStr;
        cout << "The scheduler has received information from Hospital C: total capacity is " << capacity;
        cout << " and initial occupancy is " << occupancy << endl;
    }else{
       cout << "Error" << endl;
    }

    char Feedback[BUFLENGTH];
    memset(Feedback,'\0',sizeof(Feedback));
    string str = "recvOK";
    strncpy(Feedback, str.c_str(), strlen(str.c_str()));
    if (::sendto(sockfdUDP, Feedback, sizeof(Feedback), 0, (const struct sockaddr *) &hospitalC_udp_addr,
                 sizeof(hospitalC_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input bootup message to server C");
        exit(1);
    }
}

void recv_from_A_result()
{        
    memset(AResult,'\0',sizeof(AResult));
    socklen_t serverAUDPLen = sizeof(hospitalA_udp_addr);
    if (::recvfrom(sockfdUDP, AResult, BUFLENGTH, 0, (struct sockaddr *) &hospitalA_udp_addr, &serverAUDPLen) == ERROR_FLAG) {
        perror("serverA receive failed");
        exit(1);
    }
}

void recv_from_B_result()
{        
    memset(BResult,'\0',sizeof(BResult));
    socklen_t serverBUDPLen = sizeof(hospitalB_udp_addr);
    if (::recvfrom(sockfdUDP, BResult, BUFLENGTH, 0, (struct sockaddr *) &hospitalB_udp_addr, &serverBUDPLen) == ERROR_FLAG) {
        perror("serverB receive failed");
        exit(1);
    }
}

void recv_from_C_result()
{        
    memset(CResult,'\0',sizeof(CResult));
    socklen_t serverCUDPLen = sizeof(hospitalC_udp_addr);
    if (::recvfrom(sockfdUDP, CResult, BUFLENGTH, 0, (struct sockaddr *) &hospitalC_udp_addr, &serverCUDPLen) == ERROR_FLAG) {
        perror("serverC receive failed");
        exit(1);
    } 
}

void receive_from_client(){
    // from beej's tutorial
    memset(&clientInput, 0, sizeof(clientInput));
    // receive through child socket
    if (recv(sockfdClient, clientInput, BUFLENGTH, 0) == ERROR_FLAG) {
        perror("[ERROR] scheduler: fail to receive input data from client");
        exit(1);
    }
    string loc = clientInput;
    cout << "The Scheduler has received client at location " << loc << " from the client using TCP over port " << SERVER_TCP_PORT << endl;
}

//send client position to hospital
void query_to_hospitalA(){
    string loc = clientInput;
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, loc.c_str(), strlen(loc.c_str()));
    connectA();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalA_udp_addr,
                 sizeof(hospitalA_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input message to server A");
        exit(1);
    }
    cout << "The Scheduler has sent client location to Hospital A using UDP over port " << SERVER_UDP_PORT<< endl;
}

void query_to_hospitalB(){
    string loc = clientInput;
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, loc.c_str(), strlen(loc.c_str()));
    connectB();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalB_udp_addr,
                 sizeof(hospitalB_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input message to server B");
        exit(1);
    }
    cout << "The Scheduler has sent client location to Hospital B using UDP over port " << SERVER_UDP_PORT<< endl;
}

void query_to_hospitalC(){
    string loc = clientInput;
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, loc.c_str(), strlen(loc.c_str()));
    connectC();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalC_udp_addr,
                 sizeof(hospitalC_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send input message to server C");
        exit(1);
    }
    cout << "The Scheduler has sent client location to Hospital C using UDP over port " << SERVER_UDP_PORT<< endl;
}


void send_to_client(){
    string res = res_send_to_client;
    memset(&serverResult, 0, sizeof(serverResult));
    strncpy(serverResult, res.c_str(), res.length() + 1);
    if (sendto(sockfdClient, serverResult, sizeof(serverResult),
            0,(struct sockaddr *) &client_tcp_addr,
            sizeof(client_tcp_addr)) == ERROR_FLAG) {
    perror("[ERROR] scheduler: fail to send computed result to client");
    exit(1);
    }
    cout << "The Scheduler has sent the result to client using TCP over port " << SERVER_TCP_PORT << endl;
}

void updateToA(){
    string UMSG = "Update";
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, UMSG.c_str(), strlen(UMSG.c_str()));
    connectA();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalA_udp_addr,
                 sizeof(hospitalA_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send update message to hospital A");
        exit(1);
    }
    cout << "The Scheduler has sent the result to Hospital A using UDP over port " << SERVER_UDP_PORT<< endl;
}

void updateToB(){
    string UMSG = "Update";
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, UMSG.c_str(), strlen(UMSG.c_str()));
    connectB();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalB_udp_addr,
                 sizeof(hospitalB_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send update message to hospital B");
        exit(1);
    }
    cout << "The Scheduler has sent the result to Hospital B using UDP over port " << SERVER_UDP_PORT<< endl;
}

void updateToC(){
    string UMSG = "Update";
    char dataBuff[BUFLENGTH];
    memset(dataBuff,'\0',sizeof(dataBuff));
    strncpy(dataBuff, UMSG.c_str(), strlen(UMSG.c_str()));
    connectC();
    if (::sendto(sockfdUDP, dataBuff, sizeof(dataBuff), 0, (const struct sockaddr *) &hospitalC_udp_addr,
                 sizeof(hospitalC_udp_addr)) == ERROR_FLAG) {
        perror("[Error] scheduler: fail to send update message to hospital C");
        exit(1);
    }
    cout << "The Scheduler has sent the result to Hospital C using UDP over port " << SERVER_UDP_PORT<< endl;
}