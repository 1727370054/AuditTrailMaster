#include <iostream>
#include <string>

#include "agent.h"

using namespace std;

int main(int argc, char *argv[])
{
	string ip = "127.0.0.1";
	if (argc > 2)
	{
		ip = argv[1];
		return -1;
	}

	/// 1. 初始化agent 连接数据库
	if (!Agent::GetInstance()->Init(ip))
	{
		cerr << "Agent init failed!" << endl;
		return -2;
	}
	cout << "Agent start"<< endl;

	Agent::GetInstance()->Main();

	return 0;
}

