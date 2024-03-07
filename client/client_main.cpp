#include <iostream>
#include <string>

#include "client.h"

using namespace std;

int main(int argc, char *argv[])
{
	string ip = "127.0.0.1";
	if (argc > 1)
	{
		ip = argv[1];
	}

	if (!Client::GetInstance()->Init(ip))
	{
		cerr << "client init failed!" << endl;
		return 0;
	}
	cerr << "client init success!" << endl;

	Client::GetInstance()->Main();
	return 0;
}

