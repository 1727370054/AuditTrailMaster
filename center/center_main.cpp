#include <iostream>
#include <string>

#include "center.h"

using namespace std;

static void Usage(const char* argv)
{
	cout << "=========== Center Usage ===========" << endl;
	cout << argv << " install 127.0.0.1" << endl;
}

int main(int argc, char *argv[])
{
	string cmd = "";
	if (argc > 1)
	{
		cmd = argv[1];
	}

	string ip = "";
	/// 安装系统
	if (cmd == "install")
	{
		cout << "begin install center" << endl;
		if (argc < 3)
		{
			Usage(argv[0]);
			return 0;
		}
		ip = argv[2];
		if (!Center::GetInstance()->Install(ip))
		{
			cout << "center install failed!" << endl;
			return 0;
		}
		cout << "center install success!" << endl;
	}

	return 0;
}

