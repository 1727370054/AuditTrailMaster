#include <iostream>
#include <string>

#include "center.h"

using namespace std;

enum CmdType
{
	NONUSE = 0,
	INSTALL = 1,
	ADD_DEVICE = 2
};

static void Usage(CmdType type,const char* argv)
{
	cout << "=========== Center Usage ===========" << endl;
	switch (type)
	{
	case NONUSE:
		break;
	case INSTALL:
		cout << argv << " install 127.0.0.1" << endl;
		break;
	case ADD_DEVICE:
		cout << argv << " add 127.0.0.1 dev1" << endl;
		break;
	default:
		break;
	}
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
			Usage(INSTALL, argv[0]);
			return 0;
		}
		ip = argv[2];
		if (!Center::GetInstance()->Install(ip))
		{
			cout << "center install failed!" << endl;
			return 0;
		}
		cout << "center install success!" << endl;
		return 0;
	}

	if (!Center::GetInstance()->Init())
	{
		cerr << "Center init failed!" << endl;
		return -1;
	}

	if (cmd == "add")
	{
		cout << "add device" << endl;
		// ./center add 127.0.0.1 dev1
		if (argc < 4)
		{
			Usage(ADD_DEVICE, argv[0]);
			return 0;
		}
		if (!Center::GetInstance()->AddDevice(argv[2], argv[3]))
		{
			cout << "add device failed!" << endl;
		}
		else
			cout << "add device success!" << endl;

		return 0;
	}

	Center::GetInstance()->Main();
	return 0;
}

