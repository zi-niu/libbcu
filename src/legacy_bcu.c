#include "legacy_bcu.h"
#include "libbcu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*
 * parse the options entered in command line, stores them in the option_setting structure, which use for actual command
 */
extern char GV_LOCATION_ID[];
int legacy_parse_board_id_options(int argc, char** argv, struct options_setting* setting)
{
	//find if the board model is specified first,
	//this way, board dependent setting such as choosing board-specific gpio pin are done correctly
	int argc_count;
	for (argc_count = 2; argc_count < argc; argc_count++)
	{
		//printf("parsing %s\n", argv[argc_count]);
		char* begin = strchr(argv[argc_count], '=');
		char* input = begin + 1;
		if (strncmp(argv[argc_count], "-board=", 7) == 0 && strlen(argv[argc_count]) > 7)
		{
			strcpy(setting->board, input);
			printf("board model is %s\n", setting->board);
			// break;
		}

		if (strncmp(argv[argc_count], "-id=", 4) == 0 && strlen(argv[argc_count]) > 4)
		{
			strcpy(GV_LOCATION_ID, input);
			// printf("location_id is %s\n", GV_LOCATION_ID);
		}

		if (strcmp(argv[argc_count], "-auto") == 0)
		{
			setting->auto_find_board = 1;
			printf("will auto find the board...\n");
		}
	}

	return 0;
}

int legacy_parse_options(char* cmd, int argc, char** argv, struct options_setting* setting)
{
	for (int i = 2; i < argc; i++)
	{
		//printf("parsing %s\n", argv[i]);
		char* begin = strchr(argv[i], '=');
		char* input = begin + 1;

		if (strncmp(argv[i], "-path=", strlen("-path=")) == 0 && strlen(argv[i]) > strlen("-path="))
		{
			strcpy(setting->path, input);
			printf("set customized path as: %s\n", setting->path);
		}
		else if (strncmp(argv[i], "-id=", 4) == 0 && strlen(argv[i]) > 4)
		{
			strcpy(GV_LOCATION_ID, input);
			printf("location_id is %s\n", GV_LOCATION_ID);
		}
		else if (strcmp(argv[i], "-auto") == 0)
		{
			setting->auto_find_board = 1;
			// printf("will auto find the board...\n");
		}
		else if (strncmp(argv[i], "-boothex=", 9) == 0 && strlen(argv[i]) > 9)
		{
			setting->boot_mode_hex = strtol(input, NULL, 16);
		}
		else if (strncmp(argv[i], "-bootbin=", 9) == 0 && strlen(argv[i]) > 9)
		{
			setting->boot_mode_hex = strtol(input, NULL, 2);
		}
		else if (strncmp(argv[i], "-delay=", 7) == 0 && strlen(argv[i]) > 7)
		{
			setting->delay = atoi(input);
			printf("delay is %d\n", setting->delay);
		}
		else if (strncmp(argv[i], "-hold=", 6) == 0 && strlen(argv[i]) > 6)
		{
			setting->hold = atoi(input);
			printf("hold is %d\n", setting->hold);
		}
		else if (strcmp(argv[i], "high") == 0)
		{
			setting->output_state = 1;
			printf("will set gpio high\n");
		}
		else if (strcmp(argv[i], "low") == 0)
		{
			setting->output_state = 0;
			printf("will set gpio low\n");
		}
		else if (strcmp(argv[i], "-toggle") == 0)
		{
			setting->output_state = 2;
			printf("toggle gpio\n");
		}
		else if (strcmp(argv[i], "1") == 0)
		{
			setting->output_state = 1;
			printf("will set gpio high\n");
		}
		else if (strcmp(argv[i], "0") == 0)
		{
			setting->output_state = 0;
			printf("will set gpio low\n");
		}
		else if (strncmp(argv[i], "-dump=", 6) == 0 && strlen(argv[i]) > 6)
		{
			setting->dump = 1;

			setting->dumpname = strdup(input);
			int len1 = strlen(setting->dumpname), len2 = strlen(".csv");
			if (len1 == 0)
			{
				free(setting->dumpname);
				setting->dumpname = strdup("monitor_record.csv");
			}
			else
			{
				if (strcmp(setting->dumpname + len1 - len2, ".csv"))
				{
					setting->dumpname = realloc(setting->dumpname, strlen(setting->dumpname) + strlen(".csv") + 1);
					strcat(setting->dumpname, ".csv");
				}
			}
			printf("dump data into %s file\n", setting->dumpname);
		}
		else if (strcmp(argv[i], "-dump") == 0)
		{
			setting->dump = 1;
			setting->dumpname = strdup("monitor_record.csv");
			printf("dump data into %s file\n", setting->dumpname);
		}
		else if (strcmp(argv[i], "-pmt") == 0)
		{
			setting->pmt = 1;
		}
		else if (strcmp(argv[i], "-nodisplay") == 0)
		{
			setting->nodisplay = 1;
			if (!setting->dump)
			{
				setting->dump = 1;
				setting->dumpname = strdup("monitor_record.csv");
				printf("dump data into %s file\n", setting->dumpname);
			}
		}
		else if (strncmp(argv[i], "-hz=", 4) == 0 && strlen(argv[i]) > 4)
		{
			double hz = atof(input);
			setting->refreshms = (int)(1000.0 / hz);
		}
		else if (strcmp(argv[i], "-rms") == 0)
		{
			setting->use_rms = 1;
		}
		else if (strcmp(argv[i], "-hwfilter") == 0)
		{
			setting->use_hwfilter = 1;
		}
		else if (strcmp(argv[i], "-unipolar") == 0)
		{
			setting->use_unipolar = 1;
		}
		else if (strcmp(argv[i], "-temp") == 0)
		{
			// TODO
			//setting->temperature = 1;
		}
		else if (strcmp(argv[i], "-stats") == 0)
		{
			setting->dump_statistics = 1;
		}
		else if (strcmp(argv[i], "-erase") == 0)
		{
			setting->eeprom_function = PARSER_EEPROM_ERASE;
		}
		else if (strcmp(argv[i], "-f") == 0)
		{
			setting->force = 1;
		}
		else if (strcmp(argv[i], "-w") == 0)
		{
			setting->eeprom_function = PARSER_EEPROM_WRITE_DEFAULT;
		}
		else if (strcmp(argv[i], "-r") == 0)
		{
			setting->eeprom_function = PARSER_EEPROM_READ_AND_PRINT;
		}
		else if (strcmp(argv[i], "-wf") == 0)
		{
			setting->eeprom_function = PARSER_EEPROM_WRITE_FROM_FILE;
		}
		else if (strcmp(argv[i], "-rf") == 0)
		{
			setting->eeprom_function = PARSER_EEPROM_READ_TO_FILE;
		}
		else if (strncmp(argv[i], "-wsn=", 5) == 0 && strlen(argv[i]) > 5)
		{
			setting->eeprom_function = PARSER_EEPROM_UPDATE_USER_SN;

			setting->eeprom_usr_sn = atoi(input);
			printf("eeprom user SN will be set to %d\n", setting->eeprom_usr_sn);
		}
		else if (strncmp(argv[i], "-sn=", 4) == 0 && strlen(argv[i]) > 4)
		{
			setting->eeprom_usr_sn = atoi(input);
			printf("eeprom user SN will be set to %d\n", setting->eeprom_usr_sn);
		}
		else if (strncmp(argv[i], "-brev=", 6) == 0 && strlen(argv[i]) > 6)
		{
			strcpy(setting->eeprom_board_rev, input);
			printf("eeprom user board revision will be set to %s\n", setting->eeprom_board_rev);
		}
		else if (strncmp(argv[i], "-srev=", 6) == 0 && strlen(argv[i]) > 6)
		{
			strcpy(setting->eeprom_soc_rev, input);
			printf("eeprom user soc revision will be set to %s\n", setting->eeprom_soc_rev);
		}
		else if (strcmp(argv[i], "-doc") == 0)
		{
			setting->download_doc = 1;
		}
		else if (strcmp(argv[i], "-pre") == 0)
		{
			//TODO
			//setting->download_pre_release = 1;
		}
		else if (strcmp(argv[i], "-restore") == 0)
		{
			//TODO
			//setting->restore = 1;
		}
		else if (strncmp(argv[i], "-board=", 7) == 0 && strlen(argv[i]) > 7)
		{
			strcpy(setting->board, input);
		}
		else if (strncmp(argv[i], "-bootmode=", 10) == 0 && strlen(argv[i]) > 10)
		{
			strcpy(setting->boot_mode_name, input);
		}
		else
		{
			if (bcu_get_bootmode(setting, argv[i])) {
				printf("could not recognize input %s\n", argv[i]);
				return -1;
			}
			/*if (board == NULL)
				return -1;
			int j = 0;
			int found = 0;
			while (board->mappings[j].name != NULL)
			{
				if (strcmp(board->mappings[j].name, argv[i]) == 0)
				{
					found = 1;
					strcpy(setting->gpio_name, argv[i]);
					break;
				}
				j++;
			}
			int k = 0;
			if (board->boot_modes != NULL)
			{
				while (board->boot_modes[k].name != NULL)
				{
					if (strcmp(board->boot_modes[k].name, argv[i]) == 0)
					{
						found = 1;
						setting->boot_mode_hex = board->boot_modes[k].boot_mode_hex;
						break;
					}
					k++;
				}
			}
			k = 0;
			if (board->boot_configs != NULL)
			{
				while (board->boot_configs[k].name != NULL)
				{
					if (strcmp(board->boot_configs[k].name, argv[i]) == 0)
					{
						found = 1;
						for (int bootcfg_n = 0; bootcfg_n < MAX_BOOT_CONFIG_BYTE; bootcfg_n++)
						{
							setting->boot_config_hex[bootcfg_n] = board->boot_configs[k].boot_config_hex[bootcfg_n];
						}
						break;
					}
					k++;
				}
			}
			if (!found)
			{
				printf("could not recognize input %s\n", argv[i]);
				return -1;
			}*/
		}
	}
	return 0;
}