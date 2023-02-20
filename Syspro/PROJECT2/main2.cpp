#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
//#include <ctime>
//#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include "citizen.h"
#include "BloomFilter.h"
#include "SkipList.h"
using namespace std;
extern int errno;

int main(int argc, char *argv[]) {

    const char * fifo;
    if(argc != 2){
        cout << "Error in  Input" << endl;
        return 0;
    }
    else{                               //Get data from command line input
        fifo = argv[1];
    }

    int fd1;

    char countries[40];

    fd1 = open(fifo, O_RDONLY);
    read(fd1, countries, 40);
    cout << "Child Process with ID: " << getpid() << " was assigned: " << countries << endl;

    close(fd1);

    int bloomSize = 100;
    CitizenList *cList = new CitizenList();     //Create a Citizen List that holds the records from the input file
    BFL *bfl = new BFL(bloomSize);              //Create a Bloom Filters List that holds a (pointer to a) Bloom Filter object for each virus
    SLL *vaccinatedSL = new SLL();              //Create a vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus
    SLL *nonvaccinatedSL = new SLL();           //Create a non vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus

    string cArray[5];                   //countries array for this monitor
    int numC = 0;                       //number of countries for this monitor
    char *token = strtok(countries," ");
    while (token){
        cArray[numC] = token;
        token = strtok(NULL," ");
        numC++;
    }

    for(int i = 0; i < numC; i++){

        DIR *d;
        struct dirent *dir;
        string directory = "./input_dir/" + cArray[i];
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
    /*
    //Optional Print to see that our data structures are initialized correctly
    cout << endl;
    //cout << endl << "----------------RECORDS-----------------" << endl;
    //cList->print();
    cout << endl << "-------------BLOOM FILTERS--------------" << endl;
    bfl->print();
    cout << endl << "---------VACCINATED SKIP LISTS----------" << endl;
    vaccinatedSL->print();
    cout << endl << "-------NON VACCINATED SKIP LISTS--------" << endl;
    nonvaccinatedSL->print();
    cout << endl;   */

    //SERIALIZING AND SENDING BLOOM FILTERS TO PARENT PROCESS
    fd1 = open(fifo, O_WRONLY);
    char virus[20];
    strcpy(virus, bfl->head->bf->getVirusName().c_str());
    write(fd1, virus, sizeof(virus));
    close(fd1);

    /*int i = 0;
    while(1){
        char * bloom = bfl->serialize(i);
        fd1 = open(fifo, O_WRONLY);
        if(bloom[0] == -1){
            cout << "stop writing" << endl;
            write(fd1, bloom, sizeof(bloom));
            delete[] bloom;
            break;
        }
        write(fd1, bloom, sizeof(bloom));
        delete[] bloom;
        i++; 
        close(fd1);
    }
    cout << "Success1!" << endl;*/

    //GET TRAVEL REQUEST FROM PARENT
    char IDnVIRUS[40];
    int status;
    wait(&status);
    if((fd1 = open(fifo, O_RDONLY)) == -1){
        perror("failed to open read fifo");
    }
    read(fd1, IDnVIRUS, sizeof(IDnVIRUS));
    close(fd1);
    string InV = string(IDnVIRUS);
    string ID = "", VIRUS = "";
    string word = "";
    int i = 0;
    for (char x : InV) 
    {
        if (x == ' ' && i == 0)
        {
            ID = word;
            word = "";
            i++;
        }else if(x == ' ' && i == 1){
            VIRUS = word;
            break;
        }
        else {
            word = word + x;
        }
    }

    //CHECK SKIPLISTS AND RETURN YES IF CITIZEN WITH GIVEN ID IS VACCINATED, AND NO IF NOT
    fd1 = open(fifo, O_WRONLY);
    char flag[4];
    if(vaccinatedSL->vaccineStatus(ID, VIRUS)){
        flag[0] = 'Y'; flag[1] = 'E'; flag[2] = 'S';
    }else{
        flag[0] = 'N'; flag[1] = 'O';
    }
    write(fd1,flag,sizeof(flag));
    close(fd1);

    delete cList;
    delete bfl;
    delete vaccinatedSL;
    delete nonvaccinatedSL;

    return 0;
}
