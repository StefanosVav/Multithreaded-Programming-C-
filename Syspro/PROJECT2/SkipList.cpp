#include <string>
#include <iostream>
#include "SkipList.h"
#include "citizen.h"
#include "date.h"
using namespace std;

//SKIP NODE CONSTRUCTOR

SkipNode::SkipNode(int citizenID, int level, citizen *c)
{
    ID = citizenID;
    Citizen = c;

    forward = new SkipNode*[level+1]{0};    //Initialize and fill forward array with 0
}

SkipNode::~SkipNode(){
    delete[] forward;
}

//SKIP LIST CONSTRUCTOR / DESTRUCTOR

SkipList::SkipList(int maxLVL, string vName)
{
    maxLevel = maxLVL;
    level = 0;
    virusName = vName;

    head = new SkipNode(-1, maxLevel, NULL);    //Initialize head node and with ID(key) -1 and NULL citizen*
}

SkipList::~SkipList(){
    SkipNode *curr = head;
    SkipNode *temp;
    while(curr != NULL){
        temp = curr;
        curr = curr->forward[0];
        delete temp;
    }
    //cout << "Skip List deleted" << endl;
}

//SKIP LIST FUNCTIONS

string SkipList::getVirusName(){
    return virusName;
}

//Flips a coin 10 or less times in order to find a random level for the node to-be-added
int SkipList::flipCoin()
{
    float r = (float)rand()/RAND_MAX;
    int lvl = 0;
    while (r < 0.5 && lvl < maxLevel){                     //50-50 chance of incrementing the node's level
        lvl++;
        r = (float)rand()/RAND_MAX;
    }
    return lvl;
}

void SkipList::insert(citizen *c)
{
    SkipNode *curr = head;
    //update[i] holds the pointer to node at level i from which we moved down to level i-1. At level 0, it holds a pointer to the previous node of insertion position
    SkipNode *update[maxLevel+1]{0};

    /*  Starting from the highest level of the Skip List: Move the current pointer forward while key(ID) is greater than the key of the curr->next node
        If key < curr->next->key, insert current in update and move to the next level   */
    for(int i = level; i >= 0; i--){
        while (curr->forward[i] != NULL && curr->forward[i]->ID < atoi(&c->getCitizenID()[0]))
            curr = curr->forward[i];
        update[i] = curr;
    }
    curr = curr->forward[0];
  
    /*  If curr is NULL, we have reached the end of the level or curr's key(ID) is not equal to the key to be inserted,
        the key should be inserted between update[0] and curr   */
    if(curr == NULL || curr->ID != atoi(&c->getCitizenID()[0]))
    {
        int rlevel = flipCoin();
  
        /*  If random level is greater than list's current level (node with highest level inserted in the list so far)
            initialize update value with pointer to head for further use  */
        if(rlevel > level){
            for(int i = level + 1; i < rlevel + 1; i++)
                update[i] = head;

            // Update the list curr level
            level = rlevel;
        }

        //Create new node with random level generated
        SkipNode* n = new SkipNode(atoi(&c->getCitizenID()[0]), rlevel, c);
  
        //Insert node by rearranging pointers 
        for(int i=0; i <= rlevel; i++)
        {
            n->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = n;
        }
    }
}

void SkipList::remove(int citizenID)
{
    SkipNode *curr = head;
    SkipNode *update[maxLevel + 1]{0};

    for(int i = level; i >= 0; i--)
    {
        while(curr->forward[i] != NULL && curr->forward[i]->ID < citizenID)
            curr = curr->forward[i];
        update[i] = curr;
    }
    curr = curr->forward[0];

    //If the current node is target node
    if(curr != NULL)
    {
        if(curr->ID == citizenID){
            //Starting from the lowest level, rearrange pointers to remove target node
            for(int i=0; i <= level; i++)
            {
                //If at some level i, the next node is not the target node, stop 
                if(update[i]->forward[i] != curr)
                    break;

                update[i]->forward[i] = curr->forward[i];
            }
            delete curr;

            //Remove levels with no elements 
            while(level > 0 && head->forward[level] == 0)
                level--;
            
            
            cout << "Successfully deleted key "<< citizenID <<"\n";
        }
    }
}

//Returns pointer to citizen if the ID(key) is found, else returns NULL
citizen* SkipList::check(int citizenID)
{
    SkipNode *curr = head;

    for(int i = level; i>=0; i--)
    {
        while(curr->forward[i] && curr->forward[i]->ID < citizenID)
            curr = curr->forward[i];
    }
    curr = curr->forward[0];

    if(curr != NULL){
        if(curr->ID == citizenID)
            return curr->Citizen;
    }
    return NULL;
}

//Print every level of the Skip List (only ID)
void SkipList::print()
{
    for (int i=0; i <= level; i++)
    {
        SkipNode *node = head->forward[i];
        cout << "Level " << i << ": ";
        while (node != NULL)
        {
            cout << node->ID << " ";
            node = node->forward[i];
        }
        cout << endl;
    }
    cout << endl;
}

//Print only the bottom level of the SkipList   (ID, fname, lname, country, age)
void SkipList::printBottom()
{
    SkipNode *node = head->forward[0];
    while (node != NULL)
    {
        cout << node->Citizen->getCitizenID() << " " << node->Citizen->getFirstName() << " " << node->Citizen->getLastName() << " " << node->Citizen->getCountry() << " " << node->Citizen->getAge() << endl;
        node = node->forward[0];
    }
    cout << endl;
}

//Returns the number of citizens from given country in the SkipList -->
int SkipList::noIDinRange(string country, string date1, string date2){
    int counter = 0;
    SkipNode *node = head->forward[0];

    // --> Within a given date range
    if(date1 != "" && date2 != ""){
        while (node != NULL)
        {
            if(node->Citizen->getCountry() == country && inRange(node->Citizen->getDateVaccinated(), date1, date2)){
                counter++;
            }
            node = node->forward[0];
        }
    }
    // --> Or overall
    else if(date1 == "" && date2 == ""){
        while (node != NULL)
        {
            if(node->Citizen->getCountry() == country){
                counter++;
            }
            node = node->forward[0];
        }
    }
    return counter;
}

//Returns array of ints holding the number of citizens from given country in the SkipList by age -->
int* SkipList::noIDinRangeByAge(string country, string date1, string date2){
    int *ageGroups = new int[4]{0};
    SkipNode *node = head->forward[0];

    // --> Within a given date range
    if(date1 != "" && date2 != ""){
        while (node != NULL)
        {
            if(node->Citizen->getCountry() == country && inRange(node->Citizen->getDateVaccinated(), date1, date2)){
                if(node->Citizen->getAge() < 20)
                    ageGroups[0]++;
                else if(node->Citizen->getAge() < 40)
                    ageGroups[1]++;
                else if(node->Citizen->getAge() < 60)
                    ageGroups[2]++;
                else
                    ageGroups[3]++;
            }
            node = node->forward[0];
        }
    }
    // --> Or overall
    else if(date1 == "" && date2 == ""){
        while (node != NULL)
        {
            if(node->Citizen->getCountry() == country){
                if(node->Citizen->getAge() < 20)
                    ageGroups[0]++;
                else if(node->Citizen->getAge() < 40)
                    ageGroups[1]++;
                else if(node->Citizen->getAge() < 60)
                    ageGroups[2]++;
                else
                    ageGroups[3]++;
            }
            node = node->forward[0];
        }
    }
    return ageGroups;
}

//SKIP LISTS LIST FUNCTIONS

SLL::~SLL(){
    SLL* temp;
    while(head!=NULL){
        temp = head;
        head = head->next;
        delete temp->sl;
        delete temp;
    }
    //cout << "SLL deleted" << endl;
}

//Finds Skip List with virusName = citizen's virusName and inserts, or creates new SkipList and inserts citizen in case the SkipList with virusName is not found
void SLL::insert(citizen* c){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == c->getVirusName())
        {
            temp->sl->insert(c);
            return;
        }
        temp = temp->next;
    }
    SLL* node = new SLL();
    node->sl = new SkipList(10, c->getVirusName());
    node->sl->insert(c);
    node->next = head;
    head = node;
}

//Finds Skip List with virusName = citizen's virusName and removes citizen with CitizenID
void SLL::remove(citizen* c){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == c->getVirusName())
        {
            temp->sl->remove( atoi(&c->getCitizenID()[0]) );
            return;
        }
        temp = temp->next;
    }
}

//Finds Skip List with virusName = citizen's virusName and checks if citizen with CitizenID is in it
citizen* SLL::check(citizen* c){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == c->getVirusName())
        {
            return temp->sl->check( atoi(&c->getCitizenID()[0]) );
        }
        temp = temp->next;
    }
    return NULL;
}

//Print all SkipLists by virus
void SLL::print(){
    SLL* temp = head;
    while(temp!=NULL){
        cout << "--> " << temp->sl->getVirusName() << endl;
        //OPTIONAL PRINT TO SEE EACH SKIPLIST PRINTED LEVEL BY LEVEL, REMOVE COMMENT TO CHECK
        //temp->sl->print();
        temp = temp->next;
    }
}

//Print Skip List of given Virus
void SLL::printVirus(string vName){
    SLL* temp = head;
    while(temp!=NULL){
        if(temp->sl->getVirusName() == vName){
            cout << "--> " << "Non-Vaccinated for " << vName << ":" << endl;
            temp->sl->printBottom();
        }

        temp = temp->next;
    }
}

//Checks if a citizen with ID has been vaccinated for virus vName. If we have no records of the citizen, prints NOT VACCINATED.
int SLL::vaccineStatus(string ID, string vName){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == vName)
        {
            citizen *c = temp->sl->check(atoi(&ID[0]));
            if(c!=NULL){
                if(c->getVaccinated() == "YES"){
                    return 1;
                }
                else{
                    return 0;
                }
            }
        }
        temp = temp->next;
    }
    return 0;
}

//Prints all appearances of a citizen with given ID in our SkipLists
void SLL::findAllRecords(string ID){
    SLL* temp = head;
    while(temp!=NULL){
        citizen *c = temp->sl->check(atoi(&ID[0]));
        if(c!=NULL){
            if(c->getVaccinated() == "YES")
                cout << temp->sl->getVirusName() << " YES " << c->getDateVaccinated() << endl;
            else
                cout << temp->sl->getVirusName() << " NO" << endl;
        }
        temp = temp->next;
    }
    return;
}

//Finds Skip List with virusName = vName and returns noIDinRange function for that SkipList
int SLL::VaccinatedinRange(string country, string vName, string date1, string date2){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == vName){
            return temp->sl->noIDinRange(country, date1, date2);
        }
        temp = temp->next;
    }
    return 0;
}

//Finds Skip List with virusName = vName and returns noIDinRangeByAge function for that SkipList
int* SLL::VaccinatedinRangeByAge(string country, string vName, string date1, string date2){
    SLL* temp = head;
    while(temp!=NULL){
        if (temp->sl->getVirusName() == vName){
            return temp->sl->noIDinRangeByAge(country, date1, date2);
        }
        temp = temp->next;
    }
    return 0;
}
