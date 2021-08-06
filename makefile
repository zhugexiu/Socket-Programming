all:scheduler.cpp hospitalA.cpp hospitalB.cpp hospitalC.cpp client.cpp
	g++ -std=c++11 scheduler.cpp -o scheduler

	g++ -std=c++11 hospitalA.cpp -o hospitalA

	g++ -std=c++11 hospitalB.cpp -o hospitalB

	g++ -std=c++11 hospitalC.cpp -o hospitalC

	g++ -std=c++11 client.cpp -o client

.PHONY: scheduler
scheduler: 
	./scheduler

.PHONY: hospitalA
hospitalA: 
	./hospitalA

.PHONY: hospitalB
hospitalB:
	./hospitalB

.PHONY: hospitalC
hospitalC:
	./hospitalC

.PHONY: client
client:
	./client
