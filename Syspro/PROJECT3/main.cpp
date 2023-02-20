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
#include "date.h"
#include "citizen.h"
#include "BloomFilter.h"
#include "SkipList.h"
void perror_exit(char *message);
using namespace std;
extern int errno;

int main(int argc, char *argv[]) {


    // PART 1: GET INPUT FROM COMMAND LINE AND SAVE IT
    string input_dir;
    int  numMonitors, socketBufferSize, cyclicBufferSize, sizeOfBloom, numThreads;
    if(argc != 13){
        cout << "Error in  Input" << endl;
        return 0;
    }
    else{                               //Get data from command line input
        string arg1 = argv[1], arg3 = argv[3], arg5 = argv[5], arg7 = argv[7], arg9 = argv[9], arg11 = argv[11];

        if(arg1 == "-m" && arg3 == "-b" && arg5 == "-c" && arg7 == "-s" && arg9 == "-i" && arg11 == "-t"){
            numMonitors = atoi(&argv[2][0]);
            socketBufferSize = atoi(&argv[4][0]);
            cyclicBufferSize = atoi(&argv[6][0]);
            sizeOfBloom = atoi(&argv[8][0]);
            input_dir = argv[10];
            numThreads = atoi(&argv[12][0]);
        }
        else{
            cout << "Error in  Input" << endl;
            return 0;
        }
    }

    cout << endl;
    int port = 50005;                 //49152-65535


    // PART 2: FORK NUM MONITORS CHILD PROCESSES AND EXECV MONITORSERVER FOR EACH ONE WITH THEIR RESPECTIVE ARGUMENTS
    int PIDarray[numMonitors];                          //array to hold PID0 in index0, PID1 in index1 etc.
    string cbm;                                         //string to hold countries assigned to monitors (CountryByMonitor)
    int pid;
    for (int i = 0; i < numMonitors; i++){

        //Assign country directories to clients
        DIR *d;
        struct dirent *dir;
        d = opendir("./input_dir");
        int maxCAMS = 10;                       //max number of countries assigned per monitorServer
        string countries[maxCAMS]{""};          //countries array for each monitor
        string file;
        if (d) {
            int counter = 0;
            while ((dir = readdir(d)) != NULL) {        //assigning countries to this monitor's countries array with RoundRobin

                file = dir->d_name;
                if (file == ".." || file == "."){
                    continue;
                }
                if(counter==i){
                    
                    for(int j = 0; j < maxCAMS; j++){

                        if(countries[j] == ""){
                            countries[j] = file;
                            //snippet of code to save countries assigned to each monitor in a string(used later in user commands)
                            char c = i+'0';
                            cbm.push_back(c);
                            cbm.append(file);
                            break;
                        }
                    }
                }
                if (++counter == numMonitors){
                    counter = 0;
                }
            }
            closedir(d);
        }

        cout << "Countries assigned to Monitor Server" << i << ": ";
        int cams = 0;                               //Number of countries assigned to this monitor server, used for creating array for execv
        while(countries[cams]!=""){
            cout << countries[cams] << " ";
            cams++;
        }
        cout << endl;
        
        char * arr[12+cams];                        //create array with arguments for execv
        char p[5];
        sprintf(p, "%d", port+i);                   //get char array p from int port
        arr[0] = (char*)"./monitorServer";          //assign arguments
        arr[1] = (char*)"-p";
        arr[2] = p;
        arr[3] = (char*)"-t"; 
        arr[4] = argv[12];
        arr[5] = (char*)"-b";
        arr[6] = argv[4];
        arr[7] = (char*)"-c";
        arr[8] = argv[6];
        arr[9] = (char*)"-s";
        arr[10] = argv[8];
        string path;
        for(int i = 0; i < cams; i++){              //assign country paths for this monitor
            path = input_dir + "/" + countries[i];
            arr[11+i] = new char[sizeof(path)+1];
            strcpy(arr[11+i], path.c_str());
        }
        arr[11+cams] = NULL;

        pid = fork();
        if(pid < 0){
            cout << "fork error" << endl;
            return 0;
        }
        else if(pid == 0){               //child process
            if(execv("./monitorServer", arr) == -1){        //call execv with array of given arguments for this monitor
                perror("exec failed");
            }
        }
        else{                           //parent process
            PIDarray[i] = pid;
        }
    }
    cout << endl;


    // PART 3: CONNECT TO MONITOR SERVERS THROUGH SOCKETS WITH ASSIGNED PORTS AND RECEIVE BLOOM FILTERS
    char buf[256];
    memset(buf, '\0', 256);
    bool printIP = false;
    for(int i = 0; i < numMonitors; i++){
        int sock;
        
        struct sockaddr_in server;
        struct sockaddr *serverptr = (struct sockaddr*)&server;
        struct hostent *rem;
        
        // Create Socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            perror_exit((char*)"socket");

        // Find host name
        char hostname[HOST_NAME_MAX + 1];
        gethostname(hostname, HOST_NAME_MAX + 1);

        // Find server address
        rem = gethostbyname(hostname);
        if(printIP==false){
            printIP = true;
            cout << "Parent-Client Host Name: " << rem->h_name << endl;
            cout << "IP address: " << inet_ntoa(*(struct in_addr*)rem->h_addr) << endl << endl;
        }

        server.sin_family = AF_INET;                        // Internet domain
        memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
        server.sin_port = htons(port+i);                    //Server port

        // Initiate Connection
        int connectStatus;
        do
            connectStatus = connect(sock, serverptr, sizeof(server));
        while (connectStatus < 0);
        cout << "Parent-Client connecting to " << hostname << " port: " << port+i << endl;

        /*
        while(1){
            //  Read bloom filters
            if(read(sock, buf, sizeof(buf)) < 0)
                perror_exit((char*)"read");

            if(buf[0]==-1){
                break;
            }
        }
        //bfl->deserialize(buf);*/

        if(read(sock, buf, sizeof(buf)) < 0)
            perror_exit((char*)"read");

        cout << endl << "Parent Receiving bloom filters from monitorServer " << i << ": " << buf << endl;
        
        close(sock);                // Close socket and exit

    }


    // PART 4: GET INPUT FROM USER AND PERFORM EACH COMMAND
    memset(buf, '\0', 256);
    string input;
    string arg[10];                                 //Array of arguments given by the user with each input
    int k;
    int TRC = 0, ARC = 0, DRC = 0;                  //Travel Request Counter, Accepted Requests Counter, Denied Requests Counter 
    while(true){
        sleep(1);
        cout << endl << "ENTER COMMAND:" << endl;
        for (int j = 0; j<10; j++){
            arg[j] = "";                            //Set all arguments in the array as an empty string
        }
        k = 0;
        getline(cin, input);
        char *inputC = &input[0];                   //convert string to char * in order to use strtok
        char *token = strtok(inputC," ,");
        while (token){
            arg[k] = token;                         //Set arguments as each word of the user's input (using strtok)
            token = strtok(NULL," ,");
            k++;
        }

        //COMMAND = /exit
        if (arg[0] == "/exit"){
            for (int i = 0; i < numMonitors; i++){

                int sock;
            
                struct sockaddr_in server;
                struct sockaddr *serverptr = (struct sockaddr*)&server;
                struct hostent *rem;
                
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    perror_exit((char*)"socket");

                char hostname[HOST_NAME_MAX + 1];
                gethostname(hostname, HOST_NAME_MAX + 1);

                rem = gethostbyname(hostname);

                server.sin_family = AF_INET;                        // Internet domain
                memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
                server.sin_port = htons(port+i);                    //Server port

                int connectStatus;
                do
                    connectStatus = connect(sock, serverptr, sizeof(server));
                while (connectStatus < 0);

                string buffer = arg[0];
                strcpy(buf, buffer.c_str());
                if(write(sock, buf, sizeof(buf)) < 0)
                    perror_exit((char*)"write");

                memset(buf, '\0', 256);
            }

            // Create and open log_file.xxx
            string filename = "log_file." + to_string(getpid());
            ofstream MyFile(filename);

            // Write to the file
            string word = "";
            for (auto x : cbm) 
            {
                if (x >= '0' && x<='9')
                {
                    MyFile << word << endl;
                    word = "";
                }
                else {
                    word = word + x;
                }
            }
            MyFile << word << endl;

            MyFile << "TOTAL REQUESTS: " << TRC << endl;
            MyFile << "ACCEPTED: " << ARC << endl;
            MyFile << "REJECTED: " << DRC << endl;

            // Close the file
            MyFile.close();

            break;
        }


        //COMMAND = /travelRequest citizenID date countryFrom countryTo virusName
        else if(arg[0] == "/travelRequest" && arg[5]!=""){
            //arg[1]: CitizenID, arg[2]: date, arg[3]: countryFrom, arg[4]: countryTo, arg[5]: virusName

            //find monitorServer who was assigned countryFrom (using cbm string)
            int ms;                     
            for (int i = 0; i < cbm.length(); i++){
                // Find occurrence of "countryFrom"
                size_t found = cbm.find(arg[3]);
                if (found != string::npos){

                    char monser = cbm[found-1];
                    ms = monser - '0';
                    break;
                }
            }

            cout << arg[3] << " was assigned to MonitorServer " << ms << endl;
            int sock;
        
            struct sockaddr_in server;
            struct sockaddr *serverptr = (struct sockaddr*)&server;
            struct hostent *rem;
            
            // Create Socket
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                perror_exit((char*)"socket");

            // Find host name
            char hostname[HOST_NAME_MAX + 1];
            gethostname(hostname, HOST_NAME_MAX + 1);

            // Find server address
            rem = gethostbyname(hostname);

            server.sin_family = AF_INET;                        // Internet domain
            memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
            server.sin_port = htons(port+ms);                    //Server port

            // Initiate Connection
            int connectStatus;
            do
                connectStatus = connect(sock, serverptr, sizeof(server));
            while (connectStatus < 0);

            string buffer = arg[0]+" "+arg[1]+" "+arg[5];
            strcpy(buf, buffer.c_str());
            if(write(sock, buf, sizeof(buf)) < 0)
                perror_exit((char*)"write");

            memset(buf, '\0', 256);

            if(read(sock, buf, sizeof(buf)) < 0)
                perror_exit((char*)"read");
            
            buffer = buf;
    
            if(buffer=="YES"){
                cout << "REQUEST ACCEPTED – HAPPY TRAVELS" << endl;
                ARC++; TRC++;

            //if(date)
                //cout << "RREQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE" << endl;
            }else{
                cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
                DRC++; TRC++;
            }
        }
        

        //COMMAND = /travelStats virusName date1 date2
        else if(arg[0] == "/travelStats" && arg[3]!=""){
            //arg[1]: virusName, arg[2]: date1, arg[3]: date2

            cout << "TOTAL REQUESTS: " << TRC << endl;
            cout << "ACCEPTED: " << ARC << endl;
            cout << "REJECTED: " << DRC << endl;
        }


        //COMMAND = /searchVaccinationStatus citizenID
        else if(arg[0] == "/searchVaccinationStatus" && arg[1]!=""){
            //arg[1]: CitizenID

            for (int i = 0; i < numMonitors; i++){

                int sock;
            
                struct sockaddr_in server;
                struct sockaddr *serverptr = (struct sockaddr*)&server;
                struct hostent *rem;
                
                // Create Socket
                if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                    perror_exit((char*)"socket");

                // Find host name
                char hostname[HOST_NAME_MAX + 1];
                gethostname(hostname, HOST_NAME_MAX + 1);

                // Find server address
                rem = gethostbyname(hostname);

                server.sin_family = AF_INET;                        // Internet domain
                memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
                server.sin_port = htons(port+i);                    //Server port

                // Initiate Connection
                int connectStatus;
                do
                    connectStatus = connect(sock, serverptr, sizeof(server));
                while (connectStatus < 0);

                string buffer = arg[0]+" "+arg[1];
                strcpy(buf, buffer.c_str());
                if(write(sock, buf, sizeof(buf)) < 0)
                    perror_exit((char*)"write");

                memset(buf, '\0', 256);
            }
        }
        else{
            cout << "INVALID COMMAND, TRY AGAIN" << endl;
        }
    }
    
    return 0;
}

void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}