#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS //in order to use strcpy without error
#include <windows.h>
#include <processthreadsapi.h>
#endif

#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <curses.h>
// #include "ftdi.h"
#endif

//common library for both OS
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <math.h>

#include "parser.h"
#include "version.h"
#include "yml.h"
#include "eeprom.h"

int HelloFunc()
{
	char yamfile[128] = {0};
	get_yaml_file_path(yamfile);

	printf("Hello World %s\n", get_board("imx8dxlevk")->name);
	switch (readConf("imx8dxlevk"))
	{
	case 0:
		break;
	case -1:
		printf("Trying to create new config file to %s ...\n", yamfile);
		writeConf();
		if (readConf("imx8dxlevk") < 0)
			return -1;
		break;
	case -2:
		printf("config file read error, please check or delete config file: %s and try again.\n", yamfile);
		return -2;
		break;
	case -3:
		return -3;
		break;
	default:
		break;
	}
}