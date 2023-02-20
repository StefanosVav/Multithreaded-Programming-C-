#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
//#include <ctime>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include "date.h"
#include "citizen.h"
#include "BloomFilter.h"
#include "SkipList.h"
using namespace std;
extern int errno;

int main(int argc, char *argv[]) {

    //PART 1: GET INPUT FROM COMMAND LINE AND SAVE IT
    string input_dir;
    int  numMonitors, bufferSize, sizeOfBloom;
    if(argc != 9){
        cout << "Error in  Input" << endl;
        return 0;
    }
    else{                               //Get data from command line input
        string arg1 = argv[1], arg3 = argv[3], arg5 = argv[5], arg7 = argv[7];

        if(arg1 == "-m" && arg3 == "-b" && arg5 == "-s" && arg7 == "-i"){
            numMonitors = atoi(&argv[2][0]);
            bufferSize = atoi(&argv[4][0]);
            sizeOfBloom = atoi(&argv[6][0]);
            input_dir = argv[8];
        }
        else{
            cout << "Error in  Input1" << endl;
            return 0;
        }
    }

    //PART 2: CREATE NAMED PIPES FOR COMMUNICATION BETWEEN NUM MONITORS CHILD PROCESSES
    if (mkdir("./NP", 0755) == -1 && errno != EEXIST)
        perror("Failed to create directory: ");

    string FIFOARRAY[numMonitors+1];                        //array to hold fifo1 in index1, fifo2 in index2 etc.
    string stringfifo;
    for (int i = 1; i <= numMonitors; i++){

        stringfifo = "./NP/fifo" + to_string(i);
        FIFOARRAY[i] = stringfifo;
        const char * fifo = &stringfifo[0];

        if (mkfifo(fifo, 0666) == -1 && errno != EEXIST)
            perror("Failed to create fifo: ");
    }

    //PART 3: FORK NUM MONITORS CHILD PROCESSES AND EXEC MONITOR FOR EACH ONE WITH THE NAMED PIPES AS ARGUMENTS
    int PIDarray[numMonitors+1];                            //array to hold PID1 in index1, PID2 in index2 etc.
    int pid;
    int status;
    int i;
    for (i = 1; i <= numMonitors; i++){

        stringfifo = "./NP/fifo" + to_string(i);
        const char * fifo = &stringfifo[0];

        pid = fork();
        if(pid < 0){
            cout << "fork error" << endl;
            return 0;
        }
        else if(pid == 0){               //child process
            if(execl("./Monitor", "./Monitor", fifo, NULL) == -1){
                perror("execl failed");
            }
        }
        else{                           //parent process
            PIDarray[i] = pid;
        }
    }

    //PART 4: OPEN THE NAMED PIPES AND ASSIGN COUNTRY DIRECTORIES TO THE CHILD-MONITORS
    int fd;
    char buf[bufferSize];
    string COUNTRIESARRAY[numMonitors+1];                        //array to hold countries of fifo1 in index1, countries of fifo2 in index2 etc.

    for (i = 1; i <= numMonitors; i++){

        stringfifo = "./NP/fifo" + to_string(i);
        const char * fifo = &stringfifo[0];
        // Open FIFO for write only
        if((fd = open(fifo, O_WRONLY)) == -1){
            perror("error opening write fifo");
        }

        //Assign country directories to child-monitors
        DIR *d;
        struct dirent *dir;
        d = opendir("./input_dir");
        string countries = "";
        if (d) {
            int counter = 1;
            while ((dir = readdir(d)) != NULL) {
                string file(dir->d_name);

                if (file == ".." || file == "."){
                    continue;
                }
                if(counter==i){
                    
                    if (countries == ""){
                        countries = file;
                    }else{
                        countries = countries + " " + file;
                    }
                }
                if (++counter == numMonitors+1){
                    counter = 1;
                }
            }
            closedir(d);
        }
        COUNTRIESARRAY[i] = countries;
        char files[countries.length() + 1];
 
        strcpy(files, countries.c_str());

        write(fd, files, sizeof(buf));
        close(fd);
    }

    //PART 5: GET SERIALIZED BLOOM FILTERS FROM THE CHILD-MONITORS AND DESERIALIZE THEM
    for (i = 1; i <= numMonitors; i++){

        stringfifo = "./NP/fifo" + to_string(i);
        const char * fifo = &stringfifo[0];
        // Open FIFO for read only
        if((fd = open(fifo, O_RDONLY)) == -1){
            perror("error opening write fifo");
        }

        char virus[20];

        read(fd, virus, sizeof(virus));
        close(fd);
    }
    /*
    BFL *bfl = new BFL(sizeOfBloom);

    //while(wait(&status) > 0);

    for (i = 1; i <= numMonitors; i++){

        stringfifo = "./NP/fifo" + to_string(i);
        const char * fifo = &stringfifo[0];
        char bloom[1000];
        int i = 0;
        while(1){
            if((fd = open(fifo, O_RDONLY)) == -1){
                perror("error opening read fifo");
            }
            cout << i++ << endl;
            read(fd, bloom, sizeof(bloom));
            if(bloom[0] == -1){
                break;
            }
            bfl->deserialize(bloom);
            close(fd);
        }
    }
    bfl->print();
    */

   //PART 6: GET INPUT FROM USER AND PERFORM EACH COMMAND
    string input;
    string arg[10];                                 //Array of arguments given by the user with each input
    int k;
    while(true){
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

        if (arg[0] == "/exit"){                     //COMMAND = /exit
            for (int i = 1; i<=numMonitors; i++){
                cout << "Killing process " << PIDarray[i] << endl;
                kill(PIDarray[i], SIGKILL);
            }
            break;
        }
        else if(arg[0] == "/travelRequest"){                        //COMMAND = /travelRequest citizenID date countryFrom countryTo virusName
            //if(bfl->check(arg[1], arg[5])){                       //arg[1]: CitizenID, arg[2]: date, arg[3]: countryFrom, arg[4]: countryTo, arg[5]: virusName
            if(1){
                //FINDING WHICH MONITOR PROCESS WAS ASSIGNED countryFrom
                //cout << "MAYBE" << endl;
                stringfifo = "";
                for(int j = 1; j <= numMonitors; j++){
                    
                    string country[5];
                    int a = 0;
                    char *ctr = &COUNTRIESARRAY[i][0];                   //convert string to char * in order to use strtok
                    char *token = strtok(ctr," ");
                    while (token){
                        country[a] = token;
                        token = strtok(NULL," ");
                        a++;
                    }
                    for(int b = 0; b < 5; b++){
                        if(arg[3] == country[b]){
                            stringfifo = "./NP/fifo" + to_string(j);
                            break;
                        }
                    }
                    if(stringfifo!=""){
                        cout << "CALLING MONITOR PROCESS " << j << "TO CHECK!" << endl;
                        break;
                    }
                }
                const char * fifo = &stringfifo[0];
                string IDnVIRUS = arg[1] + " " + arg[5] + " ";
                char idnvirus[40];
                strcpy(idnvirus, IDnVIRUS.c_str());

                fd = open(fifo, O_WRONLY);
                write(fd, idnvirus, sizeof(idnvirus));
                close(fd);
                
                char flag[4];
                fd = open(fifo, O_RDONLY);
                read(fd, flag, sizeof(flag));
                close(fd);
        
                if(flag=="YES"){
                    cout << "REQUEST ACCEPTED – HAPPY TRAVELS" << endl;
                    //cout << "RREQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE" << endl;
                }else{
                    cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
                }   
            }else{
                cout << "REQUEST REJECTED – YOU ARE NOT VACCINATED" << endl;
            }
        }
        /*
        else if(arg[0] == "/travelRequest" && arg[2] != ""){        //COMMAND = /vaccineStatus CitizenID virusName
            vaccinatedSL->vaccineStatus(arg[1], arg[2]);            //arg[1]: CitizenID, arg[2]: virusName
        }

        else if(arg[0] == "/vaccineStatus"){                        //COMMAND = /vaccineStatus CitizenID
            vaccinatedSL->findAllRecords(arg[1]);                   //arg[1]: CitizenID
            nonvaccinatedSL->findAllRecords(arg[1]);
        }
        else if(arg[0] == "/list-nonVaccinated-Persons" && arg[1] != ""){   //COMMAND = /list-nonVaccinated-Persons virusName
            nonvaccinatedSL->printVirus(arg[1]);                            //arg[1]: virusName
        }
        */
        else{
            cout << "INVALID COMMAND, TRY AGAIN" << endl;
        }
    }

    return 0;
}
