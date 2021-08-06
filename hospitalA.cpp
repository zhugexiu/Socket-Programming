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
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <limits.h>
#include <float.h>//浮点数的极大值与极小值在float.h
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

#define UDP_PORT 30389
#define SCHEDULER_PORT 33389
#define LOCALHOST "127.0.0.1"
#define ERROR_FLAG -1
#define BUFLENGTH 1024
#define FILE_NAME "map.txt"
#define BACKLOG 100

int sockfdUDP; // UDP socket
struct sockaddr_in serverAddrUDP; //hospitalA use to create UDP 
struct sockaddr_in clientAddrUDP; //scheduler address for UDP connection
char recvMSG[BUFLENGTH];

const string BOOT_UP_MSG = "Bootup";
string location;
string capacity;
string occupancy;
int capacity_int;
int occupancy_int;

int point_number;  
double availability;
double score;
string score_null;
char clientPosition[BUFLENGTH];

vector<string> arr;//id_to_index
map<string,int> map_index;
set<string> pointSet;
vector<vector<string>> File_save;// save each line to build graph
int p_of_client;
int p_of_hospital;
string pos;

void readFile();

void createUDPSocket();

void sendInfo();

void getClientPosition();

void connectToScheduler();

int main(int argc, char *argv[]){  
    location = argv[1];
    capacity = argv[2];
    occupancy = argv[3]; 
    capacity_int = atoi(capacity.c_str());
    occupancy_int = atoi(occupancy.c_str());   
    createUDPSocket();
    //send first
    sendInfo();
    while(true) {
        getClientPosition();
        string choice = clientPosition;
        if(choice == "Update"){
            occupancy_int += 1;
            std::string occupancy = std::to_string(occupancy_int);
            availability = ((double)capacity_int - (double)occupancy_int)/(double)capacity_int;
            if(availability < 0 || availability > 1) {
                cout << "Hospital A has been assigned to a client, occupation is updated to " << occupancy << " ,availability is updated to None" << endl;
            }else{
                cout << "Hospital A has been assigned to a client, occupation is updated to " << occupancy << " ,availability is updated to ";
                cout << availability << endl;
            }
        }else{
            cout << "Hospital A has received input from client at location " << choice << endl;
            pos = clientPosition;
            p_of_client = atoi(pos.c_str());
            p_of_hospital = atoi(location.c_str());
            //cout << "Client Position: " << p_of_client << endl;
            //cout << "Hospital Position: " << p_of_hospital << endl;
            if(p_of_client == p_of_hospital) {
                char output[BUFLENGTH];
                string str = "SameNone";
                memset(output,'\0',sizeof(output));
                strncpy(output, str.c_str(), str.length() + 1);
                //send hospital output information to scheduler
                if(::sendto(sockfdUDP,output, sizeof(output), 0, (const struct sockaddr *) &clientAddrUDP,
                        sizeof(clientAddrUDP)) == ERROR_FLAG) {
                    perror("Hospital A response failed");
                    exit(1);
                }
                cout << "Hospital A has the same location with client" << endl;
            }else{
                readFile();
            }
        }
    }   
    close(sockfdUDP);    
    return 0;
}

void readFile(){
    //vector<int> id_to_index; //matrix的id->读取的index
    //map<string,int> index_to_id_Map; //读取的index到matrix的id
    //vector<vector<int>> fileMatrix;//adj matrix
    ifstream data_file(FILE_NAME, ios::in);
    string line_s;
    while(getline(data_file,line_s)){
        stringstream ss(line_s);
        string s;
        vector<string> lineArray;
        while(getline(ss,s,' ')){
            if(s == ""){
                continue;
            }
            lineArray.push_back(s);
        }
        File_save.push_back(lineArray);
        pointSet.insert(lineArray[0]);
        pointSet.insert(lineArray[1]);
    }
    point_number  = pointSet.size();
    int n = pointSet.size();
    double graph[n][n];
    for(int i = 0; i < point_number; i++) {
        for(int j = 0; j < point_number;j++) {
            graph[i][j] = 0;
        }
    }
    /**
    for(auto l: File_save){
        cout << l[0] << " " << l[1] << " "<< l[2] <<endl;
    }
    **/
    int index = 0;
    //build graph
    for(auto l: File_save){
        if(map_index.count(l[0]) > 0 && map_index.count(l[1]) > 0){
            //如果都有 说明重复 nc le here
            map<string, int>::iterator iter;
            iter = map_index.find(l[0]);
            int index1 = iter->second;

            map<string, int>::iterator iter2;
            iter2 = map_index.find(l[1]);
            int index2 = iter2->second;

            double val = atof(l[2].c_str());
            graph[index1][index2] = val;
            graph[index2][index1] = val;
        }else if(map_index.count(l[0]) > 0 && map_index.count(l[1]) == 0 ){
            //0 yes 1 no
            map<string, int>::iterator iter;
            iter = map_index.find(l[0]);
            int index1 = iter->second;

            //int index1 = map.get(l[0]);
            int index2 = index;
            //map.put(l[1],index);
            map_index.insert(pair<string, int>(l[1],index));
            arr.push_back(l[1]);
            index++;
            double val = atof(l[2].c_str());
            graph[index1][index2] = val;
            graph[index2][index1] = val; 
        }else if(map_index.count(l[1]) > 0 && map_index.count(l[0]) == 0){
            //有1没0
            int index0 = index;
            //int index1 = map.get(l[1]);
            map<string, int>::iterator iter;
            iter = map_index.find(l[1]);
            int index1 = iter->second;
            //map.put(l[0],index0);
            map_index.insert(pair<string, int>(l[0],index0));
            arr.push_back(l[0]);
            index++;
            double val = atof(l[2].c_str());
            graph[index0][index1] = val;
            graph[index1][index0] = val;
        }else{
            //both mei you
            int index0 = index;
            int index1 = index+1;
            map_index.insert(pair<string, int>(l[0],index0));
            arr.push_back(l[0]);
            map_index.insert(pair<string, int>(l[1],index1));
            arr.push_back(l[1]);
            double val = atof(l[2].c_str());
            graph[index0][index1] = val;
            graph[index1][index0] = val;
            index = index+2;
        }
    }
   /**
    for(int i = 0; i < point_number;i++) {
        cout << ""<< endl;
        for(int j = 0; j < point_number;j++) {
            cout << graph[i][j] << " ";
        }
    }
    cout << endl;**/
    
    if(map_index.count(pos) == 0){
        //client not in the map
        char output[BUFLENGTH];
        string str = "NotFoundNone";
        memset(output,'\0',sizeof(output));
        strncpy(output, str.c_str(), str.length() + 1);
        //send hospital output information to scheduler
        if(::sendto(sockfdUDP,output, sizeof(output), 0, (const struct sockaddr *) &clientAddrUDP,
                sizeof(clientAddrUDP)) == ERROR_FLAG) {
            perror("Hospital A response failed");
            exit(1);
        }
        cout << "Hospital A does not have the location " << pos <<" in map" << endl;
        cout << "Hospital A has sent \" location not found \" to the Scheduler" << endl;
    }else{
        //pos->client loc ->hospital
        map<string, int>::iterator iter_client;
        iter_client = map_index.find(pos);
        int src = iter_client->second;
        int V = point_number; 
        map<string, int>::iterator iter_hospital;
        iter_hospital = map_index.find(location);
        int loc = iter_hospital->second;

        /**
         *dijkstra algorithm form Greek
        **/
        double dist[V];
        //The output array.  dist[i] will hold the shortest
        //distance from src to i

        // sptSet[i] will be true if vertex i is included in shortest
        // path tree or shortest distance from src to i is finalized
        bool sptSet[V];

        for (int i = 0; i < V; i++){
            dist[i] = DBL_MAX;
            sptSet[i] = false;
        }
        dist[src] = 0; 

        for(int count = 0; count < V-1;count++) {
            //pick the minimum distance
            double min = DBL_MAX; 
            int min_index;
            for (int v = 0; v < V; v++)
                if (sptSet[v] == false && dist[v] <= min)
                    min = dist[v], min_index = v;
            int u = min_index;

            sptSet[u] = true;
            //update the value of new graph
            for(int v = 0; v < V; v++) {
                if (!sptSet[v] && graph[u][v] > 0 && dist[u] != DBL_MAX && dist[u] + graph[u][v] < dist[v])
                    dist[v] = dist[u] + graph[u][v];
            }
        }
        double dis_res;
        //printSolution(dist);
        //printf("Vertex \t\t Distance from Source\n");
        for (int i = 0; i < V; i++){
            //printf("%d \t\t %f\n", i, dist[i]);
            if(i == loc) dis_res = dist[i];
        }
        
        availability = ((double)capacity_int - (double)occupancy_int)/(double)capacity_int;

        if(availability >= 0 && availability <= 1) {
            cout << "Hospital A has capacity = " << capacity_int << " occupation = " << occupancy_int <<", availability = " << availability << endl;
            cout << "Hospital A has found the shortest path to client,distance = " << dis_res << endl;
            //score = 1/(d*(1.1-a))
            score = 1 / ((dis_res) * (1.1 - availability));
            cout << "Hospital A has the score = " << score << endl;
            char output[BUFLENGTH];
            string str = to_string(score) +"|"+ to_string(dis_res);
            memset(output,'\0',sizeof(output));
            strncpy(output, str.c_str(), str.length() + 1);
            //send hospital output information to scheduler
            if(::sendto(sockfdUDP,output, sizeof(output), 0, (const struct sockaddr *) &clientAddrUDP,
                    sizeof(clientAddrUDP)) == ERROR_FLAG) {
                perror("Hospital A response failed");
                exit(1);
            }
            cout << "Hospital A has sent score = " << to_string(score) << " and distance = " << to_string(dis_res) << " to the Scheduler" << endl;

        }else{
            cout << "Hospital A has capacity = " << capacity_int << " occupation = " << occupancy_int <<", availability = " << "None" << endl;
            cout << "Hospital A has found the shortest path to client,distance = " << dis_res << endl;
            cout << "Hospital A has the score = " << "None" << endl;
            char output[BUFLENGTH];
            string str = "ANone" + to_string(dis_res);
            memset(output,'\0',sizeof(output));
            strncpy(output, str.c_str(), str.length() + 1);
            //send hospital output information to scheduler
            if(::sendto(sockfdUDP,output, sizeof(output), 0, (const struct sockaddr *) &clientAddrUDP,
                    sizeof(clientAddrUDP)) == ERROR_FLAG) {
                perror("Hospital A response failed");
                exit(1);
            }
            cout << "Hospital A has sent score = " << "None" << " and distance = " << to_string(dis_res) << " to the Scheduler" << endl;
        }
    }
    
}

void createUDPSocket(){
    sockfdUDP = socket(AF_INET, SOCK_DGRAM, 0);
    //test if create a socket successfully
    if(sockfdUDP == ERROR_FLAG){
        perror("[Error]UDP socket");
        exit(1);
    }

    // from beej's tutorial
    memset(&serverAddrUDP, 0, sizeof(serverAddrUDP));
    serverAddrUDP.sin_family = AF_INET;
    serverAddrUDP.sin_port   = htons(UDP_PORT);
    serverAddrUDP.sin_addr.s_addr = inet_addr(LOCALHOST);

    // bind socket
    if(::bind(sockfdUDP, (struct sockaddr*) &serverAddrUDP, sizeof(serverAddrUDP)) == ERROR_FLAG){
        perror("ServerA UDP bind");
        exit(1);
    }
    printf("Hospital A is up and running using UDP on port %i. \nHospital A has total capacity %s and initial occupancy %s \n", UDP_PORT,capacity.c_str(), occupancy.c_str());

}

void sendInfo(){
    char hospitalInput[BUFLENGTH];
    string str = BOOT_UP_MSG + capacity + "|" + occupancy;
    memset(hospitalInput,'\0',sizeof(hospitalInput));
    strncpy(hospitalInput, str.c_str(), str.length() + 1);
    connectToScheduler();
    //send hospital input information to scheduler
    if(::sendto(sockfdUDP,hospitalInput, sizeof(hospitalInput), 0, (const struct sockaddr *) &clientAddrUDP,
            sizeof(clientAddrUDP)) == ERROR_FLAG) {
        perror("Hospital A response failed");
        exit(1);
    }

    char hospitalInputFeedback[BUFLENGTH];
    memset(hospitalInputFeedback,'\0',sizeof(hospitalInputFeedback));
    socklen_t hospitalA_UDP_len = sizeof(clientAddrUDP);
    if (::recvfrom(sockfdUDP, hospitalInputFeedback, BUFLENGTH, 0, (struct sockaddr *) &clientAddrUDP,
            &hospitalA_UDP_len) == ERROR_FLAG) {
        perror("HospitalA receive failed");
        exit(1);
    }

    cout << "Hospital A has sent the information to Scheduler" << endl;
    cout << endl;
}

void getClientPosition(){
    memset(clientPosition,'\0',sizeof(clientPosition));
    socklen_t hospitalA_UDP_len = sizeof(clientAddrUDP);
    if (::recvfrom(sockfdUDP, clientPosition, BUFLENGTH, 0, (struct sockaddr *) &clientAddrUDP,
            &hospitalA_UDP_len) == ERROR_FLAG) {
        perror("HospitalA receive failed");
        exit(1);
    }
}

void connectToScheduler(){
    // from beej's tutorial
    memset(&clientAddrUDP, 0, sizeof(clientAddrUDP));
    clientAddrUDP.sin_family = AF_INET;
    clientAddrUDP.sin_addr.s_addr = inet_addr(LOCALHOST);
    clientAddrUDP.sin_port = htons(SCHEDULER_PORT);
}

