#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <signal.h>
#include <netdb.h>
#include <pthread.h>
#include "citizen.h"
#include "BloomFilter.h"
#include "SkipList.h"
using namespace std;
extern int errno;
void perror_exit(char *message);
void child_server(int newsock);
void sigchld_handler (int sig);

int main(int argc, char *argv[]) {

    // PART 1: GET INPUT FROM EXECV AND SAVE IT
    int numThreads, socketBufferSize, cyclicBufferSize, sizeOfBloom, port, numCountries;
    string path[10]{""};
    if(argc < 12){
        cout << "Error in  Input" << endl;
        return 0;
    }
    else{                               //Get data from command line input
        string arg1 = argv[1], arg3 = argv[3], arg5 = argv[5], arg7 = argv[7], arg9 = argv[9];

        if(arg1 == "-p" && arg3 == "-t" && arg5 == "-b" && arg7 == "-c" && arg9 == "-s"){
            port = atoi(&argv[2][0]);
            numThreads = atoi(&argv[4][0]);
            socketBufferSize = atoi(&argv[6][0]);
            cyclicBufferSize = atoi(&argv[8][0]);
            sizeOfBloom = atoi(&argv[10][0]);
            int i = 0;
            while(11+i < argc){
                path[i] = argv[11+i];
                cout << "in client with pid: " << getpid() << " -- path" << i << ": " << path[i] << endl;;
                i++;
            }
            numCountries = i;
        }
        else{
            cout << "Error in  Input" << endl;
            return 0;
        }
    }


    // PART 2: INITIALIZE DATA STRUCTURES BY PARSING THROUGH ASSIGNED FILES
    CitizenList *cList = new CitizenList();         //Create a Citizen List that holds the records from the input file
    BFL *bfl = new BFL(sizeOfBloom);                //Create a Bloom Filters List that holds a (pointer to a) Bloom Filter object for each virus
    SLL *vaccinatedSL = new SLL();                  //Create a vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus
    SLL *nonvaccinatedSL = new SLL();               //Create a non vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus

    for(int i = 0; i < numCountries; i++){

        DIR *d;
        struct dirent *dir;
        string directory = path[i];
        char *dirPath = &directory[0];
        d = opendir(dirPath);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                
                string file(dir->d_name);
                if (file == ".." || file == "."){
                    continue;
                }

                string record;
                string filePath = directory + "/" + file;
                ifstream CRF(filePath);

                while(getline(CRF, record)){
                    ERROR = false;                          //Global Variable ERROR, checks for record inconsistencies (used in the citizen constructor - citizen.cpp)

                    citizen *c = new citizen(record);       //Create citizen object initialized with the characteristics of the current line(record) in the input file
                    if (ERROR || !consistent(c, cList)){
                        //cout << "--Error in record: " << record << endl;
                        delete c;                           //Free space
                        continue;
                    }
                    
                    cList->insert(c);                       //If the new record is consistent, add it to our Citizen List,
                    if(c->getVaccinated() == "YES"){        //our vaccinated or non vaccinated skip list
                        vaccinatedSL->insert(c);
                        bfl->insert(c);                     //and our Bloom Filters list
                    }else{
                        nonvaccinatedSL->insert(c);
                    }
                }
                CRF.close();
            }
            closedir(d);
        }
    }
    
    //Optional Print to see that our data structures are initialized correctly
    /*
    cout << endl;
    //cout << endl << "----------------RECORDS-----------------" << endl;
    //cList->print();
    cout << endl << "-------------BLOOM FILTERS--------------" << endl;
    bfl->print();
    cout << endl << "---------VACCINATED SKIP LISTS----------" << endl;
    vaccinatedSL->print();
    cout << endl << "-------NON VACCINATED SKIP LISTS--------" << endl;
    nonvaccinatedSL->print();
    cout << endl;
    */


    // PART 3: INITIALIZING CONNECTION THROUGH SOCKETS WITH PARENT-CLIENT
    int sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;

    // Reap dead children asynchronously
    signal(SIGCHLD, sigchld_handler);

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit((char*)"socket");
    
    server.sin_family = AF_INET;                // Internet domain
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);              // The given port

    // Bind socket to address
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit((char*)"bind");

    //cout << "Socket name: " << getsockname(sock, serverptr, &clientlen) << endl;

    // Listen for connections
    if (listen(sock, 5) < 0) 
        perror_exit((char*)"listen");
    cout << "Server-Child listening for connections to port " << port << endl;

    clientlen = sizeof(client);
    // accept connection
    if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
        perror_exit((char*)"accept");
    // Find client's name
    if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL) {
        herror("gethostbyaddr"); 
        exit(1);
    }
    cout << "Server-Child accepted connection from " << rem->h_name << endl;


    // PART 4: SERIALIZING AND SENDING BLOOM FILTERS TO PARENT PROCESS
    char buf[256];
    memset(buf, '\0', 256);

    strcpy(buf, bfl->getViruses().c_str());
    if (write(newsock, buf, sizeof(buf)) < 0)
        perror_exit((char*)"write");

    close(newsock);
    memset(buf, '\0', 256);

    /*int i = 0;
    while(1){
        char * bloom = bfl->serialize(i);
        strcpy(buf,bloom);

        if(bloom[0] == -1){
            cout << "stop writing" << endl;
            if (write(newsock, buf, sizeof(buf)) < 0)
                perror_exit((char*)"write");
            delete[] bloom;
            break;
        }
        if (write(newsock, buf, sizeof(buf)) < 0)
            perror_exit((char*)"write");
        delete[] bloom;
        i++; 
    }*/



    // PART 5: RECEIVE COMMANDS-REQUESTS FROM PARENT-CLIENT AND REACT ACCORDINGLY 
    int TRC = 0, ARC = 0, DRC = 0;                  //Travel Request Counter, Accepted Requests Counter, Denied Requests Counter 
    string buffer;
    while(1){

        if (listen(sock, 5) < 0) 
            perror_exit((char*)"listen");
        clientlen = sizeof(client);
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
            perror_exit((char*)"accept");
        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof(client.sin_addr.s_addr), client.sin_family)) == NULL) {
            herror("gethostbyaddr"); 
            exit(1);
        }

        if (read(newsock, buf, sizeof(buf)) < 0)
            perror_exit((char*)"read");

        buffer = buf;

        if(buffer.find("travelRequest") != string::npos){

            string virus, ID;
            int k = 0;
            char *b = &buffer[0];
            char *token = strtok(b," ");
            while (token){
                if(k==1){
                    ID = token;
                }else if(k==2){
                    virus = token;
                }
                token = strtok(NULL," ");
                k++;
            }
            if(bfl->check(ID, virus)){
                strcpy(buf, "YES");
                if (write(newsock, buf, sizeof(buf)) < 0)
                    perror_exit((char*)"read");
                
                TRC++; ARC++;
            }else{
                strcpy(buf, "NO");
                if (write(newsock, buf, sizeof(buf)) < 0)
                    perror_exit((char*)"read");

                TRC++; DRC++;
            }
        }

        else if(buffer.find("searchVaccinationStatus") != string::npos){
            string ID;
            int k;
            for(k = 0; k < buffer.length(); k++){
                if(buffer[k] == ' '){
                    k++;
                    break;
                }
            }

            ID = buffer.substr(k,buffer.length()-1);
            vaccinatedSL->findAllRecords(ID);
            nonvaccinatedSL->findAllRecords(ID);
        }

        else if(buffer.find("exit") != string::npos){
            
            cout << "Process " << getpid() << " received exit message from Parent. Exiting..." << endl;
            close(newsock);

            // Create and open log_file.xxx
            string filename = "log_file." + to_string(getpid());
            ofstream MyFile(filename);

            // Write to the file
            int i = 0;
            while(11+i < argc){
                MyFile << argv[11+i] << endl;
                i++;
            }

            MyFile << "TOTAL REQUESTS: " << TRC << endl;
            MyFile << "ACCEPTED: " << ARC << endl;
            MyFile << "REJECTED: " << DRC << endl;

            // Close the file
            MyFile.close();

            break;
        }
        close(newsock);
    }
    
    close(sock);

    delete bfl;
    delete cList;
    delete vaccinatedSL;
    delete nonvaccinatedSL;

    return 0;
}

// Wait for all dead child processes
void sigchld_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}