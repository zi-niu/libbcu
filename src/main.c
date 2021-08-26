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
#include <unistd.h>
#include <pthread.h>

#include "options.h"
#include "libbcu.h"
#include "libbcu_version.h"

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS //in order to use strcpy without error
#include <windows.h>
#include <processthreadsapi.h>
#endif

#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
// #include <curses.h>
// #include "ftdi.h"
#endif

#define GET_COLUMN	0
#define GET_ROW		1

int GV_MONITOR_TERMINATED = 0;

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
	printf("%s", bcu_get_err_str(bcu_init(setting)));
}

int deinit(struct options_setting *setting)
{
	printf("%s", bcu_get_err_str(bcu_deinit(setting)));
}

int reset(struct options_setting *setting)
{
	int reset_time_ms = bcu_reset_time_ms(setting);

	printf("Board %s will reset in %.1f seconds...\n", setting->board, reset_time_ms / 1000.0);
	printf("%s", bcu_get_err_str(bcu_reset(setting)));
}

int onoff(struct options_setting *setting)
{
	int ret = 0;

	ret = bcu_onoff(setting, setting->hold);
	printf("%s", bcu_get_err_str(ret));
}

int bootmode(struct options_setting *setting)
{
	int bootconfiglen = 0, ret;

	ret = bcu_bootmode(setting, &bootconfiglen);
	if (ret)
	{
		printf("%s", bcu_get_err_str(ret));
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
		printf("%s", bcu_get_err_str(ret));
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
		printf("%s", bcu_get_err_str(ret));
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

#define FIRST_LINE 0
#define NORMAL_LINE 1
#define LAST_LINE 2
#define TABLE_RAIL 1
#define TABLE_GROUP 2
#define PRINTF_MIDDLE(TEMP, STR, LEN) sprintf(TEMP, "%*s%*s", (LEN + (int)strlen(STR) / 2), STR, (LEN - (int)strlen(STR) / 2), "")
int LEN_RAIL_NAME = 25;
int LEN_RAIL_V =  6;
int LEN_RAIL_C =  8;
int LEN_RAIL_P =  8;

struct display_colum_len
{
	int hot_key;
	int len_name;
	int len_v;
	int len_c;
	int len_p;
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
		len_all = col_len->hot_key + col_len->len_name + col_len->len_v + col_len->len_c + col_len->len_p;
		len_middle = col_len->hot_key + col_len->len_name + col_len->len_c + col_len->len_p;
		len_min = col_len->hot_key + col_len->len_name + col_len->len_p;
	} else if (table == TABLE_GROUP)
	{
		len_all = col_len->len_name + col_len->len_v + col_len->len_c + col_len->len_p;
		len_middle = col_len->len_name + col_len->len_c + col_len->len_p;
		len_min = col_len->len_name + col_len->len_p;
	}

	if (col_len->available_width > len_all)
		len_need = len_all;
	else if (col_len->available_width > len_middle)
		len_need = len_middle;
	else if (col_len->available_width > len_min)
		len_need = len_min;

	if (table == TABLE_GROUP)
		len_need = len_min;

	switch (flag)
	{
	case FIRST_LINE:
		sprintf(temp, "┌");
		strcat(out_buff, temp);
		break;
	case NORMAL_LINE:
		sprintf(temp, "├");
		strcat(out_buff, temp);
		break;
	case LAST_LINE:
		sprintf(temp, "└");
		strcat(out_buff, temp);
		break;
	default:
		break;
	}

	for (int i = 1; i < len_need - 1; i++)
	{
		if (
			(len_need == len_all && i == (len_need - col_len->len_name - col_len->len_v - col_len->len_c - col_len->len_p)) ||
			(len_need == len_all && i == (len_need - col_len->len_v - col_len->len_c - col_len->len_p)) ||
			(len_need == len_all && i == (len_need - col_len->len_c - col_len->len_p)) ||
			(len_need == len_all && i == (len_need - col_len->len_p)) ||
			(len_need == len_middle && i == (len_need - col_len->len_name - col_len->len_c - col_len->len_p)) ||
			(len_need == len_middle && i == (len_need - col_len->len_c - col_len->len_p)) ||
			(len_need == len_middle && i == (len_need - col_len->len_p)) ||
			(len_need == len_min && i == (len_need - col_len->len_name - col_len->len_p)) ||
			(len_need == len_min && i == (len_need - col_len->len_p))
		)
		{
			switch (flag)
			{
			case FIRST_LINE:
				sprintf(temp, "┬");
				strcat(out_buff, temp);
				break;
			case NORMAL_LINE:
				sprintf(temp, "┼");
				strcat(out_buff, temp);
				break;
			case LAST_LINE:
				sprintf(temp, "┴");
				strcat(out_buff, temp);
				break;
			default:
				break;
			}
			continue;
		}
		sprintf(temp, "─");
		strcat(out_buff, temp);
	}

	switch (flag)
	{
	case FIRST_LINE:
		sprintf(temp, "┐");
		strcat(out_buff, temp);
		break;
	case NORMAL_LINE:
		sprintf(temp, "┤");
		strcat(out_buff, temp);
		break;
	case LAST_LINE:
		sprintf(temp, "┘");
		strcat(out_buff, temp);
		break;
	default:
		break;
	}

	sprintf(temp, "%s\n", g_vt_clear_line);
	strcat(out_buff, temp);
}

static char catch_input_char()
{
	char ch = ' ';
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
		printf("%s", bcu_get_err_str(ret));
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
		if (ret)
			printf("%s", bcu_get_err_str(ret));

		if (!power_val.rail_num)
			continue;

		struct display_colum_len col_len;
		col_len.hot_key = 2;
		col_len.len_name = 1 + LEN_RAIL_NAME + 2;
		col_len.len_v = 1 + LEN_RAIL_V * 4 + 2;
		col_len.len_c = 1 + LEN_RAIL_C * 4 + 2;
		col_len.len_p = 1 + LEN_RAIL_P * 4 + 3;
		int len_all = col_len.hot_key + col_len.len_name + col_len.len_v + col_len.len_c + col_len.len_p;
		int len_middle = col_len.hot_key + col_len.len_name + col_len.len_c + col_len.len_p;
		int len_min = col_len.hot_key + col_len.len_name + col_len.len_p;
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
			usleep(1000 * 100);
			continue;
		}
		col_len.available_width = available_width;
		char temp[100] = {0};
		memset(&output_buff, 0, sizeof(output_buff));

		if (!setting->nodisplay)
		{
			display_line(&col_len, FIRST_LINE, TABLE_RAIL, output_buff);
			//header first line
			sprintf(temp, "│ │ ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Rail Name", LEN_RAIL_NAME / 2);
			strcat(output_buff, temp);
			sprintf(temp, " │ ");
			strcat(output_buff, temp);
			if (available_width > len_all)
			{
				PRINTF_MIDDLE(temp, "Voltage(V)", LEN_RAIL_V * 2);
				strcat(output_buff, temp);
				sprintf(temp, " │ ");
				strcat(output_buff, temp);
			}
			if (available_width > len_middle)
			{
				PRINTF_MIDDLE(temp, "Current(mA)", LEN_RAIL_C * 2);
				strcat(output_buff, temp);
				sprintf(temp, " │ ");
				strcat(output_buff, temp);
			}
			PRINTF_MIDDLE(temp, "Power(mWatt)", LEN_RAIL_P * 2);
			strcat(output_buff, temp);
			sprintf(temp, " │%s\n", g_vt_clear_line);
			strcat(output_buff, temp);

			//header second line
			sprintf(temp, "│ │ ");
			strcat(output_buff, temp);
			sprintf(temp, "%*s │ ", LEN_RAIL_NAME, "");
			strcat(output_buff, temp);
			if (available_width > len_all)
			{
				sprintf(temp, "%*s%*s%*s%*s │ ", LEN_RAIL_V, "now", LEN_RAIL_V, "avg", LEN_RAIL_V, "max", LEN_RAIL_V, "min");
				strcat(output_buff, temp);
			}
			if (available_width > len_middle)
			{
				sprintf(temp, "%*s%*s%*s%*s │ ", LEN_RAIL_C, "now", LEN_RAIL_C, setting->use_rms ? "rms" : "avg", LEN_RAIL_C, "max", LEN_RAIL_C, "min");
				strcat(output_buff, temp);
			}
			sprintf(temp, "%*s%*s%*s%*s", LEN_RAIL_P, "now", LEN_RAIL_P, "avg", LEN_RAIL_P, "max", LEN_RAIL_P, "min");
			strcat(output_buff, temp);
			sprintf(temp, " │%s\n", g_vt_clear_line);
			strcat(output_buff, temp);
			display_line(&col_len, NORMAL_LINE, TABLE_RAIL, output_buff);

			for (int i = 0; i < power_val.rail_num; i++)
			{
				sprintf(temp, "│%c│ ", i + 'A');
				strcat(output_buff, temp);
				sprintf(temp, "%-*s │ ", LEN_RAIL_NAME, power_val.rail_infos[i].rail_name);
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
					sprintf(temp, " │ ");
					strcat(output_buff, temp);
				}
				if (available_width > len_middle) {
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_now);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_avg);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_max);
					strcat(output_buff, temp);
					sprintf(temp, "%*.2f", LEN_RAIL_C, power_val.rail_infos[i].c_min);
					strcat(output_buff, temp);
					sprintf(temp, " │ ");
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
				sprintf(temp, " │%s\n", g_vt_clear_line);
				strcat(output_buff, temp);
			}
			display_line(&col_len, LAST_LINE, TABLE_RAIL, output_buff);

			//display group table
			display_line(&col_len, FIRST_LINE, TABLE_GROUP, output_buff);
			sprintf(temp, "│ ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Group Name", LEN_RAIL_NAME / 2);
			strcat(output_buff, temp);
			sprintf(temp, " │ ");
			strcat(output_buff, temp);
			PRINTF_MIDDLE(temp, "Power(mWatt)", LEN_RAIL_P * 2);
			strcat(output_buff, temp);
			sprintf(temp, " │%s\n", g_vt_clear_line);
			strcat(output_buff, temp);
			sprintf(temp, "│ %-*s │ %*s%*s%*s%*s │%s\n", LEN_RAIL_NAME, "",
			LEN_RAIL_P, "now", LEN_RAIL_P, "avg", LEN_RAIL_P, "max", LEN_RAIL_P, "min", g_vt_clear_line);
			strcat(output_buff, temp);
			display_line(&col_len, NORMAL_LINE, TABLE_GROUP, output_buff);
			for (int i = 0; i < power_val.group_num; i++)
			{
				sprintf(temp, "│ %-*s │ ", LEN_RAIL_NAME, power_val.group_infos[i].group_name);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.group_infos[i].p_now);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.group_infos[i].p_avg);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.group_infos[i].p_max);
				strcat(output_buff, temp);
				sprintf(temp, "%*.2f", LEN_RAIL_P, power_val.group_infos[i].p_min);
				strcat(output_buff, temp);
				sprintf(temp, " │%s\n", g_vt_clear_line);
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

		printf("%s", g_vt_home);
		// printf("current lines %d%s\n", available_height, g_vt_clear_line);
		// printf("current columns %d railname: %d%s\n", available_width, LEN_RAIL_NAME, g_vt_clear_line);
		printf("%s", output_buff);
		printf("%s", g_vt_clear_remain);

		usleep(1000 * setting->refreshms);
	}

	ret = bcu_monitor_unperpare(setting);
	if (ret < 0)
	{
		printf("%s", bcu_get_err_str(ret));
		return ret;
	}

	return 0;
}

int lsftdi(struct options_setting *setting)
{
	char boardlist[MAX_NUMBER_OF_BOARD][MAX_MAPPING_NAME_LENGTH] = {0};
	char location_id_str[MAX_NUMBER_OF_USB_DEVICES][MAX_LOCATION_ID_LENGTH] = {0};
	int boardnum = 0;

	boardnum = bcu_lsftdi(setting->auto_find_board, boardlist, location_id_str);
	if (boardnum < 0)
	{
		printf("%s", bcu_get_err_str(boardnum));
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
		printf("%s", bcu_get_err_str(boardnum));
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
		printf("%s", bcu_get_err_str(gpionum));
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
		printf("%s", bcu_get_err_str(bootmodenum));
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

int main(int argc, char **argv)
{
	// if (auto_complete(argc, argv) == 0)
	// 	return 0;

	char* cmd = argv[1];
	struct options_setting setting;
	int ret;
	char yamfile[128] = {0};
	bcu_get_yaml_file_path(yamfile);

	atexit(terminateBCU);

	switch (argc)
	{
	case 1:
		// print_help(NULL);
		return 0;
	case 2: 
		if (strcmp(argv[1], "conf_path") == 0 || strcmp(argv[1], "-cp") == 0)
		{
			printf("\nBCU Config file path: %s\n\n", yamfile);
			return 0;
		}
		break;
	default:
		break;
	}

	ret = opt_parser(argc, argv, &setting, cmd);
	if (ret)
		return ret;

	ret = bcu_get_yaml_file(&setting, yamfile);
	if (ret < 0)
	{
		printf("%s", bcu_get_err_str(ret));
		return ret;
	}

	bcu_update_debug_level(setting.debug);
	if (strlen(setting.location_id_string))
		bcu_update_location_id(setting.location_id_string);

	if (strlen(setting.boot_mode_name))
	{
		ret = bcu_get_boot_mode_hex(&setting);
		if (ret) {
			printf("%s", bcu_get_err_str(ret));
			return ret;
		}
	}
	if (strlen(setting.gpio_name))
	{
		ret = bcu_check_gpio_name(&setting);
		if (ret) {
			printf("%s", bcu_get_err_str(ret));
			return ret;
		}
	}

	if (strcmp(cmd, "gpio") == 0)
	{
		gpio(&setting);
	}
	else if (strcmp(cmd, "bootmode") == 0)
	{
		bootmode(&setting);
	}
	// else if (strcmp(cmd, "help") == 0)
	// {
	// 	if (argc == 2)
	// 		print_help(NULL);
	// 	if (argc == 3)
	// 		print_help(argv[2]);
	// }
	else if (strcmp(cmd, "eeprom") == 0)
	{
		eeprom(&setting);
	}
	else if (strcmp(cmd, "monitor") == 0)
	{
		monitor(&setting);
	}
	// else if (strcmp(cmd, "server") == 0)
	// {
	// 	server_monitor(&setting);
	// }
	else if (strcmp(cmd, "lsftdi") == 0)
	{
		lsftdi(&setting);
	}
	else if (strcmp(cmd, "lsboard") == 0)
	{
		lsboard();
	}
	else if (strcmp(cmd, "lsbootmode") == 0)
	{
		lsbootmode(&setting);
	}
	else if (strcmp(cmd, "lsgpio") == 0)
	{
		lsgpio(&setting);
	}
	else if (strcmp(cmd, "reset") == 0)
	{
		reset(&setting);
	}
	else if (strcmp(cmd, "onoff") == 0)
	{
		onoff(&setting);
	}
	else if (strcmp(cmd, "init") == 0)
	{
		init(&setting);
	}
	else if (strcmp(cmd, "deinit") == 0)
	{
		deinit(&setting);
	}
	// else if (strcmp(cmd, "uuu") == 0)
	// {
	// 	uuu(&setting);
	// }
	// else if (strcmp(cmd, "version") == 0)
	// {
	// 	print_version();
	// }
	// else if (strcmp(cmd, "upgrade") == 0)
	// {
	// 	upgrade_bcu(&setting);
	// }
	else
	{
		printf("%s is a invalid command\n\n", cmd);
		// print_help(NULL);
	}

	return 0;
}