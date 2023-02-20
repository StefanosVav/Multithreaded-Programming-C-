#include <string>
#include <iostream>
#include <cstring>
#include "citizen.h"
using namespace std;
bool ERROR = false;

//CITIZEN CONSTRUCTOR/DESTRUCTOR

citizen::citizen(string record){
    string trait[9];
    int i;
    for (int j = 0; j<9; j++){
        trait[j] = "";
    }
    i = 0;
    char *Record = &record[0];              //convert string to char * in order to use strtok
    char *token = strtok(Record," ,");
    while (token){
        trait[i] = token;                   //each word(token) is a citizen trait
        token = strtok(NULL," ,");
        i++;
    }
    if(trait[8]!="" || trait[6]=="" || (trait[6]=="NO" && trait[7]!="") || (trait[6]=="YES" && trait[7]=="") ) {    //In case of inconsistencies in the record
        ERROR = true;                                                                                               //Set global variable ERROR as True
    }
    else{                                                   //Else, initialize the citizen object
        citizenID = trait[0];
        firstName = trait[1];
        lastName = trait[2];
        country = trait[3];
        age = atoi(&trait[4][0]);
        virusName = trait[5];
        vaccinated = trait[6];
        if(trait[7] != "")
            dateVaccinated = trait[7];
    }
}

citizen::~citizen(){
    //cout << "Record of Citizen with ID: " << citizenID << "  is now deleted." << endl;
}

//CITIZEN GET FUNCTIONS

string citizen::getCitizenID() const{
    return citizenID;
}
string citizen::getFirstName() const{
    return firstName;
}
string citizen::getLastName() const{
    return lastName;
}
string citizen::getCountry() const{
    return country;
}
int citizen::getAge() const{
    return age;
}
string citizen::getVirusName() const{
    return virusName;
}
string citizen::getVaccinated() const{
    return vaccinated;
}
string citizen::getDateVaccinated() const{
    return dateVaccinated;
}

//FUNCTIONS FOR MY CITIZEN LIST

CitizenList::~CitizenList(){
    CitizenList* temp;
    while(head!=NULL){
        temp = head;
        head = head->next;
        delete temp->cp;
        delete temp;
    }
}

void CitizenList::insert(citizen* c){
    CitizenList* node = new CitizenList();
    node->cp = c;
    node->next = head;
    head = node;
}

void CitizenList::print(){
    CitizenList* temp = head;
    while(temp!=NULL){
        cout << temp->cp->getCitizenID() << " ";
        cout << temp->cp->getFirstName() << " ";
        cout << temp->cp->getLastName() << " ";
        cout << temp->cp->getAge() << " ";
        cout << temp->cp->getCountry() << " ";
        cout << temp->cp->getVirusName() << endl;
        temp = temp->next;
    }
}

//FUNCTION THAT CHECKS RECORD CONSISTENCY

bool consistent(citizen *c, CitizenList *cList){
    if (cList->head == NULL) return true;
    CitizenList* temp = cList->head;
    while(temp!=NULL){
        if (temp->cp->getCitizenID() == c->getCitizenID()){
            if (temp->cp->getFirstName() == c->getFirstName() && temp->cp->getLastName() == c->getLastName() && temp->cp->getAge() == c->getAge() && temp->cp->getCountry() == c->getCountry() && temp->cp->getVirusName() == c->getVirusName()){
                //inconsistency in case of second record for the same person and the same virus.
                return false;
            }
            else if(temp->cp->getFirstName() != c->getFirstName() || temp->cp->getLastName() != c->getLastName() || temp->cp->getAge() != c->getAge() || temp->cp->getCountry() != c->getCountry()){
                //inconsistency in case of record of citizen with same ID and different trait(s).
                return false;
            }
        }
        temp = temp->next;
    }
    return true;
}

//FUNCTIONS FOR MY COUNTRIES LIST

CountriesList::~CountriesList(){
    CountriesList* temp;
    while(head!=NULL){
        temp = head;
        head = head->next;
        delete temp;
    }
}

void CountriesList::insert(string c){
    CountriesList* temp = head;
    while(temp!=NULL){
        if (temp->country == c)
            return;
        temp = temp->next;
    }
    CountriesList* node = new CountriesList();
    node->country = c;
    node->next = head;
    head = node;
}

void CountriesList::print(){
    CountriesList* temp = head;
    while(temp!=NULL){
        cout << temp->country << endl;
        temp = temp->next;
    }
}