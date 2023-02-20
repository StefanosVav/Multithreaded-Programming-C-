#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include "date.h"
#include "citizen.h"
#include "BloomFilter.h"
#include "SkipList.h"
using namespace std;

int main(int argc, char *argv[]) {

    //PART 1: GET INPUT FROM COMMAND LINE (INPUT FILE NAME, BLOOM SIZE) AND SAVE IT
    string InputFile, BloomSize;
    int bloomSize = 0;
    if(argc != 5){
        cout << "Error in  Input" << endl;
        return 0;
    }
    else{                               //Get the input file and the bloom size in KILOBYTES from the command line input
        string arg1 = argv[1], arg3 = argv[3];

        if(arg1 == "-c" && arg3 == "-b"){
            InputFile = argv[2];
            BloomSize = argv[4];
        }
        else if(arg1 == "-b" && arg3 == "-c"){
            InputFile = argv[4];
            BloomSize = argv[2];
        }
        else{
            cout << "Error in  Input" << endl;
            return 0;
        }
    }
    bloomSize = 1000 * atoi(&BloomSize[0]);    //Remaking Bloom Size into an int of bytes. 1 KByte is 1000 Bytes

    //PART 2: PARSE THE INPUT FILE - SAVE THE RECORDS' DATA IN OUR OWN DATA STRUCTURES
    string record;
    ifstream CRF(InputFile + ".txt");

    CountriesList *Countries = new CountriesList();     //Create a Countries List
    CitizenList *cList = new CitizenList();     //Create a Citizen List that holds the records from the input file
    BFL *bfl = new BFL(bloomSize);              //Create a Bloom Filters List that holds a (pointer to a) Bloom Filter object for each virus
    SLL *vaccinatedSL = new SLL();              //Create a vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus
    SLL *nonvaccinatedSL = new SLL();           //Create a non vaccinated Skip Lists list that holds a (pointer to a) SkipList object for each virus

    while(getline(CRF, record)){
        ERROR = false;                          //Global Variable ERROR, checks for record inconsistencies (used in the citizen constructor - citizen.cpp)

        citizen *c = new citizen(record);       //Create citizen object initialized with the characteristics of the current line(record) in the input file
        if (ERROR || !consistent(c, cList)){
            cout << "--Error in record: " << record << endl;
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
        Countries->insert(c->getCountry());     //Add the country (if not already added) to our countries list
    }
    //Optional Print to see that our data structures are initialized correctly
    cout << endl;
    //cout << endl << "----------------RECORDS-----------------" << endl;
    //cList->print();
    cout << endl << "---------------COUNTRIES----------------" << endl;
    Countries->print();
    cout << endl << "-------------BLOOM FILTERS--------------" << endl;
    bfl->print();
    cout << endl << "---------VACCINATED SKIP LISTS----------" << endl;
    vaccinatedSL->print();
    cout << endl << "-------NON VACCINATED SKIP LISTS--------" << endl;
    nonvaccinatedSL->print();
    cout << endl;

    CRF.close();
    
    //PART 3: GET INPUT FROM USER AND PERFORM EACH COMMAND
    string input;
    string arg[10];                                 //Array of arguments given by the user with each input
    int i;
    while(true){
        cout << "ENTER COMMAND:" << endl;
        for (int j = 0; j<10; j++){
            arg[j] = "";                            //Set all arguments in the array as an empty string
        }
        i = 0;
        getline(cin, input);
        char *inputC = &input[0];                   //convert string to char * in order to use strtok
        char *token = strtok(inputC," ,");
        while (token){
            arg[i] = token;                         //Set arguments as each word of the user's input (using strtok)
            token = strtok(NULL," ,");
            i++;
        }

        if (arg[0] == "/exit"){                     //COMMAND = /exit
            break;
        }

        else if(arg[0] == "/vaccineStatusBloom"){                   //COMMAND = /vaccineStatusBloom CitizenID virusName
            if(bfl->check(arg[1], arg[2])){                         //arg[1]: CitizenID, arg[2]: virusName
                cout << "MAYBE" << endl;
            }else{
                cout << "NOT VACCINATED" << endl;
            }
        }

        else if(arg[0] == "/vaccineStatus" && arg[2] != ""){        //COMMAND = /vaccineStatus CitizenID virusName
            vaccinatedSL->vaccineStatus(arg[1], arg[2]);            //arg[1]: CitizenID, arg[2]: virusName
        }

        else if(arg[0] == "/vaccineStatus"){                        //COMMAND = /vaccineStatus CitizenID
            vaccinatedSL->findAllRecords(arg[1]);                   //arg[1]: CitizenID
            nonvaccinatedSL->findAllRecords(arg[1]);
        }

        else if(arg[0] == "/populationStatus"){                                                         //COMMAND = /populationStatus [country] virusName date1 date2
            if( arg[4] != "" && isDate(arg[3]) && isDate(arg[4]) ){                                     //CASE 1: country is defined (arg[1]: country, arg[2]: virusName)
                int NO_overall = nonvaccinatedSL->VaccinatedinRange(arg[1], arg[2], "", "");                //no arguments for date1, date2 in order to find the overall
                int YES_overall = vaccinatedSL->VaccinatedinRange(arg[1], arg[2], "", "");
                int YES_in_range = vaccinatedSL->VaccinatedinRange(arg[1], arg[2], arg[3], arg[4]);         //arg[3], arg[4] are date1 and date2 respectively
                cout << NO_overall << " " << YES_overall << endl;
                cout << arg[1] << " " << YES_in_range << " " << float(YES_in_range) / ( float(YES_overall) + float(NO_overall) ) << "%" << endl;
            }
            else if( arg[4] == "" && isDate(arg[2]) && isDate(arg[3]) ){                                    //CASE 2: country is not defined (arg[1]: virusName)
                CountriesList *temp = Countries->head;                                                          //For each country in our countries list
                while(temp!=NULL){
                    int NO_overall = nonvaccinatedSL->VaccinatedinRange(temp->country, arg[1], "", "");         //no arguments for date1, date2 in order to find the overall
                    int YES_overall = vaccinatedSL->VaccinatedinRange(temp->country, arg[1], "", "");
                    int YES_in_range = vaccinatedSL->VaccinatedinRange(temp->country, arg[1], arg[2], arg[3]);  //arg[2], arg[3] are date1 and date2 respectively
                    cout << temp->country << " " << YES_in_range << " " << float(YES_in_range) / ( float(YES_overall) + float(NO_overall) ) << "%" << endl;

                    temp = temp->next;
                }
            }
            else{
                cout << "INVALID COMMAND, TRY AGAIN" << endl;
            }
        }

        else if(arg[0] == "/popStatusByAge"){                                           //COMMAND = /popStatusByAge [country] virusName date1 date2
            int *NO_overall = new int[4];                                               //arrays of 4 age groups
            int *YES_overall = new int[4]; 
            int *YES_in_range = new int[4]; 
            if( arg[4] != "" && isDate(arg[3]) && isDate(arg[4]) ){                                     //CASE 1: country is defined (arg[1] is country, arg[2] is virus name)
                NO_overall = nonvaccinatedSL->VaccinatedinRangeByAge(arg[1], arg[2], "", "");               //no arguments for date1, date2 in order to find the overall
                YES_overall = vaccinatedSL->VaccinatedinRangeByAge(arg[1], arg[2], "", "");
                YES_in_range = vaccinatedSL->VaccinatedinRangeByAge(arg[1], arg[2], arg[3], arg[4]);        //arg[3], arg[4] are date1 and date2 respectively

                cout << arg[1] << ":" << endl;
                cout << "0-20 " << YES_in_range[0] << " " << float(YES_in_range[0]) / ( float(YES_overall[0]) + float(NO_overall[0]) ) << "%" << endl;
                cout << "21-40 " << YES_in_range[1] << " " << float(YES_in_range[1]) / ( float(YES_overall[1]) + float(NO_overall[1]) ) << "%" << endl;
                cout << "41-60 " << YES_in_range[2] << " " << float(YES_in_range[2]) / ( float(YES_overall[2]) + float(NO_overall[2]) ) << "%" << endl;
                cout << "60+ " << YES_in_range[3] << " " << float(YES_in_range[3]) / ( float(YES_overall[3]) + float(NO_overall[3]) ) << "%" << endl;
            }
            else if( arg[4] == "" && isDate(arg[2]) && isDate(arg[3]) ){                                    //CASE 2: country is not defined (arg[1] is virus name)
                CountriesList *temp = Countries->head;                                                          //For each country in our countries list
                while(temp!=NULL){
                    NO_overall = nonvaccinatedSL->VaccinatedinRangeByAge(temp->country, arg[1], "", "");        //no arguments for date1, date2 in order to find the overall
                    YES_overall = vaccinatedSL->VaccinatedinRangeByAge(temp->country, arg[1], "", "");
                    YES_in_range = vaccinatedSL->VaccinatedinRangeByAge(temp->country, arg[1], arg[2], arg[3]); //arg[2], arg[3] are date1 and date2 respectively

                    cout << temp->country << ":" << endl;
                    cout << "0-20 " << YES_in_range[0] << " " << float(YES_in_range[0]) / ( float(YES_overall[0]) + float(NO_overall[0]) ) << "%" << endl;
                    cout << "21-40 " << YES_in_range[1] << " " << float(YES_in_range[1]) / ( float(YES_overall[1]) + float(NO_overall[1]) ) << "%" << endl;
                    cout << "41-60 " << YES_in_range[2] << " " << float(YES_in_range[2]) / ( float(YES_overall[2]) + float(NO_overall[2]) ) << "%" << endl;
                    cout << "60+ " << YES_in_range[3] << " " << float(YES_in_range[3]) / ( float(YES_overall[3]) + float(NO_overall[3]) ) << "%" << endl;

                    temp = temp->next;
                }
            }
            delete NO_overall;
            delete YES_overall;
            delete YES_in_range;
        }

        else if(arg[0] == "/insertCitizenRecord" && arg[7] != ""){                      //COMMAND = /insertCitizenRecord
            ERROR = false;
            string record;                                                              //create string record holding the user's input                         
            if(arg[8]!= "")
                record = arg[1] + " " + arg[2] + " " + arg[3] + " " + arg[4] + " " + arg[5] + " " + arg[6] + " " + arg[7] + " " + arg[8];
            else
                record = arg[1] + " " + arg[2] + " " + arg[3] + " " + arg[4] + " " + arg[5] + " " + arg[6] + " " + arg[7];

            //Proceed to insert the record similarly to PART 2.
            citizen *c = new citizen(record);       //Create citizen object initialized with the characteristics of the record
            if (ERROR || !consistent(c, cList)){
                cout << "--Error in record: " << record << endl;
                delete c;                           //Free space
                continue;
            }
            
            cList->insert(c);                       //If the new record is consistent, add it to our Citizen List,
            if(arg[7] == "YES"){                    //our vaccinated or non vaccinated skip list
                vaccinatedSL->insert(c);
                bfl->insert(c);                     //and our Bloom Filters list
            }else{
                nonvaccinatedSL->insert(c);
            }
            Countries->insert(c->getCountry());     //Add the country (if not already added) to our countries list
            cout << "Succesfully inserted citizen " << arg[1] << endl;
        }

        else if(arg[0] == "/vaccinateNow" && arg[6] != ""){                     //COMMAND = /insertCitizenRecord
            time_t t = time(0);                                                 //Find todaysDate using ctime library
            struct tm * timeStruct = localtime(&t);
            string todaysDate = to_string(timeStruct->tm_mday) + "-" + to_string(timeStruct->tm_mon + 1) + "-" + to_string(timeStruct->tm_year + 1900);

            ERROR = false;
            //create string record holding the user's input, Vaccinated = YES, dateVaccinated = todaysDate
            string record = arg[1] + " " + arg[2] + " " + arg[3] + " " + arg[4] + " " + arg[5] + " " + arg[6] + " YES " + todaysDate;

            citizen *c = new citizen(record);       //Create citizen object initialized with the characteristics of the record
            if (ERROR){
                cout << "--Error in record: " << record << endl;
                delete c;                           //Free space
                continue;
            }

            citizen *ctz = vaccinatedSL->check(c);   
            citizen *ctzn = nonvaccinatedSL->check(c);   
            if(ctz != NULL){                            //CASE 1: Citizen is already vaccinated
                cout << "ERROR: CITIZEN " << ctz->getCitizenID() << " ALREADY VACCINATED ON " << ctz->getDateVaccinated() << endl; 
            }
            else if(ctz == NULL && ctzn != NULL){       //CASE 2: Citizen is non-vaccinated, but exists on our non vaccinated list
                nonvaccinatedSL->remove(c);                 //remove from the non-vaccinated skiplist

                bfl->insert(c);                             //Add to our Bloom Filters List
                vaccinatedSL->insert(c);                    //and our vaccinated skip list
                cout << "Succesfully vaccinated citizen " << arg[1] << endl;
            }
            else if(ctz == NULL && ctzn == NULL){        //CASE 3: Citizen is not in our database
                cList->insert(c); 
                bfl->insert(c);                             //Add to our Bloom Filters List
                vaccinatedSL->insert(c);                    //and our vaccinated skip list
                cout << "Succesfully vaccinated citizen " << arg[1] << endl;
                Countries->insert(c->getCountry());         //Add the country (if not already added) to our countries list
            }
        }
        else if(arg[0] == "/list-nonVaccinated-Persons" && arg[1] != ""){   //COMMAND = /list-nonVaccinated-Persons virusName
            nonvaccinatedSL->printVirus(arg[1]);                            //arg[1]: virusName
        }

        else{
            cout << "INVALID COMMAND, TRY AGAIN" << endl;
        }
    }

    delete Countries;
    delete cList;
    delete bfl;
    delete vaccinatedSL;
    delete nonvaccinatedSL;
    
    return 0;
}