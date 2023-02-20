#include <string>
#include <iostream>
#include <cstring>
#include "date.h"
using namespace std;

//Check if a string has the form of a date
bool isDate(string str){
    int nchar = 0, ndash = 0;
    for (char c : str){
        if(c == '-')
            ndash++;
        nchar++;
    }
    if(nchar > 7 && ndash == 2)             //if string is of type: XX-XX-XXXX, XX-X-XXXX, X-X-XXXX
        return true;
    return false;                           //else, it's not a date
}

//Convert a string into an array of 3 ints: day, month, year -- Used to compare dates
int* StringToDate(string date){
    int *dmy = new int[3];
    int i = 0;
    char *Date = &date[0];
    char *token = strtok(Date,"- ");
    while (token){
        dmy[i] = atoi(&token[0]);
        token = strtok(NULL,"- ");
        i++;
    }
    return dmy;
}

//Check if a date is within the range of date1 and date2
bool inRange(string date, string date1, string date2){
    if( !( isDate(date) && isDate(date1) && isDate(date2) )){
        cout << "error" << endl;
        return false;
    }
        
    int *d = new int[3];
    int *d1 = new int[3];
    int *d2 = new int[3];
    d = StringToDate(date);
    d1 = StringToDate(date1);
    d2 = StringToDate(date2);
    
    //Date here is represented as an array where d[0] : day, d[1] : month, d[2] : year. We compare:
    if (d[2] > d1[2] || ( d[2] == d1[2] && d[1] > d1[1] ) || ( d[2] == d1[2] && d[1] == d1[1] && d[0] > d1[0] ) ) {     //if date is later than date1
        if (d[2] < d2[2] || ( d[2] == d2[2] && d[1] < d2[1] ) || ( d[2] == d2[2] && d[1] == d2[1] && d[0] < d2[0] ) ){  //and earlier than date2
            delete[] d;
            delete[] d1;
            delete[] d2;
            return true;                                                                                                //then it is in range
        }                                                                                            
    }
    delete[] d;
    delete[] d1;
    delete[] d2;
    return false;                                                                                                       //else, it is not.
}