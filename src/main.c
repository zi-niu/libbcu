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
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#include "options.h"
#include "libbcu.h"
#include "libbcu_version.h"

int GV_MONITOR_TERMINATED = 0;

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

int monitor(struct options_setting *setting)
{
	signal(SIGINT, handle_sigint);
	int ret;
	powers power_val;

	memset(&power_val, 0, sizeof(power_val));

	ret = bcu_monitor_perpare(setting);
	if (ret < 0)
	{
		printf("%s", bcu_get_err_str(ret));
		return ret;
	}

	while(!GV_MONITOR_TERMINATED && !(bcu_monitor_is_stop())){
		ret = bcu_monitor_getvalue(setting, &power_val);
		if (ret)
			printf("%s", bcu_get_err_str(ret));
		// printf("val: %f\n", power_val.rail_infos[0].c_avg);
		printf("%s", g_vt_home);
		printf("-----------------------------------------------------------------------------------------------------------------------------------------------%s\n", g_vt_clear_line);
		printf("%-30s | %6s%6s%6s%6s | %10s%10s%10s%10s | %10s%10s%10s%10s%s\n", "Rail Name",
		       "now", "avg", "max", "min",
		       "now", "avg", "max", "min",
		       "now", "avg", "max", "min", g_vt_clear_remain);
		printf("-----------------------------------------------------------------------------------------------------------------------------------------------%s\n", g_vt_clear_line);
		for (int i = 0; i < power_val.rail_num; i++)
		{
			printf("%-30s | ", power_val.rail_infos[i].rail_name);
			printf("%6.2f", power_val.rail_infos[i].v_now);
			printf("%6.2f", power_val.rail_infos[i].v_avg);
			printf("%6.2f", power_val.rail_infos[i].v_max);
			printf("%6.2f", power_val.rail_infos[i].v_min);
			printf(" | ");
			printf("%10.2f", power_val.rail_infos[i].c_now);
			printf("%10.2f", power_val.rail_infos[i].c_avg);
			printf("%10.2f", power_val.rail_infos[i].c_max);
			printf("%10.2f", power_val.rail_infos[i].c_min);
			printf(" | ");
			printf("%10.2f", power_val.rail_infos[i].p_now);
			printf("%10.2f", power_val.rail_infos[i].p_avg);
			printf("%10.2f", power_val.rail_infos[i].p_max);
			printf("%10.2f", power_val.rail_infos[i].p_min);
			printf("%s\n", g_vt_clear_line);
		}
		printf("-----------------------------------------------------------------------------------------------------------------------------------------------%s\n", g_vt_clear_line);
		printf("%-30s | %10s%10s%10s%10s%s\n", "Group Name",
		       "now", "avg", "max", "min", g_vt_clear_remain);
		for (int i = 0; i < power_val.group_num; i++)
		{
			printf("%-30s | ", power_val.group_infos[i].group_name);
			printf("%10.2f", power_val.group_infos[i].p_now);
			printf("%10.2f", power_val.group_infos[i].p_avg);
			printf("%10.2f", power_val.group_infos[i].p_max);
			printf("%10.2f", power_val.group_infos[i].p_min);
			printf("%s\n", g_vt_clear_line);
		}
		printf("-----------------------------------------------------------------------------------------------------------------------------------------------%s\n", g_vt_clear_line);


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