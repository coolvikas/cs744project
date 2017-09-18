#include <stdio.h>
#include <iostream>
#include <string.h>
using namespace std;

int main(int argc, char const *argv[])
{
	int  filename[50];
	cout<<"sizeof filename = "<<sizeof filename<<endl;
	const char *shared = "new.txt";
	string share = shared;
	cout<<share<<" length=" <<strlen(shared)<<endl;
		return 0;
}