#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include <map>
#include <iterator>
#include<string.h>
#include<pthread.h>
#include<unistd.h> //for read and write functions
#include<arpa/inet.h> //for inet_ntop() function
using namespace std;
map <char*, pair<int,int> > gquiz1;

 
int main()
{
    map <int, int> gquiz1;        // empty map container
 
    // insert elements in random order
    gquiz1.insert( pair <int, pair <int,int> > (1, (50,40));
    gquiz1.insert(pair <int, int> (2, 30));
    gquiz1.insert(pair <int, int> (3, 60));
    gquiz1.insert(pair <int, int> (4, 20));
    gquiz1.insert(pair <int, int> (5, 50));
    gquiz1.insert(pair <int, int> (6, 50)); 
    gquiz1.insert(pair <int, int> (7, 10));
 
    // printing map gquiz1
    map <int, int> :: iterator itr;
    cout << "\nThe map gquiz1 is : \n";
    cout << "\tKEY\tELEMENT\n";
    for (itr = gquiz1.begin(); itr != gquiz1.end(); ++itr)
    {
        cout  <<  '\t' << itr->first 
              <<  '\t' << itr->second << '\n';
    }
    cout << endl;

}