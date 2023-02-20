#include <string>
#include <iostream>
#include <cmath>
#include "BloomFilter.h"
using namespace std;

//FUNCTIONS FOR BLOOM FILTER

BloomFilter::BloomFilter(string vName, int bloomSize){
    virusName = vName;
    K = 16;
    M = bloomSize;
    array = new bool[M]{0};
}

BloomFilter::~BloomFilter(){
    //cout << "BloomFilter " << virusName << " is now deleted!" << endl;
    delete[] array;
}

//Inserts ID in Bloom Filter by setting K bits to 1 (hash_i % M will return the index of the bit that is to be set to 1)
void BloomFilter::insert(string ID){
    char * str = &ID[0];
    int index;
    
    for(int i = 0; i < K; i++){
        index = hash_i(str, i) % M;
        array[index] = 1;
    }
}

//Checks if ID is in BloomFilter by examining whether the bit indexes returned by hash_i % M have been set to 1
bool BloomFilter::check(string ID){
    char * str = &ID[0];
    int index;
    for(int i = 0; i < K; i++){
        index = hash_i(str, i) % M;
        if(array[index] != 1){
            return 0;
        }
    }
    return 1;
}

string BloomFilter::getVirusName(){
    return virusName;
}

char * BloomFilter::serialize(){

    char* bloom = new char[512];
    int i = 0;
    while(i < virusName.length()){
        bloom[i] = virusName[i];
        i++;
    }
    bloom[i] = ' ';
    i++;
    // Convert binary array to decimal int
    unsigned long int decimal = 0;
    for(int j=0; j<100; j++){
        cout << pow(2,j) << endl;
        if(array[j])
            decimal += + pow(2,j);
        //cout << decimal << endl;
    }
    string dec = to_string(decimal);
    cout << dec << endl;

    for(int j = 0; j < dec.length(); j++, i++){
        bloom[i] = dec[j];
    }
    
    return bloom;
}

void BloomFilter::deserialize(char * bloom){
    int i = 0;
    while(bloom[i] != ' '){
        i++;
    }
    i++;
    int j = 0;
    while(j < M){
        array[j] = bloom[i];
        i++; j++;
    }
}


//FUNCTIONS FOR BLOOM FILTERS LIST

BFL::BFL(int bloomSize){
    bSize = bloomSize*8;                //bSize in bits: bits-> 8*bytes
}

BFL::~BFL(){
    BFL* temp;
    while(head!=NULL){
        temp = head;
        head = head->next;
        delete temp->bf;
        delete temp;
    }
    //cout << "Bloom Filters List deleted" << endl;
}

//Finds the Bloom Filter with the virusName of the given citizen record and inserts
void BFL::insert(citizen* c){
    if(c->getVaccinated() == "NO") return;
    BFL* temp = head;
    while(temp!=NULL){
        if (temp->bf->getVirusName() == c->getVirusName())
        {
            temp->bf->insert(c->getCitizenID());
            return;
        }
        temp = temp->next;
    }
    BFL* node = new BFL(bSize);
    node->bf = new BloomFilter(c->getVirusName(), bSize);
    node->bf->insert(c->getCitizenID());
    node->next = head;
    head = node;
}

void BFL::print(){
    BFL* temp = head;
    while(temp!=NULL){
        cout << temp->bf->getVirusName() << endl;
        temp = temp->next;
    }
}

bool BFL::check(string ID, string vName){
    BFL* temp = head;
    while(temp!=NULL){
        if (temp->bf->getVirusName() == vName){
            return temp->bf->check(ID);
        }
        temp = temp->next;
    }
    return 0;
}

char * BFL::serialize(int i){

    BFL* temp = head;
    int count = 0;
    while(temp!=NULL && count < i){
        temp = temp->next;
        count++;
    }
    //Creating array to return -1 so that we know there are no more bloomFilters in the list
    if (temp==NULL){
        char* array = new char[1];
        array[0] = -1;
        return array; 
    }

    return temp->bf->serialize();
}

void BFL::deserialize(char * bloom){
    BFL* node = new BFL(bSize);

    string virus;
    int i = 0;
    while(bloom[i] != ' '){
        virus[i] = bloom[i];
        i++;
    }
    node->bf = new BloomFilter(virus, bSize);
    node->bf->deserialize(bloom);
    node->next = head;
    head = node;
}

string BFL::getViruses(){
    string viruses = "";
    BFL* temp = head;
    while(temp!=NULL){
        viruses.append(temp->bf->getVirusName());
        viruses.append(" ");
        temp = temp->next;
    }
    return viruses;
}


//HASH FUNCTIONS

unsigned long djb2(char *str) {
	int c;
	unsigned long hash = 5381;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;        // hash * 33 + c

	return hash;
}

unsigned long sdbm(char *str) {
	int c;
	unsigned long hash = 0;

	while ((c = *str++))
		hash = c + (hash << 6) + (hash << 16) - hash;

	return hash;
}

// Return the result of the ith hash function. This function uses djb2 and sdbm.
unsigned long hash_i(char *str, unsigned int i) {
	return djb2(str) + i * sdbm(str) + i * i;
}