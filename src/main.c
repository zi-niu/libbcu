/*
* Copyright 2021 NXP.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of the NXP Semiconductor nor the names of its
* contributors may be used to endorse or promote products derived from this
* software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS //in order to use strcpy without error
#include <windows.h>
#include <processthreadsapi.h>
#include <process.h>
#endif

#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
// #include <curses.h>
// #include "ftdi.h"
#endif

#include "options.h"
#include "libbcu.h"
#include "libbcu_version.h"
#include "legacy_bcu.h"

#define LEGACY_MODE 1

#define GET_COLUMN	0
#define GET_ROW		1

int GV_MONITOR_TERMINATED = 0;

void print_err_str(struct options_setting *setting, int err_num)
{
	char err_str[500] = {0};
	printf("%s", bcu_get_err_str(err_num));

	switch (err_num)
	{
	case -LIBBCU_ERR_NO_THIS_BOARD:
		sprintf(err_str, "Missing option <--board=>\n\n"
		       "Or use option <--auto> to find the board automatically\n"
		       "NOTE: if other boards are also connected to the same host, <--auto> may break its ttyUSB function temporarily.\n");
		break;
	case -LIBBCU_ERR_NO_BOOT_MODE_OPT:
		sprintf(err_str, "Please add <-m> or <--bootmode=> option.\n");
		break;
	case -LIBBCU_ERR_NO_GET_OR_SET:
		sprintf(err_str, "Missing option: <--get> or <--set>/<--set=>!\n");
		break;
	case -LIBBCU_ERR_NO_SET_VAL_OPT:
		sprintf(err_str, "Missing option: <--set=>\n"
		       "Please enter a valid output state, 1 to set logic level high, 0 to set it low.\n");
		break;
	case -LIBBCU_ERR_INVALID_GPIO_NAME_OPT:
		sprintf(err_str, "Please enter the name of the gpio pin by using option <-g>/<--gpioname=>.\n"
		       "To check a list of available gpio pin, please use command:\n  ./bcu lsgpio --board=xxx\n");
		break;
	case -LIBBCU_ERR_EEPROM_NO_OPT:
		sprintf(err_str, "Missing option: <-w>/<--write> or <-r>/<read>!\n");
		break;
	case -LIBBCU_ERR_UNSPPORTED_GPIO:
		sprintf(err_str, "Please double check the GPIO name with command:\n"
		       "./bcu lsgpio --board=xxx\n");
		break;
	case -LIBBCU_ERR_EEPROM_BREV_MISMATCH:
		sprintf(err_str, "If the board has been reworked to a new revision, please use the below command to update the value in EEPROM:\n"
			  "\n# ./bcu eeprom -w -board=[new BOARDNAME showed in cmd lsboard]\n");
		break;
	case -LIBBCU_ERR_EEPROM_BOARD_MISMATCH:
		sprintf(err_str, "Please check the value of <--board>/<-b>.\n");
		break;
	case -LIBBCU_ERR_EEPROM_EMPTY:
		sprintf(err_str, "Please use below command to program the EEPROM.\n"
				 "\n# ./bcu eeprom -w -board=%s\n"
				 "\nThen you can choose to replace option <-board=> with <-auto>.\n"
				 "NOTE: If other boards are also connected to the same host, <-auto> may break its ttyUSB function temporarily.\n",
		setting->board);
		break;
	}

	printf("%s", err_str);
}

static int monitor_size(int columns_or_rows)
{
#ifdef _WIN32
	//printf("monitor_dimension not yet implemented for windows\n");
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int columns, rows;

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	//printf("columns: %d\n", columns);
	//printf("rows: %d\n", rows);
	if (columns_or_rows == 0)
		return columns;
	else
		return rows;

#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	// printf ("current lines %d\n", w.ws_row);
	// printf ("current columns %d\n", w.ws_col);
	if (columns_or_rows == GET_COLUMN)
		return w.ws_col;
	else
		return w.ws_row;
#endif
}

int init(struct options_setting *setting)
{
	int ret;
	ret = bcu_init(setting);
	print_err_str(setting, ret);
	return ret;
}

int deinit(struct options_setting *setting)
{
	int ret;
	ret = bcu_deinit(setting);
	print_err_str(setting, ret);
	return ret;
}

int reset(struct options_setting *setting)
{
	int ret;
	int reset_time_ms = bcu_reset_time_ms(setting);

	printf("Board %s will reset in %.1f seconds...\n", setting->board, reset_time_ms / 1000.0);
	ret = bcu_reset(setting);
	print_err_str(setting, ret);
	return ret;
}

int onoff(struct options_setting *setting)
{
	int ret;
	ret = bcu_onoff(setting, setting->hold);
	print_err_str(setting, ret);
	return ret;
}

int bootmode(struct options_setting *setting)
{
	int bootconfiglen = 0, ret;

	ret = bcu_bootmode(setting, &bootconfiglen);
	if (ret)
	{
		print_err_str(setting, ret);
		return ret;
	}

	switch (setting->get_or_set)
	{
	case 1: //set
		printf("Set boot_mode: %s successfully!\n", setting->boot_mode_name);
		break;
	case 2: //get
		if (bootconfiglen)
		{
			printf("Get boot_mode: %s, boot_mode_hex: 0x%x, ", setting->boot_mode_name, setting->boot_mode_hex);
			for (int i = 0; i < bootconfiglen; i++)
				printf("boot_config_%d_hex: 0x%x\n", i, setting->boot_config_hex[i]);
		}
		else
			printf("Get boot_mode: %s, hex value: 0x%x\n", setting->boot_mode_name, setting->boot_mode_hex);
		break;
	default:
		break;
	}

	return 0;
}

int gpio(struct options_setting *setting)
{
	int ret;

	ret = bcu_gpio(setting);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

	switch (setting->get_or_set)
	{
	case 1: //set
		printf("Set GPIO: %s to %s successfully!\n", setting->gpio_name, setting->output_state ? "HIGH" : "LOW");
		break;
	case 2: //get
		printf("Get GPIO: %s level: %s\n", setting->gpio_name, ret ? "HIGH" : "LOW");
		break;
	default:
		break;
	}

	return 0;
}

int eeprom(struct options_setting *setting)
{
	int ret;
	eeprom_informations eeprom_info;
	memset(&eeprom_info, 0, sizeof(eeprom_informations));

	ret = bcu_eeprom(setting, &eeprom_info);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

	switch (setting->eeprom_function)
	{
	case PARSER_EEPROM_READ_AND_PRINT:
		break;
	case PARSER_EEPROM_WRITE_DEFAULT:
		printf("Write %s default values to %s EEPROM successfully\n\n", setting->board, eeprom_info.type);
		break;
	case PARSER_EEPROM_UPDATE_USER_SN:
		printf("Write %s default values to %s EEPROM successfully\n", setting->board, eeprom_info.type);
		printf("Update user SN %d to %s EEPROM successfully\n\n", eeprom_info.user_sn, eeprom_info.type);
		break;
	case PARSER_EEPROM_ERASE:
		printf("Erase %s EEPROM successfully\n\n", eeprom_info.type);
		break;
	default:
		break;
	}

	printf(">>>>>> Registered %s EEPROM on board <<<<<<\n\n", eeprom_info.type);
	if (strlen(eeprom_info.ftdi_sn))
		printf("FTDI EEPROM SN: %s\n", eeprom_info.ftdi_sn);
	printf("Board Info: %s Rev %s\n", eeprom_info.board_id, eeprom_info.board_rev);
	printf("  SoC Info: %s Rev %s\n", eeprom_info.soc_id, eeprom_info.soc_rev);
	if (strlen(eeprom_info.pmic_rev))
		printf(" PMIC Info: %s Rev %s\n", eeprom_info.pmic_id, eeprom_info.pmic_rev);
	else
		printf(" PMIC Info: %s\n", eeprom_info.pmic_id);
	printf("Number of available power rails: %d\n", eeprom_info.rail_number);
	printf("Serial Number: %d\n", eeprom_info.user_sn);

	return 0;
}

void handle_sigint(int sig)
{
	printf("\nTerminating monitor command: %d\n", sig);
	GV_MONITOR_TERMINATED = 1;
}

char* g_vt_clear = (char*)"\x1B[2J";
char* g_vt_clear_remain = (char*)"\x1B[0J";
char* g_vt_clear_line = (char*)"\x1B[K";
char* g_vt_return_last_line = (char*)"\x1B[1A";
char* g_vt_home = (char*)"\x1B[H";
#ifdef _WIN32
char* g_vt_back_enable = (char*)"\x1B[4m";
char* g_vt_back_default = (char*)"\x1B[24m";
#endif
#if defined(linux) || defined(__APPLE__)
char* g_vt_back_enable = (char*)"\x1B[100m";
char* g_vt_back_default = (char*)"\x1B[49m";
#endif

#define FIRST_LINE 0
#define NORMAL_LINE 1
#define LAST_LINE 2
#define TABLE_RAIL 1
#define TABLE_GROUP 2
#define PRINTF_MIDDLE(TEMP, STR, LEN) sprintf(TEMP, "%*s%*s", (LEN + (int)strlen(STR) / 2), STR, (LEN - (int)strlen(STR) / 2), "")
#define PRINTF_MIDDLE_COLOR(TEMP, STR, OSTR, LEN) sprintf(TEMP, "%*s%*s", (LEN + 11 + (int)strlen(OSTR) / 2), STR, (LEN - (int)strlen(OSTR) / 2), "")
int LEN_RAIL_NAME = 25;
int LEN_RAIL_V =  6;
int LEN_RAIL_C =  8;
int LEN_RAIL_P =  9;
int LEN_GROUP_P =  9;
int LEN_RANGE_1 =  8;
int LEN_RANGE_2 =  6;

struct display_colum_len
{
	int hot_key;
	int len_name;
	int len_v;
	int len_c;
	int len_p;
	int len_g_p;
	int len_range;
	int available_width;
};

void display_line(struct display_colum_len *col_len, int flag, int table, char *out_buff)
{
	int len_all;
	int len_middle;
	int len_min;
	int len_need = 0;
	char temp[50] = {0};

	if (table == TABLE_RAIL)
	{
		len_all = col_len->hot_key + col_len->len_name + col_len->len_v + col_len->len_c + col_len->len_p + col_len->len_range;
		len_middle = col_len->hot_key + col_len->len_name + col_len->len_c + col_len->len_p + col_len->len_range;
		len_min = col_len->hot_key + col_len->len_name + col_len->len_p + col_len->len_range;
	} else if (table == TABLE_GROUP)
	{
		len_min = col_len->len_name + col_len->len_g_p;
	}

	if (col_len->available_width > len_all)
		len_need = len_all;
	else if (col_len->available_width > len_middle)
		len_need = len_middle;
	else if (col_len->available_width > len_min)
		len_need = len_min;

	if (table == TABLE_GROUP)
		len_need = len_min;

	for (int i = 0; i < len_need; i++)
	{
		sprintf(temp, "-");
		strcat(out_buff, temp);
	}

	sprintf(temp, "%s\n", g_vt_clear_line);
	strcat(out_buff, temp);
}

#ifdef _WIN32
#include <conio.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#endif

static int kb_hit() 
{
#ifdef _WIN32
	return _kbhit();
#else	
	struct termios term;
	tcgetattr(0, &term);

	struct termios term2 = term;
	term2.c_lflag &= ~ICANON;
	tcsetattr(0, TCSANOW, &term2);

	int byteswaiting;
	ioctl(0, FIONREAD, &byteswaiting);

	tcsetattr(0, TCSANOW, &term);

	return byteswaiting > 0;
#endif
}

static char catch_input_char()
{
	char ch = -1;
	if (kb_hit())
	{
#ifdef _WIN32
		ch = _getch();
#else
#ifdef __APPLE__
		system("stty -icanon");
#endif
		ch = (char)getchar();
#endif

	}
	return ch;
}

int monitor(struct options_setting *setting)
{
	signal(SIGINT, handle_sigint);
	int ret;
	powers power_val;
	char output_buff[30000];

	memset(&power_val, 0, sizeof(power_val));

	ret = bcu_monitor_perpare(setting);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

	LEN_RAIL_NAME = bcu_return_rail_name_max_len(setting);
	if (LEN_RAIL_NAME < 14)
		LEN_RAIL_NAME = 14;
	if (LEN_RAIL_NAME % 2)
		LEN_RAIL_NAME++;

	while(!GV_MONITOR_TERMINATED && !(bcu_monitor_is_stop()))
	{
		ret = bcu_monitor_getvalue(setting, &power_val);
		if (ret) {
			print_err_str(setting, ret);
			break;
		}

		if (!power_val.rail_num)
			continue;

		if (power_val.range_ctrl == MONITOR_RANGE_MA) {
			LEN_RAIL_C = 8;
			LEN_RAIL_P = 9;
		} else if (power_val.range_ctrl == MONITOR_RANGE_AUTO)
			LEN_RAIL_C = LEN_RAIL_P = 11;
		else
			LEN_RAIL_C = LEN_RAIL_P = 11;

		struct display_colum_len col_len;
		col_len.hot_key = 2;
		col_len.len_name = 1 + LEN_RAIL_NAME + 2;
		col_len.len_v = 1 + LEN_RAIL_V * 4 + 2;
		col_len.len_c = 1 + LEN_RAIL_C * 4 + 2;
		col_len.len_p = 1 + LEN_RAIL_P * 4 + 2;
		col_len.len_range = 1 + 3 + LEN_RANGE_1 + 3 + LEN_RANGE_2 + 3;
		col_len.len_g_p = 1 + LEN_GROUP_P * 4 + 3;
		int len_all = col_len.hot_key + col_len.len_name + col_len.len_v + col_len.len_c + col_len.len_p + col_len.len_range;
		int len_middle = col_len.hot_key + col_len.len_name + col_len.len_c + col_len.len_p + col_len.len_range;
		int len_min = col_len.hot_key + col_len.len_name + col_len.len_p + col_len.len_range;
		int max_length, location_length, available_width, available_height;
		available_width = monitor_size(GET_COLUMN);
		available_height = monitor_size(GET_ROW);
		if (available_width < len_min + 1)
		{
			printf("%s", g_vt_home);
			printf("The terminal size is too small!%s\n", g_vt_clear_line);
			printf("Monitor needs at least %d width to show power data!%s\n", len_min + 1, g_vt_clear_line);
			printf("Now terminal width is: %d%s\n", available_width, g_vt_clear_line);
			printf("%s", g_vt_clear_remain);
#ifdef _WIN32
			Sleep(100);
#else
			usleep(1000 * 100);
#endif
			continue;
		}
		if (available_height < power_val.rail_num + power_val.group_num + 14 + 1)
		{
			printf("%s", g_vt_home);
			printf("The terminal size is too small!%s\n", g_vt_clear_line);
			printf("Monitor needs at least %d height to show power data!%s\n", power_val.rail_num + power_val.group_num + 14 + 1, g_vt_clear_line);
			printf("Now terminal height is: %d%s\n", available_height, g_vt_clear_line);
			printf("%s", g_vt_clear_remain);
#ifdef _WIN32
			Sleep(100);
#else
			usleep(1000 * 100);
#endif
			continue;
		}
		col_len.available_width = available_width;
		char temp[200] = {0};
		memset(&output_buff, 0, sizeof(output_buff));

		if (!setting->nodisplay)
		{
			display_line(&col_len, FIRST_LINE, TABLE_RAIL, output_buff);
			//header first line
			sprintf(temp, "| | ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Rail Name", LEN_RAIL_NAME / 2);
			strcat(output_buff, temp);
			sprintf(temp, " | ");
			strcat(output_buff, temp);
			if (available_width > len_all)
			{
				PRINTF_MIDDLE(temp, "Voltage(V)", LEN_RAIL_V * 2);
				strcat(output_buff, temp);
				sprintf(temp, " | ");
				strcat(output_buff, temp);
			}
			if (available_width > len_middle)
			{
				if (power_val.range_ctrl == MONITOR_RANGE_MA)
					PRINTF_MIDDLE(temp, "Current(mA)", LEN_RAIL_C * 2);
				else if (power_val.range_ctrl == MONITOR_RANGE_AUTO) {
					char tp_title[50] = {0};
					sprintf(tp_title, "Current(mA)/%s(uA)%s", g_vt_back_enable, g_vt_back_default);
					PRINTF_MIDDLE_COLOR(temp, tp_title, "Current(mA)/(uA)", LEN_RAIL_C * 2);
				} else
					PRINTF_MIDDLE(temp, "Current(uA)", LEN_RAIL_C * 2);
				strcat(output_buff, temp);
				sprintf(temp, " | ");
				strcat(output_buff, temp);
			}
			if (power_val.range_ctrl == MONITOR_RANGE_MA)
				PRINTF_MIDDLE(temp, "Power(mWatt)", LEN_RAIL_P * 2);
			else if (power_val.range_ctrl == MONITOR_RANGE_AUTO) {
				char tp_title[50] = {0};
				sprintf(tp_title, "Power(mWatt)/%s(uWatt)%s", g_vt_back_enable, g_vt_back_default);
				PRINTF_MIDDLE_COLOR(temp, tp_title, "Power(mWatt)/(uWatt)", LEN_RAIL_P * 2);
			} else
				PRINTF_MIDDLE(temp, "Power(uWatt)", LEN_RAIL_P * 2);
			strcat(output_buff, temp);
			sprintf(temp, " | ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "C-Range Ctrl(mA)", (LEN_RANGE_1 + LEN_RANGE_2 + 6) / 2);
			strcat(output_buff, temp);
			sprintf(temp, " |%s\n", g_vt_clear_line);
			strcat(output_buff, temp);

			//header second line
			sprintf(temp, "| | ");
			strcat(output_buff, temp);
			sprintf(temp, "%*s | ", LEN_RAIL_NAME, "");
			strcat(output_buff, temp);
			if (available_width > len_all)
			{
				sprintf(temp, "%*s%*s%*s%*s | ", LEN_RAIL_V, "now", LEN_RAIL_V, "avg", LEN_RAIL_V, "max", LEN_RAIL_V, "min");
				strcat(output_buff, temp);
			}
			if (available_width > len_middle)
			{
				sprintf(temp, "%*s%*s%*s%*s | ", LEN_RAIL_C, "now", LEN_RAIL_C, setting->use_rms ? "rms" : "avg", LEN_RAIL_C, "max", LEN_RAIL_C, "min");
				strcat(output_buff, temp);
			}
			sprintf(temp, "%*s%*s%*s%*s | ", LEN_RAIL_P, "now", LEN_RAIL_P, "avg", LEN_RAIL_P, "max", LEN_RAIL_P, "min");
			strcat(output_buff, temp);
			sprintf(temp, "%-*s%-*s", LEN_RANGE_1 + 3, "Range1", LEN_RANGE_2 + 3, "Range2");
			strcat(output_buff, temp);
			sprintf(temp, " |%s\n", g_vt_clear_line);
			strcat(output_buff, temp);
			display_line(&col_len, NORMAL_LINE, TABLE_RAIL, output_buff);

			for (int i = 0; i < power_val.rail_num; i++)
			{
				if (i < 26)
					sprintf(temp, "|%c| ", i + 'A');
				else
					sprintf(temp, "|%c| ", i + 'a' - 26);
				strcat(output_buff, temp);
				sprintf(temp, "%-*s | ", LEN_RAIL_NAME, power_val.rail_infos[i].rail_name);
				strcat(output_buff, temp);
				if (available_width > len_all) {
					sprintf(temp, "%*.2f", LEN_RAIL_V, power_val.rail_infos[i].v_now);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_V, power_val.rail_infos[i].v_avg);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_V, power_val.rail_infos[i].v_max);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_V, power_val.rail_infos[i].v_min);
					strcat(output_buff, temp);
					sprintf(temp, " | ");
					strcat(output_buff, temp);
				}
				if (available_width > len_middle) {
					if (power_val.rail_infos[i].range_level && power_val.range_ctrl == MONITOR_RANGE_AUTO) {
						sprintf(temp, "%s", g_vt_back_enable);
						strcat(output_buff, temp);
					}
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_now);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_avg);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_max);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_min);
					strcat(output_buff, temp);
					if (power_val.rail_infos[i].range_level && power_val.range_ctrl == MONITOR_RANGE_AUTO) {
						sprintf(temp, "%s", g_vt_back_default);
						strcat(output_buff, temp);
					}
					sprintf(temp, " | ");
					strcat(output_buff, temp);
				}
				if (power_val.rail_infos[i].range_level && power_val.range_ctrl == MONITOR_RANGE_AUTO) {
					sprintf(temp, "%s", g_vt_back_enable);
					strcat(output_buff, temp);
				}
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.rail_infos[i].p_now);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.rail_infos[i].p_avg);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.rail_infos[i].p_max);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.rail_infos[i].p_min);
				strcat(output_buff, temp);
				if (power_val.rail_infos[i].range_level && power_val.range_ctrl == MONITOR_RANGE_AUTO) {
					sprintf(temp, "%s", g_vt_back_default);
					strcat(output_buff, temp);
				}
				sprintf(temp, " | ");
				strcat(output_buff, temp);
				sprintf(temp, "[%c]%-*.1f", power_val.rail_infos[i].range_switch ? '*' : ' ', LEN_RANGE_1, power_val.rail_infos[i].range_list[0]);
				strcat(output_buff, temp);
				if (power_val.rail_infos[i].range_switch >= 0)
					sprintf(temp, "[%c]%-*.1f", power_val.rail_infos[i].range_switch ? ' ' : '*', LEN_RANGE_2, power_val.rail_infos[i].range_list[1]);
				else
					sprintf(temp, "   %*s", LEN_RANGE_2, "");
				strcat(output_buff, temp);
				sprintf(temp, " |%s\n", g_vt_clear_line);
				strcat(output_buff, temp);
			}
			display_line(&col_len, LAST_LINE, TABLE_RAIL, output_buff);

			//display group table
			display_line(&col_len, FIRST_LINE, TABLE_GROUP, output_buff);
			sprintf(temp, "| ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Group Name", LEN_RAIL_NAME / 2);
			strcat(output_buff, temp);
			sprintf(temp, " | ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Power(mWatt)", LEN_GROUP_P * 2);
			strcat(output_buff, temp);
			sprintf(temp, " |%s\n", g_vt_clear_line);
			strcat(output_buff, temp);
			sprintf(temp, "| %-*s | %*s%*s%*s%*s |%s\n", LEN_RAIL_NAME, "",
			LEN_GROUP_P, "now", LEN_GROUP_P, "avg", LEN_GROUP_P, "max", LEN_GROUP_P, "min", g_vt_clear_line);
			strcat(output_buff, temp);
			display_line(&col_len, NORMAL_LINE, TABLE_GROUP, output_buff);
			for (int i = 0; i < power_val.group_num; i++)
			{
				sprintf(temp, "| %-*s | ", LEN_RAIL_NAME, power_val.group_infos[i].group_name);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_GROUP_P, power_val.group_infos[i].p_now);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_GROUP_P, power_val.group_infos[i].p_avg);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_GROUP_P, power_val.group_infos[i].p_max);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_GROUP_P, power_val.group_infos[i].p_min);
				strcat(output_buff, temp);
				sprintf(temp, " |%s\n", g_vt_clear_line);
				strcat(output_buff, temp);
			}
			display_line(&col_len, LAST_LINE, TABLE_GROUP, output_buff);
		}
		if (available_width > len_middle) {
			sprintf(temp, "Total sample times: %lld   ", power_val.sample_times);
			strcat(output_buff, temp);
		}
		unsigned long interval = power_val.time_now - power_val.time_start;
		if (interval < 10000)
			sprintf(temp, "Sampling time: %ldms   ", interval);
		else
			sprintf(temp, "Sampling time: %.1lfs   ", interval / 1000.0);
		strcat(output_buff, temp);
			sprintf(temp, "Sampling rate: %.1lftimes/s", power_val.sample_times / (interval / 1000.0));
		strcat(output_buff, temp);
		sprintf(temp, "%s\n", g_vt_clear_line);
		strcat(output_buff, temp);
		if (setting->dump)
		{
			sprintf(temp, "Dump data to %s ", setting->dumpname);
			strcat(output_buff, temp);
			if (setting->dump_statistics)
			{
				sprintf(temp, "with stats data");
				strcat(output_buff, temp);
			}
			sprintf(temp, "%s\n", g_vt_clear_line);
			strcat(output_buff, temp);
		}
		sprintf(temp, "Hot-key: 1=reset %s; 2=reset MaxMin; 3=reset %s and MaxMin; 4=switch show mA/auto/uA; Ctrl-C to exit...%s\n",
						setting->use_rms ? "RMS" : "Avg", setting->use_rms ? "RMS" : "Avg", g_vt_clear_line);
		strcat(output_buff, temp);
		// sprintf(temp, "         5=reset board; 6=resume the board; Ctrl-C to exit...%s\n", g_vt_clear_line);
		// strcat(output_buff, temp);
		sprintf(temp, "press the letter on keyboard to control coresponding extra sense resistor(Extra SR)%s\n", g_vt_clear_line);
		strcat(output_buff, temp);
		sprintf(temp, "pressed (Please pay attention to letter case): %s\n", g_vt_clear_line);
		strcat(output_buff, temp);

		printf("%s", g_vt_home);
		// printf("current lines %d%s\n", available_height, g_vt_clear_line);
		// printf("current columns %d railname: %d%s\n", available_width, LEN_RAIL_NAME, g_vt_clear_line);
		printf("%s", output_buff);
		printf("%s", g_vt_clear_remain);

		char ch = catch_input_char();
		if (ch > 0)
			bcu_monitor_set_hotkey(ch);

		int ms_each_sample = (power_val.time_now - power_val.time_start) / power_val.sample_times;
		int ms_delay;
		if (setting->refreshms < ms_each_sample)
			ms_delay = ms_each_sample;
		else
			ms_delay = setting->refreshms;
#ifdef _WIN32
		Sleep(ms_delay);
#else
		usleep(1000 * ms_delay);
#endif
	}

	ret = bcu_monitor_unperpare(setting);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

	return 0;
}

#if defined(linux) || defined(__APPLE__)
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define PORT 65432

char *iso8601(char *local_time)
{
	time_t t_time;
	struct tm *t_tm;
 
	t_time = time(NULL);
	t_tm = localtime(&t_time);
	if (t_tm == NULL)
		return NULL;
 
	if (strftime(local_time, 32, "%FT%T%z", t_tm) == 0)
		return NULL;

	local_time[25] = local_time[24];
	local_time[24] = local_time[23];
	local_time[22] = ':';
	local_time[26] = '\0';
	return local_time;
}

int server(struct options_setting *setting)
{
	signal(SIGINT, handle_sigint);
	int ret;
	powers power_val;

	int serverSocket;

	struct sockaddr_in server_addr;
	struct sockaddr_in clientAddr;
	int addr_len = sizeof(clientAddr);
	int client;
	char sendbuf[4096] = { 0 };
	char recvbuf[4096] = { 0 };
	int iDataNum;
	char logtime[32] = { 0 };
	int accepted;

	#ifdef _WIN32
	WSADATA wsaData;
	unsigned long ul = 1;
	// Initialize Winsock
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0) {
		printf("WSAStartup failed with error: %d\n", ret);
		return 1;
	}
#endif

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
	if (serverSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
#else
	if (serverSocket < 0)
	{
		perror("socket");
#endif
		return -1;
	}

#ifdef _WIN32
	ret = ioctlsocket(serverSocket, FIONBIO, (unsigned long*)&ul);
	if (ret == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return -10;
	}
#endif

	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(serverSocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
#ifdef _WIN32
	if (ret == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
#else
	if (ret < 0)
	{
		perror("connect");
#endif
		return -11;
	}

	ret = listen(serverSocket, 5);
#ifdef _WIN32
	if (ret == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
#else
	if (ret < 0)
	{
		perror("listen");
#endif
		return -12;
	}

retry:
	printf("[%s] listening Port %d on all IPs.\n", iso8601(logtime), PORT);
	accepted = 0;
	do {
		client = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*)&addr_len);
#ifdef _WIN32
		if (client == INVALID_SOCKET)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				printf("accept failed with error: %d\n", WSAGetLastError());
				closesocket(serverSocket);
				WSACleanup();
				return -10;
			}
#else
		if (client < 0)
		{
			perror("accept");
			return -10;
#endif
		}
		else
		{
			accepted = 1;
		}
	} while (!accepted);
	printf("[%s] IP is %s\n", iso8601(logtime), inet_ntoa(clientAddr.sin_addr));
	printf("[%s] Port is %d\n", iso8601(logtime), htons(clientAddr.sin_port));
	printf("[%s] Start collecting data...\n", iso8601(logtime));
	printf("[%s] Waiting request message...\n", iso8601(logtime));

	memset(&power_val, 0, sizeof(power_val));

	ret = bcu_monitor_perpare(setting);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

	while(!GV_MONITOR_TERMINATED && !(bcu_monitor_is_stop()))
	{
		ret = bcu_monitor_getvalue(setting, &power_val);
		if (ret) {
			print_err_str(setting, ret);
			break;
		}

		if (!power_val.rail_num)
			continue;

#ifdef _WIN32
		iDataNum = recv(client, recvbuf, 1024, 0);
#else
		iDataNum = recv(client, recvbuf, 1024, MSG_DONTWAIT);
#endif
		if (iDataNum > 0)
		{
			// printf("Received data: [%s]\n", recvbuf);
			if (strcmp(recvbuf, "data request") == 0)
			{
				printf("[%s] Data Request.\n", iso8601(logtime));
				strcpy(sendbuf, "");

				char time[32] = {0};
				strcat(sendbuf, iso8601(time));
				strcat(sendbuf, ";");

				for (int i = 0; i < power_val.rail_num; i++)
				{
					strcat(sendbuf, power_val.rail_infos[i].rail_name);
					strcat(sendbuf, ":");
					char temp[20] = {0};
					sprintf(temp, "%lf", power_val.rail_infos[i].p_avg);
					strcat(sendbuf, temp);
					strcat(sendbuf, ";");
				}

				send(client, sendbuf, strlen(sendbuf), 0);

				bcu_monitor_set_hotkey('1');
			}
			else if (strcmp(recvbuf, "quit") == 0)
			{
				printf("[%s] Client requests to exit server monitor...\n", iso8601(logtime));
				ret = 1;
				break;
			} else {
				send(client, "Unsupported request!", strlen("unsupported request!"), 0);
			}
		} else if (iDataNum == 0){
			printf("[%s] Client disconnects! Stop collecting data...\n", iso8601(logtime));
			ret = 2;
			ret = bcu_monitor_unperpare(setting);
			if (ret < 0)
			{
				print_err_str(setting, ret);
				return ret;
			}
			goto retry;
		}

		memset(recvbuf, 0, sizeof recvbuf);
	}

	ret = bcu_monitor_unperpare(setting);
	if (ret < 0)
	{
		print_err_str(setting, ret);
		return ret;
	}

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}

int lsftdi(struct options_setting *setting)
{
	char boardlist[MAX_NUMBER_OF_BOARD][MAX_MAPPING_NAME_LENGTH] = {0};
	char location_id_str[MAX_NUMBER_OF_USB_DEVICES][MAX_LOCATION_ID_LENGTH] = {0};
	int boardnum = 0;

	boardnum = bcu_lsftdi(setting, boardlist, location_id_str);
	if (boardnum < 0)
	{
		print_err_str(setting, boardnum);
		return boardnum;
	}
	else if (boardnum == 0)
		printf("No board connected to the host!\n");
	else if (boardnum == 1)
		printf("There is 1 board on this host.\n\n");
	else if (boardnum > 1)
	{
		printf("There are %d boards on this host.\n", boardnum);
		printf("Please add [--board=] or [--id=] option when use other commands\n\n");
	}

	for (int i = 0; i < boardnum; i++)
	{
		if (setting->auto_find_board)
			printf("Board: %-20s is on location: id=%s\n", boardlist[i], location_id_str[i]);
		else
			printf("Board is on location: id=%s\n", location_id_str[i]);
	}

	return boardnum;
}

int lsboard(void)
{
	char boardlist[MAX_NUMBER_OF_BOARD][MAX_MAPPING_NAME_LENGTH] = {0};
	int boardnum = 0;

	boardnum = bcu_lsboard(boardlist);
	if (boardnum < 0)
	{
		print_err_str(NULL, boardnum);
		return boardnum;
	}

	printf("\nlist of supported board model:\n\n");
	for (int i = 0; i < boardnum; i++)
	{
		printf("	%s\n", boardlist[i]);
	}

	return boardnum;
}

int lsgpio(struct options_setting *setting)
{
	char gpiolist[MAX_NUMBER_OF_GPIO][MAX_MAPPING_NAME_LENGTH] = {0};
	int gpionum = 0;

	gpionum = bcu_lsgpio(setting, gpiolist);
	if (gpionum < 0)
	{
		print_err_str(setting, gpionum);
		return gpionum;
	}

	printf("\navailable gpio:\n\n");
	for (int i = 0; i < gpionum; i++)
	{
		printf("	%s\n", gpiolist[i]);
	}

	return gpionum;
}

int lsbootmode(struct options_setting *setting)
{
	char bootmodelist[MAX_NUMBER_OF_BOOTMODE][MAX_MAPPING_NAME_LENGTH] = {0};
	int bootmodenum = 0;

	bootmodenum = bcu_lsbootmode(setting, bootmodelist);
	if (bootmodenum < 0)
	{
		print_err_str(setting, bootmodenum);
		return bootmodenum;
	}

	printf("\navailable boot mode:\n");
	for (int i = 0; i < bootmodenum; i++)
	{
		printf("	%s\n", bootmodelist[i]);
	}

	return bootmodenum;
}

void terminateBCU(void)
{
	bcu_remove_all_ftdi_port();
}

void print_version()
{
	printf("version %s\n", LIBBCU_GIT_VERSION);
}

void print_help(char *arg)
{
	printf("Usage: <COMMAND> [-m:ab:i:d::nwrg:vhc]\n");
	printf("  Support commands:\n");
	printf("    reset      reset the board, and then boot from BOOTMODE_NAME\n"
	       "               or the boot mode value set by [--boothex=] [--bootbin=]\n");
	print_opt("reset");

	printf("    onoff      press the ON/OFF button once for -hold= time(ms)\n");
	print_opt("onoff");

	printf("    init       enable the remote control with a boot mode\n");
	print_opt("init");

	printf("    deinit     disable the remote control\n");
	print_opt("deinit");

	printf("    monitor    monitor power consumption\n");
	print_opt("monitor");

	printf("    server     monitor power consumption over TCP/IP server\n");
	print_opt("server");

	printf("    eeprom     EEPROM read and program\n");
	print_opt("eeprom");

	printf("    gpio       set/get level state of pin GPIO_NAME\n");
	print_opt("gpio");

	printf("    bootmode   set/get the boot mode\n");
	print_opt("bootmode");

	printf("    lsftdi     list all boards connected by ftdi device\n");
	print_opt("lsftdi");

	printf("    lsboard    list all supported board models\n");
	printf("\n");
	// print_opt("lsboard");

	printf("    lsbootmode show a list of available BOOTMODE_NAME of a board\n");
	print_opt("lsbootmode");

	printf("    lsgpio     show a list of available GPIO_NAME of a board\n");
	print_opt("lsgpio");

}

int main(int argc, char **argv)
{
	if (argc > 1)
		if (strstr(argv[1], "bcu") != NULL)
			if (auto_complete(argc, argv) == 0)
				return 0;

	char* cmd = argv[1];
	struct options_setting setting;
	int ret;
	char yamfile[128] = {0};
	bcu_get_yaml_file_path(yamfile);

	atexit(terminateBCU);

	switch (argc)
	{
	case 1:
		print_help(NULL);
		return 0;
	case 2: 
		if (strcmp(argv[1], "conf_path") == 0 || strcmp(argv[1], "-cp") == 0)
		{
			printf("\nBCU Config file path: %s\n\n", yamfile);
			return 0;
		}
		else if (strcmp(cmd, "help") == 0 || strcmp(argv[1], "-h") == 0)
		{
			print_help(NULL);
			return 0;
		}
	default:
		break;
	}
	
#if LEGACY_MODE
	memset(&setting, 0, sizeof(struct options_setting));//initialized to zero
	set_options_default(&setting);

	//legacy_parse_board_id_options(cmd, argc, argv, &setting);

	if (legacy_parse_options(cmd, argc, argv, &setting) == -1) {
		//return 0;
	}
#else
	ret = opt_parser(argc, argv, &setting, cmd);
	if (ret)
		return ret;
#endif

	bcu_update_debug_level(setting.debug);

	if (strlen(setting.board) || setting.auto_find_board)
	{
		ret = bcu_get_yaml_file(&setting, yamfile);
		if (ret < 0)
		{
			print_err_str(&setting, ret);
			return ret;
		}
	}


	if (strlen(setting.location_id_string))
		bcu_update_location_id(setting.location_id_string);

	if (strlen(setting.boot_mode_name))
	{
		ret = bcu_get_boot_mode_hex(&setting);
		if (ret) {
			print_err_str(&setting, ret);
			return ret;
		}
	}
	if (strlen(setting.gpio_name))
	{
		ret = bcu_check_gpio_name(&setting);
		if (ret) {
			print_err_str(&setting, ret);
			return ret;
		}
	}
	if (strlen(setting.board))
	{
		ret = bcu_check_eeprom_data(&setting);
		if (ret) {
			print_err_str(&setting, ret);
			return ret;
		}
	}

	if (strcmp(cmd, "gpio") == 0)
	{
		ret = gpio(&setting);
	}
	else if (strcmp(cmd, "bootmode") == 0)
	{
		ret = bootmode(&setting);
	}
	else if (strcmp(cmd, "eeprom") == 0)
	{
		ret = eeprom(&setting);
	}
	else if (strcmp(cmd, "monitor") == 0)
	{
		ret = monitor(&setting);
	}
	else if (strcmp(cmd, "server") == 0)
	{
		ret = server(&setting);
	}
	else if (strcmp(cmd, "lsftdi") == 0)
	{
		ret = lsftdi(&setting);
	}
	else if (strcmp(cmd, "lsboard") == 0)
	{
		ret = lsboard();
	}
	else if (strcmp(cmd, "lsbootmode") == 0)
	{
		ret = lsbootmode(&setting);
	}
	else if (strcmp(cmd, "lsgpio") == 0)
	{
		ret = lsgpio(&setting);
	}
	else if (strcmp(cmd, "reset") == 0)
	{
		ret = reset(&setting);
	}
	else if (strcmp(cmd, "onoff") == 0)
	{
		ret = onoff(&setting);
	}
	else if (strcmp(cmd, "init") == 0)
	{
		ret = init(&setting);
	}
	else if (strcmp(cmd, "deinit") == 0)
	{
		ret = deinit(&setting);
	}
	// else if (strcmp(cmd, "uuu") == 0)
	// {
	// 	uuu(&setting);
	// }
	 else if (strcmp(cmd, "version") == 0)
	 {
	 	print_version();
	 }
	// else if (strcmp(cmd, "upgrade") == 0)
	// {
	// 	upgrade_bcu(&setting);
	// }
#if LEGACY_MODE
	else if (strcmp(cmd, "get_level") == 0)
	{
		if (argc < 4)
		{
			printf("usage: get_level [GPIO_NAME] [-board=/-auto] [-id=]\r\n");
			return 0;
		}
		setting.get_or_set = 2;
		strcpy(setting.gpio_name, argv[2]);
		ret = gpio(&setting);
	}
	else if (strcmp(cmd, "set_gpio") == 0)
	{
		if (argc < 5)
		{
			printf("usage: set_gpio [GPIO_NAME] [1/0] [-board=/-auto] [-id=]\r\n");
			return 0;
		}
		setting.get_or_set = 1;
		strcpy(setting.gpio_name, argv[2]);
		setting.output_state = atoi(argv[3]);
		ret = gpio(&setting);
	}
	else if (strcmp(cmd, "set_boot_mode") == 0)
	{
		if (argc < 4)
		{
			printf("usage: set_boot_mode [BOOTMODE_NAME] [-board=/-auto] [-id=] [-boothex=] [-bootbin=]\r\n");
			return 0;
		}

		setting.get_or_set = 1;
		ret = bootmode(&setting);
	}
	else if (strcmp(cmd, "get_boot_mode") == 0)
	{
		if (argc < 3)
		{
			printf("usage: get_boot_mode[-board=/-auto] [-id=]\r\n");
			return 0;
		}

		setting.get_or_set = 2;
		ret = bootmode(&setting);
	}
#endif
	else
	{
		printf("%s is a invalid command\n\n", cmd);
		// print_help(NULL);
	}

	return ret;
}