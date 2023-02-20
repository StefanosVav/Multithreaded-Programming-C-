all: travelMonitorClient monitorServer

travelMonitorClient: main.cpp citizen.cpp BloomFilter.cpp SkipList.cpp date.cpp
	g++ -std=c++11 -o travelMonitorClient main.cpp citizen.cpp BloomFilter.cpp SkipList.cpp date.cpp

monitorServer: main2.cpp citizen.cpp BloomFilter.cpp SkipList.cpp date.cpp
	g++ -std=c++11 -o monitorServer main2.cpp citizen.cpp BloomFilter.cpp SkipList.cpp date.cpp

	
clean:
	rm travelMonitorClient monitorServer