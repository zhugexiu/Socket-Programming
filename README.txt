***************************************************************************************************


Full Name: Xiuqi Zhuge
USC ID: 2533549389


***************************************************************************************************


What you have done in the assignment:

This project implements four coherent files - a front end client, scheduler (main server) and 3 hospitals (backend servers). The object of this assignment is to assign the client to the appropriate hospital based on its location and hospital's location, capacity and availability. The client establishes a TCP connection with the scheduler to send its own location and receive the result. The scheduler builds a UDP connection with hospital A, hospital B and hospital C to send client's location and receive hospitals' information and result. All three results return to scheduler, then scheduler decides which hospital will be assigned to client based on its own rules.

***************************************************************************************************

What your code files are and what each one of them does:

*client.cpp:
	Using './client <location index>' to input client's location after scheduler and three hospitals boot up. The client connects to the scheduler by TCP socket, then send its location and get back the result. 

-------------------------------------------------------------------------------------

*scheduler.cpp:
	The scheduler boots up firstly and configures connection with the client. The scheduler receives information of three hospitals by UDP connections. After scheduler receives client position information, scheduler sends client location to all three hospitals and waits three hospitals' information back. Then scheduler compare them and choose the most appropriate hospital to send to client.

-------------------------------------------------------------------------------------

*hospitalA.cpp && hospitalB.cpp && hosptalC.cpp:
	The hospital boots up after scheduler boots up. Hospital read "map.txt" file which contains location and distance of each point. I used a set to record how many points in this file, a matrix to record the graph of all points. Since points are not in order, I must use map and a vector to reindex these points. Then I used Dijkstra’s algorithm to find the shortest distance of client position and hospital location if it is legal. There are some corner cases to make result become illegal, like I cannot find client location in the map, availability is illegal, and so on.
 

***************************************************************************************************
The format of all the messages exchanged.

Client:
create_tcp(): initial the tcp with socket and connect.

send_to_server(): send client location to scheduler 

rec_from_server(): receive result from scheduler 
	1. After receiving output from the Scheduler: The client has received results from the         Scheduler: assigned to Hospital <A/B/C/None>.
	2. After receiving output the Scheduler, errors:
	Location <vertex index> not found
	or
	Score = None, No assignment.

-------------------------------------------------------------------------------------

Scheduler:

void create_bind_tcp_socket(): create TCP bind and listen

void createUDPSocket(): create UDP socket to connect with three hospitals.

void recv_from_A(), void recv_from_B(), void recv_from_C(): receive three hospitals' information such as occupancy and capacity.

void recv_from_A_result(), void recv_from_B_result(), void recv_from_C_result(): receive three hospitals final score and distance

void receive_from_client(): receive client position

void query_to_hospitalA(), void query_to_hospitalB(), void query_to_hospitalC(): send client location to three hospitals

void send_to_client():send the decision back to the client

void updateToA(), void updateToB(), void updateToC(): update hospital occupancy and if assign a client.

-------------------------------------------------------------------------------------
		
Hospital A&B&C:

void createUDPSocket(): create UDP socket to connect with scheduler

void sendInfo(): send occupancy and capacity to scheduler

void getClientPosition(): receive message from scheduler.

***************************************************************************************************
Any idiosyncrasy of your project. It should say under what conditions the project
fails, if any.

If the hospital location is not in the map, but we do not need to concern about this in this project.


***************************************************************************************************
Reused Code:

*for socket connections like bind(), sendto(), recvfrom() from Beej
*for Dijkstra’s algorithm from https://www.geeksforgeeks.org/dijkstras-shortest-path-algorithm-greedy-algo-7/


***************************************************************************************************

