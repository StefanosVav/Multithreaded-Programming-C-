#ifndef CITIZEN_H
#define CITIZEN_H
#include <string>
using namespace std;

extern bool ERROR;  //Global Variable ERROR, checks for record inconsistencies (used in the citizen constructor)

class citizen {
    string citizenID;
    string firstName;
    string lastName;
    string country;
    int age;
    string virusName;
    string vaccinated;
    string dateVaccinated;
public:
    citizen(string);
    ~citizen();
    string getCitizenID() const;
    string getFirstName() const;
    string getLastName() const;
    string getCountry() const;
    int getAge() const;
    string getVirusName() const;
    string getVaccinated() const;
    string getDateVaccinated() const;
};

class CitizenList { 
public: 
    ~CitizenList();
    citizen *cp; 
    CitizenList* next; 
    CitizenList* head = NULL;
    void insert(citizen*);
    void print();
}; 

bool consistent(citizen *, CitizenList *);

class CountriesList { 
public: 
    ~CountriesList();
    string country; 
    CountriesList* next; 
    CountriesList* head = NULL;
    void insert(string);
    void print();
}; 

#endif