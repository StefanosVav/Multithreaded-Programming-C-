#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <string>
#include "citizen.h"
using namespace std;

class SkipNode{                     //SkipNode, uses key: ID, value: pointer to citizen object
public:
    SkipNode(int, int, citizen*);
    ~SkipNode();
    int ID;
    citizen *Citizen;
    SkipNode **forward;             //Array to hold pointers to node of different level
};

class SkipList{
    string virusName;
    int maxLevel;                   //max level that the skip list could reach 
    int level;                      //highest level where a node exists in this SkipList
    SkipNode *head;
public:
    SkipList(int, string);
    ~SkipList();
    string getVirusName();
    int flipCoin();
    void insert(citizen *);
    void remove(int);
    citizen* check(int);
    void print();                                   //Print entire skip list
    void printBottom();                             //Print only the bottom level
    int noIDinRange(string, string, string);        //Number of citizens from country in range date1, date2
    int *noIDinRangeByAge(string, string, string);  //Number of citizens from country in range date1, date2 by age
};

class SLL{                          //Skip Lists list
public: 
    ~SLL();
    SkipList *sl; 
    SLL* next; 
    SLL* head = NULL;
    void insert(citizen*);
    void remove(citizen*);
    citizen* check(citizen*);
    void print();
    void printVirus(string);
    int vaccineStatus(string, string);             //For /vaccineStatus + 2 arguments
    void findAllRecords(string);                    //For /vaccineStatus + 1 argument
    int VaccinatedinRange(string, string, string, string);          //country, virus, date1, date2
    int *VaccinatedinRangeByAge(string, string, string, string);
};

#endif