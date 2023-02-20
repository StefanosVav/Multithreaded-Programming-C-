#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <string>
#include "citizen.h"
using namespace std;

class BloomFilter{
    int M;
    int K;
    bool * array;
    string virusName;
public:
    BloomFilter(string, int);
    ~BloomFilter();
    void insert(string);
    bool check(string);
    string getVirusName();
};

class BFL {             //Bloom Filters List
    int bSize;
public: 
    BFL(int);
    ~BFL();
    BloomFilter *bf; 
    BFL* next;
    BFL* head = NULL;
    void insert(citizen*);
    void print();
    bool check(string, string);
}; 

unsigned long djb2( char *);
unsigned long sdbm( char *);
unsigned long hash_i( char *, unsigned int);

#endif
