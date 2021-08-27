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

#ifdef _WIN32
#include "getopt.h"
#else
#include <getopt.h>
#endif
#include <string.h>
#include <stdlib.h>
#include "options.h"
#include "error_def.h"

int lopt;
const char *optstring = "m:ab:i:d::nwrg:vhc";
const char *bcu_param[] = { "-m", "-a", "-b", "-i", "-d", "-n",
			    "-w", "-r", "-g", "-v", "-h", "-c", NULL };

enum options
{
	op_bootmode = 1,
	op_auto,
	op_board,
	op_id,
	op_hold,
	op_boothex,
	op_bootbin,
	op_dump,
	op_nodisplay,
	op_pmt,
	op_stats,
	op_rms,
	op_hz,
	op_hwfilter,
	op_unipolar,
	op_write,
	op_write_sn,
	op_set_sn,
	op_set_brev,
	op_set_srev,
	op_read,
	op_erase,
	op_gpioname,
	op_set,
	op_get,
	op_version,
	op_help,
	op_config_path,
	op_debug
};

const struct option long_options[] =
{
	{"bootmode",	required_argument,	&lopt, op_bootmode	},  //short: m
	{"auto",	no_argument,		&lopt, op_auto		},  //short: a
	{"board",	required_argument,	&lopt, op_board		},  //short: b
	{"id",		required_argument,	&lopt, op_id		},  //short: i
	{"hold",	required_argument,	&lopt, op_hold		},
	{"boothex",	required_argument,	&lopt, op_boothex	},
	{"bootbin",	required_argument,	&lopt, op_bootbin	},
	{"dump",	optional_argument,	&lopt, op_dump		},  //short: d
	{"nodisplay",	no_argument,		&lopt, op_nodisplay	},  //short: n
	{"pmt",		no_argument,		&lopt, op_pmt		},
	{"stats",	no_argument,		&lopt, op_stats		},
	{"rms",		no_argument,		&lopt, op_rms		},
	{"hz",		required_argument,	&lopt, op_hz		},
	{"hwfilter",	no_argument,		&lopt, op_hwfilter	},
	{"unipolar",	no_argument,		&lopt, op_unipolar	},
	{"write",	no_argument,		&lopt, op_write		},  //short: w
	{"write_sn",	required_argument,	&lopt, op_write_sn	},
	{"set_sn",	required_argument,	&lopt, op_set_sn	},
	{"set_brev",	required_argument,	&lopt, op_set_brev	},
	{"set_srev",	required_argument,	&lopt, op_set_srev	},
	{"read",	no_argument,		&lopt, op_read		},  //short: r
	{"erase",	no_argument,		&lopt, op_erase		},
	{"gpioname",	required_argument,	&lopt, op_gpioname	},  //short: g
	{"set",		optional_argument,	&lopt, op_set		},
	{"get",		no_argument,		&lopt, op_get		},
	{"version",	no_argument,		&lopt, op_version	},  //short: v
	{"help",	no_argument,		&lopt, op_help		},  //short: h
	{"config_path",	no_argument,		&lopt, op_config_path	},  //short: c
	{"debug",	required_argument,	&lopt, op_debug		},
	{0, 0, 0, 0}
};

const struct command_opt bcu_commands[] =
{
	{"reset",		{op_bootmode, op_auto, op_board, op_id, op_hold, op_boothex, op_bootbin, op_debug}},
	{"onoff",		{op_auto, op_board, op_id, op_hold, op_debug}},
	{"init",		{op_bootmode, op_auto, op_board, op_id, op_debug}},
	{"deinit",		{op_bootmode, op_auto, op_board, op_id, op_debug}},
	{"monitor",		{op_auto, op_board, op_id, op_dump, op_nodisplay, op_pmt, op_stats, op_rms, op_hz, op_hwfilter, op_unipolar, op_debug}},
	{"eeprom",		{op_board, op_id, op_write, op_write_sn, op_set_sn, op_set_brev, op_set_srev, op_read, op_erase, op_debug}},
	{"gpio",		{op_auto, op_board, op_id, op_gpioname, op_set, op_get, op_debug}},
	{"bootmode",		{op_bootmode, op_auto, op_board, op_id, op_boothex, op_bootbin, op_set, op_get, op_debug}},
	{"lsftdi",		{op_auto, op_debug}},
	{"lsboard",		{0}},
	{"lsbootmode",		{op_auto, op_board, op_debug}},
	{"lsgpio",		{op_auto, op_board, op_debug}},
	{NULL,			{0}}
};

int is_opt_supported(char *command_str, int opt)
{
	int ret = -1;
	int i = 0, j = 0;

	while (bcu_commands[i].cmd_name)
	{
		if (!strcmp(bcu_commands[i].cmd_name, command_str))
		{
			while (bcu_commands[i].option_val[j])
			{
				if (bcu_commands[i].option_val[j] == opt)
				{
					ret = 0;
					return ret;
				}
				j++;
			}
		}
		i++;
	}
	return ret;
}

void set_options_default(struct options_setting *setting)
{
	setting->debug = 1;
	strcpy(setting->board, "");
	setting->auto_find_board = 0;
	setting->path[0] = '\0';
	setting->location_id_string[0] = '\0';
	setting->output_state = -1;
	setting->delay = 0;
	setting->hold = 0;
	setting->boot_mode_name[0] = '\0';
	setting->boot_mode_hex = -1;
	for (unsigned int i = 0; i < MAX_BOOT_CONFIG_BYTE; i++)
		setting->boot_config_hex[i] = -1;
	setting->get_or_set = -1;
	setting->gpio_name[0] = '\0';
	setting->dump = 0;
	setting->force = 0;
	setting->pmt = 0;
	setting->nodisplay = 0;
	setting->dumpname = NULL;
	setting->refreshms = 200;
	setting->use_rms = 0;
	setting->rangefixed = 0;
	setting->use_hwfilter = 0;
	setting->use_unipolar = 0;
	setting->dump_statistics = 0;
	setting->eeprom_function = 0;
	setting->download_doc = 0;
}

int opt_parser(int argc, char **argv, struct options_setting *setting, char *cmd)
{
	int opt, loidx;

	set_options_default(setting);

	while((opt = getopt_long(argc, argv, optstring, long_options, &loidx)) != -1)
	{
		if (opt == 0)
			opt = lopt;

		// printf("opt_parser: opt: %d\n", opt);

		switch(opt)
		{
		case 'm':
		case op_bootmode:
			opt = op_bootmode;
			strcpy(setting->boot_mode_name, optarg);
			break;
		case 'a':
		case op_auto:
			opt = op_auto;
			setting->auto_find_board = 1;
			break;
		case 'b':
		case op_board:
			opt = op_board;
			strcpy(setting->board, optarg);
			break;
		case 'i':
		case op_id:
			opt = op_id;
			strcpy(setting->location_id_string, optarg);
			printf("location_id is %s\n", setting->location_id_string);
			break;
		case op_hold:
			setting->hold = atoi(optarg);
			printf("hold is %d\n", setting->hold);
			break;
		case op_boothex:
			setting->boot_mode_hex = strtol(optarg, NULL, 16);
			break;
		case op_bootbin:
			setting->boot_mode_hex = strtol(optarg, NULL, 2);
			break;
		case 'd':
		case op_dump:
			opt = op_dump;
			setting->dump = 1;
			if (optarg)
			{
				if (!setting->dumpname)
					free(setting->dumpname);

				if (strlen(optarg) == 0)
				{
					setting->dumpname = strdup("monitor_record.csv");
					break;
				}
				setting->dumpname = strdup(optarg);
				int len1 = strlen(setting->dumpname), len2 = strlen(".csv");
				if(strcmp(setting->dumpname + len1 - len2, ".csv"))
				{
					setting->dumpname = realloc(setting->dumpname, strlen(setting->dumpname) + strlen(".csv") + 1);
					strcat(setting->dumpname, ".csv");
				}
			}
			else
				setting->dumpname = strdup("monitor_record.csv");
			break;
		case 'n':
		case op_nodisplay:
			opt = op_nodisplay;
			setting->nodisplay = 1;
			if (!setting->dump)
			{
				setting->dump = 1;
				setting->dumpname = strdup("monitor_record.csv");
				printf("dump data into %s file\n", setting->dumpname);
			}
			break;
		case op_pmt:
			setting->pmt = 1;
			break;
		case op_stats:
			setting->dump_statistics = 1;
			break;
		case op_rms:
			setting->use_rms = 1;
			break;
		case op_hz:
		{
			double hz = atof(optarg);
			setting->refreshms = (int)(1000.0 / hz);
		}break;
		case op_hwfilter:
			setting->use_hwfilter = 1;
			break;
		case op_unipolar:
			setting->use_unipolar = 1;
			break;
		case 'w':
		case op_write:
			opt = op_write;
			setting->eeprom_function = PARSER_EEPROM_WRITE_DEFAULT;
			break;
		case op_write_sn:
			setting->eeprom_function = PARSER_EEPROM_UPDATE_USER_SN;
			setting->eeprom_usr_sn = atoi(optarg);
			printf("eeprom user SN will be set to %d\n", setting->eeprom_usr_sn);
			break;
		case op_set_sn:
			setting->eeprom_usr_sn = atoi(optarg);
			printf("eeprom user SN will be set to %d\n", setting->eeprom_usr_sn);
			break;
		case op_set_brev:
			strcpy(setting->eeprom_board_rev, optarg);
			printf("eeprom user board revision will be set to %s\n", setting->eeprom_board_rev);
			break;
		case op_set_srev:
			strcpy(setting->eeprom_soc_rev, optarg);
			printf("eeprom user soc revision will be set to %s\n", setting->eeprom_soc_rev);
			break;
		case 'r':
		case op_read:
			opt = op_read;
			setting->eeprom_function = PARSER_EEPROM_READ_AND_PRINT;
			break;
		case op_erase:
			setting->eeprom_function = PARSER_EEPROM_ERASE;
			break;
		case 'g':
		case op_gpioname:
			opt = op_gpioname;
			strcpy(setting->gpio_name, optarg);
			break;
		case op_set:
			setting->get_or_set = 1;
			if (optarg)
				setting->output_state = atoi(optarg);
			break;
		case op_get:
			setting->get_or_set = 2;
			break;
		case 'v':
		case op_version:
			opt = op_version;
			break;
		case 'h':
		case op_help:
			opt = op_help;
			break;
		case 'c':
		case op_config_path:
			opt = op_config_path;
			break;
		case op_debug:
			setting->debug = atoi(optarg);
			break;
		default:
			printf("Error option!\n");
			return ERR_OPT;
		}

		if (is_opt_supported(cmd, opt))
		{
			printf("Command [%s] does not support option [%s]\n", cmd, long_options[opt - 1].name);
			return ERR_OPT;
		}
	}

	return 0;
}



















































#define linux_auto_argd() linux_auto_arg(" ", "")

void linux_auto_arg(const char *space, const char *filter)
{
	// char *str = filter;

	const char *param[] = { "-b", "-d", "-v", "-V", "-s", NULL };
	int i = 0;

	for (int i = 0; param[i]; i++)
	{
		printf("%s%s", param[i], space);
	}
}

int linux_autocomplete(int argc, char **argv)
{
	char *last;
	char *cur;

	// printf("argc:%d", argc);

	if (argc == 3)
	{
		last = argv[2];
	}else
	{
		cur = argv[2];
		last = argv[3];
	}

	// if (cur[0] == '-')
	// {
		// if (strlen(cur) == 1)
			linux_auto_argd();
		// else
		// 	cout << cur << " " << endl;

		return 0;
	// }

	// if (last.size()>=3)
	// {
	// 	if (last.substr(last.size() - 3) == "uuu" &&(cur.empty() || cur[0] == '-'))
	// 	{
	// 		linux_auto_arg();
	// 		cout << cur << endl;
	// 	}

	// }else if(last.empty())
	// {
	// 	linux_auto_arg();
	// }
	// else if (last == "-b")
	// {
	// 	return g_BuildScripts.PrintAutoComplete(cur);

	// }else if(last[0] == '-')
	// {
	// 	linux_auto_arg();
	// }

	// uuu_for_each_ls_file(linux_autocomplete_ls, cur.c_str(), NULL);
}

int auto_complete(int argc, char **argv)
{
	if (linux_autocomplete(argc, argv) == 0)
		return 0;

	return 1;
}