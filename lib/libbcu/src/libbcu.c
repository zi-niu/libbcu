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
#define _CRT_SECURE_NO_WARNINGS //in order to use strcpy without error
#include <windows.h>
#include <processthreadsapi.h>
#include <process.h>
#endif

#if defined(linux) || defined(__APPLE__)
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <curses.h>
#include <pthread.h>
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
#include "libbcu_version.h"
#include "yml.h"
#include "eeprom.h"
#include "board.h"
#include "board.c"

int GV_DEBUG_LEVEL = 1;

int check_board_eeprom(struct board_info *board, int retmode)
{
	void* head = NULL;
	void* end_point;
	int status, j;

	j = 0;
	while(board->mappings[j].name!=NULL)
	{
		if(board->mappings[j].type == i2c_eeprom || board->mappings[j].type == ftdi_eeprom)
		{
			end_point = build_device_linkedlist_forward(&head, board->mappings[j].path);
			if (end_point == NULL)
				return -LIBBCU_ERR_BUILD_DEVICE_LINK;

			struct eeprom_device* eeprom = end_point;
			status = bcu_eeprom_checkboard(eeprom, board->eeprom_data);
			if (board->mappings[j].type == i2c_eeprom)
				free_device_linkedlist_backward(end_point);
			ft4232h_i2c_remove_all();

			if (status == 0)
				return 0;
			else
			{
				j++;
				continue;
			}
		}
		j++;
	}

	if (retmode)
		return status;
	return -1;
}

int find_board_by_eeprom(char *boardname, int *boardnum)
{
	int status, i;

	for (i = 0; i < num_of_boards; i++)
	{
		strcpy(boardname, board_list[i].name);
		*boardnum = i;
		struct board_info *board = get_board_by_id(i);
		status = check_board_eeprom(board, 0);
		if (status != -1)
			return status;
	}

	return -1;
}

char *bcu_get_err_str(int err_num)
{
	if (err_num)
		mprintf(2, "\nraw error code: %d\n", err_num);
	switch (err_num)
	{
	case 0:
		return "Success!\n";
	case -LIBBCU_ERR_NO_THIS_BOARD:
		return "\n<ERROR------cut------------------"
		       "\nnot supported board model or missing option <--board=>\n\n"
		       "Or use option <--auto> to find the board automatically\n"
		       "NOTE: if other boards are also connected to the same host, <--auto> may break its ttyUSB function temporarily.\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_BUILD_DEVICE_LINK:
		return "\n<ERROR------cut------------------"
		       "\nBuilding device linked list failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_OPEN_FTDI_CHANNEL:
		return "\n<ERROR------cut------------------"
		       "\nCan't open FTDI channel!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_BOOT_MODE_OPT:
		return "\n<ERROR------cut------------------"
		       "\nNo boot mode provided!\n"
		       "Please add <-m> or <--bootmode=> option.\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_BOOT_MODE_CFG:
		return "\n<ERROR------cut------------------"
		       "\nNo boot_mode configuration!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_BOOT_CFG_CFG:
		return "\n<ERROR------cut------------------"
		       "\nNo boot_cfg configuration!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_BOOT_MODE_OFFSET:
		return "\n<ERROR------cut------------------"
		       "\nSomething is wrong with the pin bitmask!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_SET_GPIO_FAIL:
		return "\n<ERROR------cut------------------"
		       "\nSet GPIO failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_SET_BOOTMODE_FAIL:
		return "\n<ERROR------cut------------------"
		       "\nSet boot mode failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_INIT_FAIL:
		return "\n<ERROR------cut------------------"
		       "\nBCU init failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_RESET_FAIL:
		return "\n<ERROR------cut------------------"
		       "\nBCU reset failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_ONOFF_FAIL:
		return "\n<ERROR------cut------------------"
		       "\nBCU onoff executed failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_BOOT_FROM_SWITCH:
		return "\n<ERROR------cut------------------"
		       "\nPin bootmode_sel is disabled, boot from BOOT SWITCH!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_REMOTE_EN:
		return "\n<ERROR------cut------------------"
		       "\nCannot find gpio remote_en!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_GET_BOOT_CFG:
		return "\n<ERROR------cut------------------"
		       "\nBCU get boot config failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_GET_BOOT_MODE:
		return "\n<ERROR------cut------------------"
		       "\nBCU get boot mode failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_GET_OR_SET:
		return "\n<ERROR------cut------------------"
		       "\nMissing option: <--get> or <--set>/<--set=>!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_NO_SET_VAL_OPT:
		return "\n<ERROR------cut------------------"
		       "\nMissing option: <--set=>\n"
		       "Please enter a valid output state, 1 to set logic level high, 0 to set it low.\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_INVALID_GPIO_NAME_OPT:
		return "\n<ERROR------cut------------------\n"
			"Could not detect a valid gpio name entered.\n"
			"Please enter the name of the gpio pin by using option <-g>/<--gpioname=>.\n"
			"To check a list of available gpio pin, please use command:\n  ./bcu lsgpio --board=xxx\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_FIND_GPIO_PATH:
		return "\n<ERROR------cut------------------"
		       "\nMissing option: <--get> or <--set>/<--set=>!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_GET_GPIO_LEVEL:
		return "\n<ERROR------cut------------------"
		       "\nBCU get gpio failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_READ:
		return "\n<ERROR------cut------------------"
		       "\nBCU read EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_WRITE_DEFAULT:
		return "\n<ERROR------cut------------------"
		       "\nBCU write default value to EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_UPDATE_USRSN:
		return "\n<ERROR------cut------------------"
		       "\nBCU update user SN number to EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_UPDATE_BREV:
		return "\n<ERROR------cut------------------"
		       "\nBCU update board revison to EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_UPDATE_SREV:
		return "\n<ERROR------cut------------------"
		       "\nBCU update soc revison to EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_ERASE:
		return "\n<ERROR------cut------------------"
		       "\nBCU erase EEPROM failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_EEPROM_NO_OPT:
		return "\n<ERROR------cut------------------"
		       "\nMissing option: <-w>/<--write> or <-r>/<read>!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_UNSPPORTED_GPIO:
		return "\n<ERROR------cut------------------"
		       "\nThe GPIO name is unsupported!\n"
		       "Please double check the GPIO name with command:\n"
		       "./bcu lsgpio --board=xxx\n"
		       "------------end------------------>\n\n";
	
	
	case -LIBBCU_ERR_MUTEX_LOCK_INIT:
		return "\n<ERROR------cut------------------"
		       "\nMutex lock init failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_MOT_OPEN_FILE:
		return "\n<ERROR------cut------------------"
		       "\nMonitor: Cannot open dump file!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_MOT_GROUP_PARSE:
		return "\n<ERROR------cut------------------"
		       "\nMonitor: Group parse failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_MOT_PAC1934_CANNOT_ACCESS:
		return "\n<ERROR------cut------------------"
		       "\nMonitor: PAC1934 cannot access!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_MOT_NO_SR:
		return "\n<ERROR------cut------------------"
		       "\nMonitor: SR name not found!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_YML_OPEN_FILE:
		return "\n<ERROR------cut------------------"
		       "\nYAML: Cannot open yaml file!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_ERR_YML_PARSER:
		return "\n<ERROR------cut------------------"
		       "\nYAML: Yaml file parse failed!\n"
		       "------------end------------------>\n\n";
	case -LIBBCU_WARN_YML_OLD:
		return "\n WARNNING: YAML: Yaml file too old!\n\n";
	case -LIBBCU_ERR_YML_READ:
		return "\n<ERROR------cut------------------"
		       "\nYAML: Read yaml file failed!\n"
		       "------------end------------------>\n\n";
	default:
		return "\n<ERROR------cut------------------"
		       "\nNot handled error!\n"
		       "------------end------------------>\n\n";
	}
}

struct gpio_device* get_gpio(char* gpio_name, struct board_info* board)
{
	int i = 0;
	char path[MAX_PATH_LENGTH];
	void* head = NULL;
	void* end_point;
	struct gpio_device* gpio = NULL;

	if (get_path(path, gpio_name, board) == -1)
		return NULL;
	end_point = build_device_linkedlist_forward(&head, path);
	if (end_point == NULL)
	{
		mprintf(2, "get_gpio: error building device linked list\n");
		return NULL;
	}
	gpio = end_point;

	return gpio;
}

int get_gpio_id(char* gpio_name, struct board_info* board)
{
	int id = 0;
	char path[MAX_PATH_LENGTH];

	id = get_path(path, gpio_name, board);

	return id;
}

void free_gpio(struct gpio_device* gpio)
{
	void* end_point = (void*)gpio;
	free_device_linkedlist_backward(end_point);
	gpio = NULL;
	return;
}

int set_gpio_level(struct options_setting* setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	void* head = NULL;
	void* end_point;
	char path[MAX_PATH_LENGTH];
	int status = -1;

	if (setting->output_state == -1)
	{
		mprintf(2, "please enter a valid output state, 1 to set logic level high, 0 to set it low\n");
		return -LIBBCU_ERR_NO_SET_VAL_OPT;
	}

	if (strlen(setting->path) == 0)
	{
		if (strlen(setting->gpio_name) == 0)
		{
			mprintf(2, "could not detect a valid gpio name entered\n");
			mprintf(2, "please enter the name of the gpio pin,\n");
			mprintf(2, "to see a list of available gpio pin, please use command lsgpio\n");
			return -LIBBCU_ERR_INVALID_GPIO_NAME_OPT;
		}

		if (get_path(path, setting->gpio_name, board) == -1)
		{
			mprintf(2, "failed to find gpio path\n");
			return -LIBBCU_ERR_FIND_GPIO_PATH;
		}
		end_point = build_device_linkedlist_forward(&head, path);
	}
	else
		end_point = build_device_linkedlist_forward(&head, setting->path);

	if (end_point == NULL)
	{
		mprintf(2, "set_gpio: error building device linked list\n");
		return -LIBBCU_ERR_BUILD_DEVICE_LINK;
	}

	//delay
	msleep(setting->delay);

	struct gpio_device* gpio = end_point;
	if (setting->output_state == 1)
		status = gpio->gpio_write(gpio, 0xFF);
	else if (setting->output_state == 0)
		status = gpio->gpio_write(gpio, 0x00);
	else if (setting->output_state == 2)
		status = gpio->gpio_toggle(gpio);

	if (status) {
		mprintf(2, "set gpio failed, error = 0x%x\n", status);
		return -LIBBCU_ERR_SET_GPIO_FAIL;
	} else
		mprintf(3, "set gpio successfully\n");

	//hold time
	msleep(setting->hold);

	free_device_linkedlist_backward(end_point);

	return 0;
}

int get_gpio_level(struct options_setting* setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	void* head = NULL;
	void* end_point;
	char path[MAX_PATH_LENGTH];
	int status = -1;
	unsigned char buff = 0;

	if (strlen(setting->path) == 0)
	{
		if (strlen(setting->gpio_name) == 0)
		{
			mprintf(2, "could not detect a valid gpio name entered\n");
			mprintf(2, "please enter the name of the gpio pin,\n");
			mprintf(2, "to see a list of available gpio pin, please use command lsgpio\n");
			return -LIBBCU_ERR_INVALID_GPIO_NAME_OPT;

		}
		if (get_path(path, setting->gpio_name, board) == -1) {
			mprintf(2, "failed to find gpio path\n");
			return -LIBBCU_ERR_FIND_GPIO_PATH;
		}
		end_point = build_device_linkedlist_forward(&head, path);
	}
	else
		end_point = build_device_linkedlist_forward(&head, setting->path);

	if (end_point == NULL)
	{
		mprintf(2, "get_gpio_level: error building device linked list\n");
		return -LIBBCU_ERR_BUILD_DEVICE_LINK;
	}

	struct gpio_device* gpio = end_point;

	status = gpio->gpio_read(gpio, &buff);

	if (status) {
		mprintf(2, "get gpio failed, error = 0x%x\n", status);
		return -LIBBCU_ERR_GET_GPIO_LEVEL;
	} else {
		mprintf(3, "get %s level=%s\n", setting->gpio_name, buff ? "HIGH" : "LOW");
		status = buff ? 1 : 0;
	}

	free_device_linkedlist_backward(end_point);

	return status;
}

int set_boot_config(struct options_setting* setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;

	for (int config_num = 0; config_num < board->boot_cfg_byte_num; config_num++)
	{
		if (setting->boot_config_hex[config_num] == -1)
		{
			mprintf(2, "could not detect a valid boot_config_hex_%d!\n", config_num);
			mprintf(2, "set_boot_config failed\n");
			return -LIBBCU_ERR_NO_BOOT_MODE_OPT;
		}
		char cfg_str[10] = "boot_cfg";
		char num_str[2] = "0";
		num_str[0] += config_num;
		strcat(cfg_str, num_str);
		gpio = get_gpio(cfg_str, board);
		if (gpio == NULL)
		{
			mprintf(2, "set_boot_config: No boot_cfg%d configuration!\n", config_num);
			return -LIBBCU_ERR_NO_BOOT_CFG_CFG;
		}

		if (get_boot_mode_offset(gpio->pin_bitmask) < 0)
		{
			free_gpio(gpio);
			return -LIBBCU_ERR_BOOT_MODE_OFFSET;
		}

		unsigned char hex_with_offset = setting->boot_config_hex[config_num] << (get_boot_mode_offset(gpio->pin_bitmask));
		status = gpio->gpio_write(gpio, hex_with_offset);

		if (status) {
			mprintf(2, "set boot config %d failed, error = 0x%x\n", config_num, status);
			return -LIBBCU_ERR_SET_BOOTMODE_FAIL;
		} else
			mprintf(3, "set boot config %d successfully\n", config_num);

		free_gpio(gpio);
	}

	return 0;
}

int set_boot_mode(struct options_setting* setting)
{
	if (setting->boot_mode_hex == -1)
	{
		mprintf(2, "could not detect a valid boot_mode,\nplease entered a valid boot mode\n");
		mprintf(2, "set_boot_mode failed\n");
		return -LIBBCU_ERR_NO_BOOT_MODE_OPT;
	}
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;

	gpio = get_gpio("boot_mode", board);
	if (gpio == NULL)
	{
		mprintf(2, "set_boot_mode: No boot_mode configuration!\n");
		return -LIBBCU_ERR_NO_BOOT_MODE_CFG;
	}

	if (get_boot_mode_offset(gpio->pin_bitmask) < 0)
	{
		free_gpio(gpio);
		return -LIBBCU_ERR_BOOT_MODE_OFFSET;
	}

	unsigned char hex_with_offset = setting->boot_mode_hex << (get_boot_mode_offset(gpio->pin_bitmask));
	status = gpio->gpio_write(gpio, hex_with_offset);

	if (status) {
		mprintf(2, "set boot mode failed, error = 0x%x\n", status);
		free_gpio(gpio);
		return -LIBBCU_ERR_SET_BOOTMODE_FAIL;
	} else
		mprintf(3, "set boot mode successfully\n");

	free_gpio(gpio);

	if (board->boot_cfg_byte_num > 0)
		return set_boot_config(setting);

	return 0;
}

int get_boot_config(struct options_setting* setting, unsigned char boot_modehex, int *bootcfglen)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;
	unsigned char read_buf;
	int read_boot_config_hex[MAX_BOOT_CONFIG_BYTE] = { 0 };

	for (int config_num = 0; config_num < board->boot_cfg_byte_num; config_num++)
	{
		char cfg_str[10] = "boot_cfg";
		char num_str[2] = "0";
		num_str[0] += config_num;
		strcat(cfg_str, num_str);

		gpio = get_gpio(cfg_str, board);
		if (gpio == NULL)
		{
			mprintf(2, "get_boot_config: No boot_cfg configuration!\n");
			free_gpio(gpio);
			return -LIBBCU_ERR_NO_BOOT_CFG_CFG;
		}
		if (get_boot_mode_offset(gpio->pin_bitmask) < 0)
		{
			free_gpio(gpio);
			return -LIBBCU_ERR_BOOT_MODE_OFFSET;
		}
		status = gpio->gpio_read(gpio, &read_buf);
		read_buf = read_buf >> get_boot_mode_offset(gpio->pin_bitmask);
		if (status) {
			mprintf(2, "get_boot_config %d failed, error = 0x%x\n", config_num, status);
		} else
			read_boot_config_hex[config_num] = read_buf;

		free_gpio(gpio);
	}

	char *bootmodestr = get_boot_config_name_from_hex(board, read_boot_config_hex, boot_modehex);
	if (bootmodestr == NULL) {
		mprintf(2, "get_boot_config: cannot find the boot config string.\n");
		return -LIBBCU_ERR_GET_BOOT_CFG;
	} else {
		mprintf(3, "get_boot_mode: %s, ", bootmodestr);
		mprintf(3, "boot_mode_hex: 0x%x, ", boot_modehex);
		strcpy(setting->boot_mode_name, bootmodestr);
		*bootcfglen = board->boot_cfg_byte_num;
		for (int i = 0; i < board->boot_cfg_byte_num; i++)
		{
			mprintf(3, "boot_config_%d_hex: 0x%x\n", i,
				read_boot_config_hex[i]);
			setting->boot_config_hex[i] = read_boot_config_hex[i];
		}
	}

	return 0;
}

static int get_boot_mode(struct options_setting* setting, int *bootcfglen)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;
	unsigned char read_buf;

	gpio = get_gpio("bootmode_sel", board);
	if (gpio != NULL)
	{
		status = gpio->gpio_read(gpio, &read_buf);
		if (status)
			mprintf(2, "get_boot_mode failed, error = 0x%x\n", status);
		if (read_buf != (board->mappings[get_gpio_id("bootmode_sel", board)].initinfo & 0xF))
		{
			mprintf(2, "get_boot_mode: bootmode_sel is disabled, boot from BOOT SWITCH!\n");
			free_gpio(gpio);
			return -LIBBCU_ERR_BOOT_FROM_SWITCH;
		}
		free_gpio(gpio);
	}
	else
	{
		gpio = get_gpio("remote_en", board);
		if (gpio == NULL)
		{
			mprintf(2, "get_boot_mode: Cannot find gpio remote_en!\n");
			free_gpio(gpio);
			return -LIBBCU_ERR_NO_REMOTE_EN;
		}
		status = gpio->gpio_read(gpio, &read_buf);
		if (status)
			mprintf(2, "get_boot_mode failed, error = 0x%x\n", status);
		if (read_buf != (board->mappings[get_gpio_id("remote_en", board)].initinfo & 0xF))
		{
			mprintf(2, "get_boot_mode: remote_en is disabled, boot from BOOT SWITCH!\n");
			free_gpio(gpio);
			return -LIBBCU_ERR_BOOT_FROM_SWITCH;
		}
		free_gpio(gpio);
	}

	gpio = get_gpio("boot_mode", board);
	if (gpio == NULL)
	{
		mprintf(2, "get_boot_mode: No boot_mode configuration!\n");
		free_gpio(gpio);
		return -LIBBCU_ERR_NO_BOOT_MODE_CFG;
	}
	if (get_boot_mode_offset(gpio->pin_bitmask) < 0)
	{
		free_gpio(gpio);
		return -LIBBCU_ERR_BOOT_MODE_OFFSET;
	}
	status = gpio->gpio_read(gpio, &read_buf);
	read_buf = read_buf >> get_boot_mode_offset(gpio->pin_bitmask);
	free_gpio(gpio);
	if (status) {
		mprintf(2, "get_boot_mode failed, error = 0x%x\n", status);
		return -LIBBCU_ERR_GET_BOOT_MODE;
	}
	else
	{
		if (get_boot_mode_name_from_hex(board, read_buf) == NULL) {
			mprintf(3, "get_boot_mode hex value: 0x%x, cannot find the boot mode string.\n",
				read_buf);
		}
		else
		{
			if (!board->boot_cfg_byte_num) {
				mprintf(3, "get_boot_mode: %s, hex value: 0x%x\n",
					get_boot_mode_name_from_hex(board, read_buf),
					read_buf);
				strcpy(setting->boot_mode_name, get_boot_mode_name_from_hex(board, read_buf));
				setting->boot_mode_hex = read_buf;
				bootcfglen = 0;
			} else {
				status = get_boot_config(setting, read_buf, bootcfglen);
				setting->boot_mode_hex = read_buf;
				if (status < 0) {
					mprintf(2, "%s", bcu_get_err_str(status));
					return -LIBBCU_ERR_GET_BOOT_MODE;
				}
			}
		}
	}

	return 0;
}

void bcu_update_debug_level(int debug_level)
{
	GV_DEBUG_LEVEL = debug_level;
}

void bcu_update_location_id(char *location_id)
{
	strcpy(GV_LOCATION_ID, location_id);
}

int bcu_check_gpio_name(struct options_setting *setting)
{
	struct board_info *board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	int j = 0;

	while (board->mappings[j].name != NULL)
	{
		if (strcmp(board->mappings[j].name, setting->gpio_name) == 0)
			return 0;
		j++;
	}

	return -LIBBCU_ERR_UNSPPORTED_GPIO;
}

int bcu_get_boot_mode_hex(struct options_setting *setting)
{
	struct board_info *board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	int k = 0;

	if (board->boot_configs != NULL)
	{
		while (board->boot_configs[k].name != NULL)
		{
			if (strcmp(board->boot_configs[k].name, setting->boot_mode_name) == 0)
			{
				for (int bootcfg_n = 0; bootcfg_n < MAX_BOOT_CONFIG_BYTE; bootcfg_n++)
					setting->boot_config_hex[bootcfg_n] = board->boot_configs[k].boot_config_hex[bootcfg_n];
				break;
			}
			k++;
		}
	}

	k = 0;
	if (board->boot_modes != NULL)
	{
		while (board->boot_modes[k].name != NULL)
		{
			if (strcmp(board->boot_modes[k].name, setting->boot_mode_name) == 0)
			{
				setting->boot_mode_hex = board->boot_modes[k].boot_mode_hex;
				break;
			}
			k++;
		}
	}
	else
		return -LIBBCU_ERR_NO_BOOT_MODE_CFG;

	return 0;
}

int bcu_init(struct options_setting *setting)
{
	struct board_info *board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	void *head = NULL;
	void *end_point;
	char path[MAX_PATH_LENGTH];
	char name[MAX_MAPPING_NAME_LENGTH];
	int status = 0, initid = 1, output = 0, k = 0;

	while (output >= 0)
	{
		output = get_gpio_info_by_initid(name, path, initid, board);
		if (output < 0)
			break;

		if (strcmp(name, "boot_mode") == 0)
		{
			if (setting->boot_mode_hex != -1) {
				status = set_boot_mode(setting);
				if (status < 0)
					return status;
			} else
				return -LIBBCU_ERR_NO_BOOT_MODE_OPT;

			initid++;
			continue;
		}

		end_point = build_device_linkedlist_forward(&head, path);
		if (end_point == NULL)
			return -LIBBCU_ERR_BUILD_DEVICE_LINK;

		struct gpio_device* gpio = end_point;
		if (output)
			status = gpio->gpio_write(gpio, 0xFF);
		else
			status = gpio->gpio_write(gpio, 0x00);

		if (status)
		{
			mprintf(2, "set %s %s failed, error = 0x%x\n", name, output ? "high" : "low", status);
			mprintf(2, "board initialization failed!\n");
			return -LIBBCU_ERR_SET_GPIO_FAIL;
		}
		else
		{
			if (strcmp(name, "remote_en") == 0) {
				mprintf(3, "ENABLE remote control\n");
			} else
				mprintf(3, "set %s %s successfully\n", name, output ? "high" : "low");
		}

		initid++;

		free_device_linkedlist_backward(end_point);
	}

	return 0;
}

int bcu_reset_time_ms(struct options_setting *setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;

	return board->reset_time_ms;
}

int bcu_reset(struct options_setting *setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;
	int a = 0, mask = 0;
	char sr_name[100];

	status = bcu_init(setting);
	if (status < 0)
	{
		mprintf(2, "%s", bcu_get_err_str(status));
		mprintf(2, "board reset failed!\n");
		return -LIBBCU_ERR_INIT_FAIL;
	}

	mprintf(3, "Set ALL sense resistances to smaller ones\n");
	while (board->mappings[a].name != NULL)
	{
		if (board->mappings[a].type == power)
		{
			strcpy(sr_name, "SR_");
			strcat(sr_name, board->mappings[a].name);

			gpio = get_gpio(sr_name, board);
			if (gpio == NULL)
			{
				a++;
				continue;
			}
			status = gpio->gpio_write(gpio, 0xFF); //set it high.
			free_gpio(gpio);
		}
		a++;
	}

	mprintf(1, "resetting in: %ds", board->reset_time_ms / 1000);

	gpio = get_gpio("reset", board);
	mask = board->mappings[get_gpio_id("reset", board)].initinfo & 0xF;
	if (gpio == NULL)
	{
		mprintf(2, "reset: error building device linked list\n");
		return -LIBBCU_ERR_BUILD_DEVICE_LINK;
	}

	//delay
	msleep(setting->delay);

	status = gpio->gpio_write(gpio, mask ? 0x00 : 0xFF); //reset low
	if (setting->hold > 0)
		msleep(setting->hold);
	else
	{
		if (board->reset_time_ms <= 1000)
			msleep(board->reset_time_ms);
		else
		{
			int i;
			for(i = board->reset_time_ms; i > 0; i = i - 1000)
			{
				mprintf(1, "\b\b%ds", i / 1000);
				fflush(stdout);
				msleep(1000);
			}
		}
	}
	mprintf(1, "\n");

	if (setting->boot_mode_hex != -1)
	{
		status |= gpio->gpio_write(gpio, mask ? 0xFF : 0x00) << 1;//reset high
	}
	free_gpio(gpio);

	if (setting->boot_mode_hex == -1)
	{
		if (have_gpio("bootmode_sel", board) != -1)
		{
			gpio = get_gpio("bootmode_sel", board);
			if (gpio == NULL)
			{
				mprintf(2, "reset: error building device linked list\n");
				return -LIBBCU_ERR_BUILD_DEVICE_LINK;
			}
			status |= gpio->gpio_write(gpio, 0xFF) << 2; //bootmode_sel high to disable it.
			free_gpio(gpio);

			msleep(10);

			gpio = get_gpio("reset", board);
			mask = board->mappings[get_gpio_id("reset", board)].initinfo & 0xF;
			if (gpio == NULL)
			{
				mprintf(2, "reset: error building device linked list\n");
				return -LIBBCU_ERR_BUILD_DEVICE_LINK;
			}
			status |= gpio->gpio_write(gpio, mask ? 0xFF : 0x00) << 3; //reset high
			free_gpio(gpio);

			if (!status)
			{
				mprintf(3, "DISABLE remote bootmode control, boot by BOOT SWITCH\n");
				mprintf(3, "remote control is still ENABLED\n");
			}
		}
		else
		{
			gpio = get_gpio("remote_en", board);
			mask = board->mappings[get_gpio_id("remote_en", board)].initinfo & 0xF;
			if (gpio == NULL)
			{
				mprintf(2, "reset: error building device linked list\n");
				return -LIBBCU_ERR_BUILD_DEVICE_LINK;
			}
			status |= gpio->gpio_write(gpio, mask ? 0x00 : 0xFF) << 2; //remote_en low
			if (!status)
				mprintf(3, "DISABLE remote control, boot by BOOT SWITCH\n");
			free_gpio(gpio);
		}
	}

	if (status) {
		mprintf(2, "reset failed, error = 0x%x\n", status);
		return -LIBBCU_ERR_RESET_FAIL;
	} else
		mprintf(3, "reset successfully\n");

	return 0;
}

int bcu_onoff(struct options_setting *setting, int delay_us)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;
	int mask;

	status = bcu_init(setting);
	if (status < 0)
	{
		mprintf(2, "%s", bcu_get_err_str(status));
		mprintf(2, "board reset failed!\n");
		return -LIBBCU_ERR_INIT_FAIL;
	}

	if (delay_us == 0)
		delay_us = 500;

	gpio = get_gpio("onoff", board);
	if (gpio == NULL)
	{
		mprintf(2, "onoff: error building device linked list\n");
		return -LIBBCU_ERR_BUILD_DEVICE_LINK;
	}

	mprintf(3, "onoff button will be pressed for %dus\n", delay_us);

	mask = board->mappings[get_gpio_id("onoff", board)].initinfo & 0xF;

	status = gpio->gpio_write(gpio, mask ? 0x00 : 0xFF); //set it off.
	msleep(delay_us);
	status = gpio->gpio_write(gpio, mask ? 0xFF : 0x00); //set it on.

	free_gpio(gpio);

	if (status) {
		mprintf(2, "onoff execute failed, error = 0x%x\n", status);
		return -LIBBCU_ERR_ONOFF_FAIL;
	} else
		mprintf(3, "onoff execute successfully\n");

	return 0;
}

int bcu_deinit(struct options_setting *setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;
	struct gpio_device* gpio = NULL;
	int status = -1;
	int mask;

	gpio = get_gpio("remote_en", board);
	if (gpio == NULL)
	{
		mprintf(2, "deinitialize: Cannot find gpio remote_en!\n");
		return -LIBBCU_ERR_BUILD_DEVICE_LINK;
	}
	mask = board->mappings[get_gpio_id("remote_en", board)].initinfo & 0xF;
	status = gpio->gpio_write(gpio, mask ? 0x00 : 0xFF); //set it off.
	if (!status)
		mprintf(3, "DISABLE remote control: remote_en\n");
	free_gpio(gpio);

	gpio = get_gpio("bootmode_sel", board);
	if (gpio != NULL)
	{
		mask = board->mappings[get_gpio_id("bootmode_sel", board)].initinfo & 0xF;
		status = gpio->gpio_write(gpio, mask ? 0x00 : 0xFF); //set it off.
		if (!status)
			mprintf(3, "DISABLE remote control: bootmode_sel\n");
		free_gpio(gpio);
	}

	gpio = get_gpio("ft_reset_boot_mode", board);
	if (gpio != NULL)
	{
		status = gpio->gpio_write(gpio, 0x00);
		msleep(50);
		status = gpio->gpio_write(gpio, 0xFF);
		if (!status)
			mprintf(3, "RESET boot mode pin: ft_reset_boot_mode\n");
		free_gpio(gpio);
	}

	return 0;
}

static int get_msecond(unsigned long* current_time)
{
#ifdef _WIN32

	SYSTEMTIME time;
	FILETIME ftime;
	GetSystemTime(&time);
	SystemTimeToFileTime(&time, &ftime);
	*current_time = (ftime.dwLowDateTime + ((UINT64)ftime.dwHighDateTime << 32)) / 10000;

	return 0;
#else
	struct timespec current;
	clock_gettime(CLOCK_REALTIME, &current);
	*current_time = current.tv_sec * 1000.00 + current.tv_nsec / 1000000;
	return 0;
#endif
}

#ifdef _WIN32
HANDLE hThread;
DWORD ThreadID;
#else
pthread_t monitor_thread;
#endif
struct monitor_thread_data monitor_td;

//DWORD WINAPI bcu_monitor(LPVOID threadarg)
void *bcu_monitor(void *threadarg)
{
	struct monitor_thread_data *m_td =  (struct monitor_thread_data *) threadarg;
	// mprintf(4, "Thread ID : %d\n", m_td->thread_id);
	// mprintf(4, "Dump : %d\n", m_td->is_dump);
	// mprintf(4, "Dump name : %s\n", m_td->dumpname);
	// mprintf(4, "HWFilter : %d\n", m_td->is_hwfilter);
	// mprintf(4, "PMT : %d\n", m_td->is_pmt);
	// mprintf(4, "RMS : %d\n", m_td->is_rms);
	// mprintf(4, "STATS : %d\n", m_td->is_stats);
	// mprintf(4, "Unipolar : %d\n", m_td->is_unipolar);

	struct board_info* board = get_board(m_td->setting->board);
	if (board == NULL) {
		// mprintf(2, "entered board model are not supported.\n");
		m_td->ret = -LIBBCU_ERR_NO_THIS_BOARD;
		m_td->is_stop = 1;
		return threadarg;
	}

#if 1
	char previous_path[MAX_PATH_LENGTH];
	void* head = NULL;
	void* end_point = NULL;

	double voltage = 0;
	double current = 0;
	double power = 0;
	char ch = 0;
	char sr_path[MAX_PATH_LENGTH];

	char sr_name[100];
	unsigned long start;
	get_msecond(&start);
	unsigned long long times = 1;
	FILE* fptr = NULL;
	int reset_flag = 0;

	if (m_td->is_dump == 1)
	{
		fptr = fopen(m_td->dumpname, "w+");
		if (fptr == NULL)
		{
			mprintf(2, "\nOpen file ERROR!\nPlease check if the \"%s\" file is still opened.\nExit...\n", m_td->dumpname);
			m_td->ret = -LIBBCU_ERR_MOT_OPEN_FILE;
			m_td->is_stop = 1;
			return threadarg;
		}

		/*print first row*/
		int i = 1, index_n;
		fprintf(fptr, "time(ms)");
		index_n = get_power_index_by_showid(i, board);
		while (index_n != -1)
		{
			if (board->mappings[index_n].type == power && board->mappings[index_n].initinfo != 0)
			{
				if (!m_td->is_pmt)
					fprintf(fptr, ",%s voltage(V),%s current(mA)", board->mappings[index_n].name, board->mappings[index_n].name);
				else
					fprintf(fptr, ",%s voltage(V),%s current(mA),%s power(mW)", board->mappings[index_n].name, board->mappings[index_n].name, board->mappings[index_n].name);
			}
			i++;
			index_n = get_power_index_by_showid(i, board);
		}
	}

	int n = 0; //n is the number of variables need to be measured
	int index = 0;
	while (board->mappings[index].name != NULL)
	{
		if (board->mappings[index].type == power)
			n++;
		index++;
	}
	//ideally I should be able to use n as the size of array, but Visual studio does not allow variable length array declaration...
	// so 100 maximum variable for now...I could use dynamic memory allocation instead, but then I will have to free it one by one in the end..
	int name[MAX_NUMBER_OF_POWER];
	double vnow[MAX_NUMBER_OF_POWER]; double vavg[MAX_NUMBER_OF_POWER]; double vmax[MAX_NUMBER_OF_POWER]; double vmin[MAX_NUMBER_OF_POWER];
	double cnow[MAX_NUMBER_OF_POWER]; double cavg[MAX_NUMBER_OF_POWER]; double cmax[MAX_NUMBER_OF_POWER]; double cmin[MAX_NUMBER_OF_POWER];
	double pnow[MAX_NUMBER_OF_POWER]; double pavg[MAX_NUMBER_OF_POWER]; double pmax[MAX_NUMBER_OF_POWER]; double pmin[MAX_NUMBER_OF_POWER];
	double data_size[MAX_NUMBER_OF_POWER];
	double cnow_fwrite[MAX_NUMBER_OF_POWER];
	double pnow_fwrite[MAX_NUMBER_OF_POWER];
	int sr_level[MAX_NUMBER_OF_POWER];
	int range_control = 0;
	int range_level[MAX_NUMBER_OF_POWER] = {0};
	double cur_range[MAX_NUMBER_OF_POWER];
	double unused_range[MAX_NUMBER_OF_POWER];

	//initialize
	for (int i = 0; i < MAX_NUMBER_OF_POWER; i++)
	{
		vavg[i] = 0; vmax[i] = 0; vmin[i] = 99999;
		cavg[i] = 0; cmax[i] = 0; cmin[i] = 99999;
		pavg[i] = 0; pmax[i] = 0; pmin[i] = 99999;
		data_size[i] = 0;
		name[i] = 0;
	}

	//power groups
	struct group groups[MAX_NUMBER_OF_POWER];
	groups_init(groups, MAX_NUMBER_OF_POWER);

	int num_of_groups = parse_groups(groups, board);
	if (num_of_groups == -1)
	{
		free_device_linkedlist_backward(end_point);
		if (m_td->is_dump == 1)
			fclose(fptr);
		m_td->ret = -LIBBCU_ERR_MOT_GROUP_PARSE;
		m_td->is_stop = 1;
		return threadarg;
	}

	if (m_td->is_dump == 1)
	{
		for (int i = 0; i < num_of_groups; i++)
		{
			fprintf(fptr, ",%s Power(mW)", board->power_groups[i].group_name);
		}
		fprintf(fptr, "\n");
	}

	//get first channels of all groups of rails
	int a = 0, pac_group_num = 0, last_pac_group = 0, pac_channel_num = 0;
	struct pac193x_reg_data pac_data[MAX_NUMBER_OF_POWER];
	char pac193x_group_path[MAX_NUMBER_OF_POWER][MAX_PATH_LENGTH];
	while (board->mappings[a].name != NULL)
	{
		if (board->mappings[a].type == power && board->mappings[a].initinfo != 0) //-------------------------------------------------------------------------
		{
			end_point = build_device_linkedlist_smart(&head, board->mappings[a].path, head, previous_path);
			strcpy(previous_path, board->mappings[a].path);

			if (end_point == NULL) {
				mprintf(2, "monitor:failed to build device linkedlist\n");
				if (m_td->is_dump == 1)
					fclose(fptr);
				m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
				m_td->is_stop = 1;
				return threadarg;
			}
			struct power_device* pd = end_point;

			int now_pac_group = pd->power_get_group(pd);
			if(last_pac_group != now_pac_group)
			{
				strcpy(pac193x_group_path[now_pac_group - 1], board->mappings[a].path);
				pac_group_num++;
				last_pac_group = now_pac_group;
			}

			pac_channel_num++;

			strcpy(sr_name, "SR_");
			strcat(sr_name, board->mappings[a].name);
			if (get_path(sr_path, sr_name, board) != -1)
			{
				end_point = build_device_linkedlist_smart(&head, sr_path, head, previous_path);
				strcpy(previous_path, sr_path);

				if (end_point == NULL) {
					mprintf(2, "monitor:failed to build device linkedlist\n");
					if (m_td->is_dump == 1)
						fclose(fptr);
					m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
					m_td->is_stop = 1;
					return threadarg;
				}
				struct gpio_device* gd = end_point;
				unsigned char data;
				gd->gpio_get_output(gd, &data);

				if (data == 0)
					sr_level[a] = 0;
				else
					sr_level[a] = 1;
			}
			else
			{
				sr_level[a] = -1;
			}
		}
		a++;
	}

	int pac_group_count = pac_group_num;
	for(a = 0; a < MAX_NUMBER_OF_POWER; a++)
	{
		if (strlen(pac193x_group_path[a]) < 10)
		{
			continue;
		}
		if (pac_group_count <= 0)
			break;
		end_point = build_device_linkedlist_smart(&head, pac193x_group_path[a], head, previous_path);
		strcpy(previous_path, pac193x_group_path[a]);

		if (end_point == NULL) {
			mprintf(2, "monitor:failed to build device linkedlist\n");
			if (m_td->is_dump == 1)
				fclose(fptr);
			m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
			m_td->is_stop = 1;
			return threadarg;
		}
		struct power_device* pd = end_point;

		if (!m_td->is_unipolar)
			pd->power_write_bipolar(pd, 1);
		else
			pd->power_write_bipolar(pd, 0);

		//snapshot once to skip the first data that may not match the polarity configuration
		pd->power_set_snapshot(pd);

		pac_group_count--;
	}

	while (!m_td->is_stop)
	{
		//first refresh all pac1934's
		pac_group_count = pac_group_num;
		for(a = 0; a < MAX_NUMBER_OF_POWER; a++)
		{
			if (strlen(pac193x_group_path[a]) < 10)
			{
				continue;
			}
			if (pac_group_count <= 0)
				break;
			end_point = build_device_linkedlist_smart(&head, pac193x_group_path[a], head, previous_path);
			strcpy(previous_path, pac193x_group_path[a]);
			if (end_point == NULL) {
				mprintf(2, "monitor:failed to build device linkedlist\n");
				if (m_td->is_dump == 1)
					fclose(fptr);
				m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
				m_td->is_stop = 1;
				return threadarg;
			}
			struct power_device* pd = end_point;

			pd->power_set_snapshot(pd);
			pac_group_count--;
		}
		if (pac_group_num < 4)
			msleep(1);

		int cannotacc = 0;
		pac_group_count = pac_group_num;
		for(a = 0; a < MAX_NUMBER_OF_POWER; a++)
		{
			if (pac_group_count <= 0)
				break;

			if (strlen(pac193x_group_path[a]) < 10)
			{
				continue;
			}
			end_point = build_device_linkedlist_smart(&head, pac193x_group_path[a], head, previous_path);
			strcpy(previous_path, pac193x_group_path[a]);

			if (end_point == NULL) {
				mprintf(2, "monitor:failed to build device linkedlist\n");
				if (m_td->is_dump == 1)
					fclose(fptr);
				m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
				m_td->is_stop = 1;
				return threadarg;
			}
			struct power_device* pd = end_point;

			if (m_td->is_hwfilter)
				pd->power_set_hwfilter(pd, 1);
			else
				pd->power_set_hwfilter(pd, 0);

			if (!m_td->is_unipolar)
				pd->power_set_bipolar(pd, 1);
			else
				pd->power_set_bipolar(pd, 0);

			int b;
			for (b = 0; b < 50; b++)
			{
				if (pd->power_get_data(pd, &pac_data[a]) >= 0)
				{
					break;
				}
				else
					cannotacc = 1;
			}
			if (b == 50)
			{
				mprintf(2, "PAC1934 cannot access!\n");
				if (m_td->is_dump == 1)
					fclose(fptr);
				m_td->ret = -LIBBCU_ERR_MOT_PAC1934_CANNOT_ACCESS;
				m_td->is_stop = 1;
				return threadarg;
			}
			else if(cannotacc == 1)
				break;
			pac_group_count--;
		}
		if(cannotacc == 1)
			continue;

		//calculate the value and store them in array
		int i = 0;//i is the index of all mappings
		int j = 0;//j is the index of the power related mapping only
		while (board->mappings[i].name != NULL)
		{
			if (board->mappings[i].type == power && board->mappings[i].initinfo != 0)
			{
				name[j] = i;

				if (reset_flag == 1)
				{
					strcpy(sr_name, "SR_");
					strcat(sr_name, board->mappings[name[j]].name);
					if (get_path(sr_path, sr_name, board) != -1)
					{
						end_point = build_device_linkedlist_smart(&head, sr_path, head, previous_path);
						strcpy(previous_path, sr_path);

						if (end_point == NULL) {
							mprintf(2, "monitor:failed to build device linkedlist\n");
							if (m_td->is_dump == 1)
								fclose(fptr);
							m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
							m_td->is_stop = 1;
							return threadarg;
						}
						struct gpio_device* gd = end_point;
						unsigned char data;
						gd->gpio_get_output(gd, &data);

						if (data == 0)
							sr_level[j] = 0;
						else
							sr_level[j] = 1;
					}
					else
					{
						sr_level[j] = -1;
					}
				}

				end_point = build_device_linkedlist_smart(&head, board->mappings[i].path, head, previous_path);
				strcpy(previous_path, board->mappings[i].path);

				if (end_point == NULL) {
					mprintf(2, "monitor:failed to build device linkedlist\n");
					if (m_td->is_dump == 1)
						fclose(fptr);
					m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
					m_td->is_stop = 1;
					return threadarg;
				}
				struct power_device* pd = end_point;

				if(sr_level[j] == 0)
					pd->switch_sensor(pd, 1);
				else
					pd->switch_sensor(pd, 0);

				int group = pd->power_get_group(pd) - 1;
				int sensor = pd->power_get_sensor(pd) - 1;
				voltage = pac_data[group].vbus[sensor] - (pac_data[group].vsense[sensor] / 1000000);
				current = pac_data[group].vsense[sensor] / pd->power_get_cur_res(pd);
				cnow_fwrite[j] = current;
				pnow_fwrite[j] = current * voltage;

				if (range_control == 0)
				{
					range_level[j] = (char)(0 | range_level[j] << 4);  //mA
				}
				else if (range_control == 2)
				{
					current *= 1000;
					range_level[j] = (char)(1 | range_level[j] << 4);  //uA
				}
				else
				{
					if ( (!(range_level[j] & 0xf) && cavg[j] < 1) || ((range_level[j] & 0xf) && cavg[j] < 1000))
					{
						current *= 1000;
						range_level[j] = (char)(1 | range_level[j] << 4);  //uA
					}
					else
						range_level[j] = (char)(0 | range_level[j] << 4);  //mA
				}

				cur_range[j] = 100000.0 / pd->power_get_cur_res(pd);
				unused_range[j] = 100000.0 / pd->power_get_unused_res(pd);
				// cur_range[j] = pac_data[pd->power_get_group(pd) - 1].vsense[pd->power_get_sensor(pd) - 1];

				// mprintf(4, "current %f\n", current);
				double power = current * voltage;
				vnow[j] = voltage;
				cnow[j] = current;
				pnow[j] = power;
				if (range_level[j] == 0x0 || range_level[j] == 0x11)
				{
					vmin[j] = (voltage < vmin[j]) ? voltage : vmin[j];
					cmin[j] = (current < cmin[j]) ? current : cmin[j];
					pmin[j] = (power < pmin[j]) ? power : pmin[j];
					vmax[j] = (voltage > vmax[j]) ? voltage : vmax[j];
					cmax[j] = (current > cmax[j]) ? current : cmax[j];
					pmax[j] = (power > pmax[j]) ? power : pmax[j];
					vavg[j] = (data_size[j] * vavg[j] + voltage) / (double)(data_size[j] + 1);
					pavg[j] = (data_size[j] * pavg[j] + power) / ((double)(data_size[j] + 1));
				}
				else if (range_level[j] == 0x01)
				{
					vmin[j] = (voltage < vmin[j]) ? voltage : vmin[j];
					cmin[j] = (current < cmin[j] * 1000) ? current : cmin[j] * 1000;
					pmin[j] = (power < pmin[j] * 1000) ? power : pmin[j] * 1000;
					vmax[j] = (voltage > vmax[j]) ? voltage : vmax[j];
					cmax[j] = (current > cmax[j] * 1000) ? current : cmax[j] * 1000;
					pmax[j] = (power > pmax[j] * 1000) ? power : pmax[j] * 1000;
					cavg[j] *= 1000;
					vavg[j] = (data_size[j] * vavg[j] + voltage) / (double)(data_size[j] + 1);
					pavg[j] = (data_size[j] * pavg[j] * 1000 + power) / ((double)(data_size[j] + 1));
				} 
				else if (range_level[j] == 0x10)
				{
					vmin[j] = (voltage < vmin[j]) ? voltage : vmin[j];
					cmin[j] = (current < cmin[j] / 1000) ? current : cmin[j] / 1000;
					pmin[j] = (power < pmin[j] / 1000) ? power : pmin[j] / 1000;
					vmax[j] = (voltage > vmax[j]) ? voltage : vmax[j];
					cmax[j] = (current > cmax[j] / 1000) ? current : cmax[j] / 1000;
					pmax[j] = (power > pmax[j] / 1000) ? power : pmax[j] / 1000;
					cavg[j] /= 1000;
					vavg[j] = (data_size[j] * vavg[j] + voltage) / (double)(data_size[j] + 1);
					pavg[j] = (data_size[j] * pavg[j] / 1000 + power) / ((double)(data_size[j] + 1));
				}

				if (m_td->is_rms)
				{
					cavg[j] = sqrt((data_size[j] * cavg[j] * cavg[j] + current * current) / (double)(data_size[j] + 1));
					pavg[j] = cavg[j] * vavg[j];
				}
				else
					cavg[j] = (data_size[j] * cavg[j] + current) / (double)(data_size[j] + 1);

				data_size[j] = data_size[j] + 1;

				j++;
			}
			else
				j++;
			i++;
		}

		if (reset_flag == 1)
			reset_flag = 0;

		//get groups data
		for (int k = 0; k < num_of_groups; k++)
		{
			groups[k].sum = 0;
			for (int x = 0; x < groups[k].num_of_members; x++)
			{
				int mi = groups[k].member_index[x];
				if (range_level[mi] == 0x01 || range_level[mi] == 0x11)
					pnow[mi] /= 1000;
				groups[k].sum += pnow[mi];
			}
			groups[k].max = (groups[k].sum > groups[k].max) ? groups[k].sum : groups[k].max;
			groups[k].min = (groups[k].sum < groups[k].min) ? groups[k].sum : groups[k].min;
			groups[k].avg = (groups[k].avg_data_size * groups[k].avg + groups[k].sum) / (groups[k].avg_data_size + 1);
			groups[k].avg_data_size++;
		}
		//dump data to file
		if (m_td->is_dump == 1)
		{
			unsigned long now;
			get_msecond(&now);
			fprintf(fptr, "%ld", (long)now - start);//add time before the first element
			for (int m = 0; m < n + 1; m++)
			{
				int k = get_power_index_by_showid(m, board);
				if (k < 0)
					continue;
				if (board->mappings[k].initinfo != 0)
				{
					if (!m_td->is_pmt)
						fprintf(fptr, ",%lf,%lf", vnow[k], cnow_fwrite[k]);
					else
						fprintf(fptr, ",%lf,%lf,%lf", vnow[k], cnow_fwrite[k], pnow_fwrite[k]);
				}
			}
			for (int k = 0; k < num_of_groups; k++)
			{
				fprintf(fptr, ",%lf", groups[k].sum);
			}
			fprintf(fptr, "\n");
		}

#ifdef _WIN32
		WaitForSingleObject(m_td->h_mutex, INFINITE);
#else
		pthread_mutex_lock(&m_td->mutex);
#endif

		memset(&m_td->monitor_powers, 0, sizeof(m_td->monitor_powers));

		int rail_number = 0;
		for (int m = 0; m < n + 1; m++)
		{
			int k = get_power_index_by_showid(m, board);
			if (k < 0)
				continue;
			if (board->mappings[k].initinfo != 0)
			{
				strcat(m_td->monitor_powers.rail_infos[rail_number].rail_name, board->mappings[k].name);
				if(range_level[k] == 0x01 || range_level[k] == 0x11)
					m_td->monitor_powers.rail_infos[rail_number].range_level = 1;
				else
					m_td->monitor_powers.rail_infos[rail_number].range_level = 0;
				m_td->monitor_powers.rail_infos[rail_number].range_switch = sr_level[k];
				m_td->monitor_powers.rail_infos[rail_number].range_list[0] = sr_level[k] ? cur_range[k] : unused_range[k];
				m_td->monitor_powers.rail_infos[rail_number].range_list[1] = sr_level[k] ? unused_range[k] : cur_range[k];
				m_td->monitor_powers.rail_infos[rail_number].v_now = vnow[k];
				m_td->monitor_powers.rail_infos[rail_number].v_avg = vavg[k];
				m_td->monitor_powers.rail_infos[rail_number].v_min = vmin[k];
				m_td->monitor_powers.rail_infos[rail_number].v_max = vmax[k];
				m_td->monitor_powers.rail_infos[rail_number].c_now = cnow[k];
				m_td->monitor_powers.rail_infos[rail_number].c_avg = cavg[k];
				m_td->monitor_powers.rail_infos[rail_number].c_min = cmin[k];
				m_td->monitor_powers.rail_infos[rail_number].c_max = cmax[k];
				m_td->monitor_powers.rail_infos[rail_number].p_now = pnow[k];
				m_td->monitor_powers.rail_infos[rail_number].p_avg = pavg[k];
				m_td->monitor_powers.rail_infos[rail_number].p_min = pmin[k];
				m_td->monitor_powers.rail_infos[rail_number].p_max = pmax[k];
				rail_number++;
			}
		}
		m_td->monitor_powers.rail_num = rail_number;
		if (num_of_groups > 0)
		{
			for (int k = 0; k < num_of_groups; k++)
			{
				strcat(m_td->monitor_powers.group_infos[k].group_name, groups[k].name);
				m_td->monitor_powers.group_infos[k].p_now = groups[k].sum;
				m_td->monitor_powers.group_infos[k].p_avg = groups[k].avg;
				m_td->monitor_powers.group_infos[k].p_min = groups[k].min;
				m_td->monitor_powers.group_infos[k].p_max = groups[k].max;
			}
			m_td->monitor_powers.group_num = num_of_groups;
		}
		m_td->monitor_powers.sample_times = times + 1;
		m_td->monitor_powers.time_start = start;
		get_msecond(&m_td->monitor_powers.time_now);
		m_td->monitor_powers.range_ctrl = range_control;
#ifdef _WIN32
		ReleaseMutex(m_td->h_mutex);
#else
		pthread_mutex_unlock(&m_td->mutex);
#endif

		times++;

		//finally,switch the SR
		ch = m_td->hot_key;
		if (isxdigit(ch))
		{
			int hotkey_index = (int)ch - '0';
			int bootmodenum = 0;
			int input_num = 0;
			switch (hotkey_index)
			{
			case 1:
				for (int k = 0; k < n; k++)
				{
					cavg[k] = 0;
					vavg[k] = 0;
					pavg[k] = 0;
					data_size[k] = 0;
				}
				for (int k = 0; k < num_of_groups; k++)
				{
					groups[k].avg = 0;
					groups[k].avg_data_size = 0;
				}
				if (m_td->is_dump == 1 && !m_td->is_pmt)
					fprintf(fptr, "HOT-KEY %d PRESSED: Reset AVG value\n", hotkey_index);
				break;
			case 2:
				for (int k = 0; k < n; k++)
				{
					cmin[k] = 99999;
					vmin[k] = 99999;
					pmin[k] = 99999;
					cmax[k] = 0;
					vmax[k] = 0;
					pmax[k] = 0;
				}
				for (int k = 0; k < num_of_groups; k++)
				{
					groups[k].max = 0;
					groups[k].min = 99999;
				}
				if (m_td->is_dump == 1 && !m_td->is_pmt)
					fprintf(fptr, "HOT-KEY %d PRESSED: Reset MAXMIN value\n", hotkey_index);
				break;
			case 3:
				for (int k = 0; k < n; k++)
				{
					cavg[k] = 0;
					vavg[k] = 0;
					pavg[k] = 0;
					data_size[k] = 0;
					cmin[k] = 99999;
					vmin[k] = 99999;
					pmin[k] = 99999;
					cmax[k] = 0;
					vmax[k] = 0;
					pmax[k] = 0;
				}
				for (int k = 0; k < num_of_groups; k++)
				{
					groups[k].avg = 0;
					groups[k].avg_data_size = 0;
					groups[k].max = 0;
					groups[k].min = 99999;
				}
				if (m_td->is_dump == 1 && !m_td->is_pmt)
					fprintf(fptr, "HOT-KEY %d PRESSED: Reset AVG and MAXMIN value\n", hotkey_index);
				break;
			case 4:
				if (!m_td->is_stats)
				{
					range_control++;
					if (range_control > 2)
						range_control = 0;
				}
				break;
			default:
				break;
			}
		}
		if (isalpha(ch))
		{
			int sr_index;
			ch = (int)ch - 64;
			if (ch < 27)
				sr_index = get_power_index_by_showid((int)ch, board);//is the ascii code for letter A
			else
			{
				sr_index = get_power_index_by_showid((int)ch - 32 + 26, board);//is the ascii code for letter a
			}
			if (sr_index < n && sr_index < MAX_NUMBER_OF_POWER && sr_index >= 0)
			{
				strcpy(sr_name, "SR_");
				if (board->mappings[name[sr_index]].name == NULL)
				{
					if (m_td->is_dump == 1)
						fclose(fptr);
					m_td->ret = -LIBBCU_ERR_MOT_NO_SR;
					m_td->is_stop = 1;
					return threadarg;
				}
				strcat(sr_name, board->mappings[name[sr_index]].name);

				if (get_path(sr_path, sr_name, board) != -1)
				{
					end_point = build_device_linkedlist_smart(&head, sr_path, head, previous_path);
					strcpy(previous_path, sr_path);
					if (end_point == NULL) {
						mprintf(2, "monitor:failed to build device linkedlist\n");
						if (m_td->is_dump == 1)
						fclose(fptr);
						m_td->ret = -LIBBCU_ERR_BUILD_DEVICE_LINK;
						m_td->is_stop = 1;
						return threadarg;
					}
					struct gpio_device* gd = end_point;
					unsigned char data;
					gd->gpio_get_output(gd, &data);
					if (data == 0)
						gd->gpio_write(gd, 0xFF);
					else
						gd->gpio_write(gd, 0x00);

					msleep(2);
					reset_flag = 1; //to force refresh sr_level, if some rails share SR_ pin
				}
			}
		}
#ifdef _WIN32
		WaitForSingleObject(m_td->h_mutex, INFINITE);
#else
		pthread_mutex_lock(&m_td->mutex);
#endif
		m_td->hot_key = 0;
#ifdef _WIN32
		ReleaseMutex(m_td->h_mutex);
#else
		pthread_mutex_unlock(&m_td->mutex);
#endif
	}

	free_device_linkedlist_backward(end_point);
	if (m_td->is_dump == 1)
	{
		if (m_td->is_stop && m_td->is_stats && !m_td->is_pmt)
		{
			fprintf(fptr, "AVG%s", m_td->is_rms ? "(RMS for current)" : "");
			for (int m = 0; m < n + 1; m++)
			{
				int k = get_power_index_by_showid(m, board);
				if (k < 0)
					continue;
				if (board->mappings[k].initinfo != 0)
				{
					if (!m_td->is_pmt)
						fprintf(fptr, ",%lf,%lf", vavg[k], cavg[k]);
					else
						fprintf(fptr, ",%lf,%lf,%lf", vavg[k], cavg[k], pavg[k]);
				}
			}
			for (int k = 0; k < num_of_groups; k++)
			{
				fprintf(fptr, ",%lf", groups[k].avg);
			}
			fprintf(fptr, "\n");

			fprintf(fptr, "MAX");
			for (int m = 0; m < n + 1; m++)
			{
				int k = get_power_index_by_showid(m, board);
				if (k < 0)
					continue;
				if (board->mappings[k].initinfo != 0)
				{
					if (!m_td->is_pmt)
						fprintf(fptr, ",%lf,%lf", vmax[k], cmax[k]);
					else
						fprintf(fptr, ",%lf,%lf,%lf", vmax[k], cmax[k], pmax[k]);
				}
			}
			for (int k = 0; k < num_of_groups; k++)
			{
				fprintf(fptr, ",%lf", groups[k].max);
			}
			fprintf(fptr, "\n");

			fprintf(fptr, "MIN");
			for (int m = 0; m < n + 1; m++)
			{
				int k = get_power_index_by_showid(m, board);
				if (k < 0)
					continue;
				if (board->mappings[k].initinfo != 0)
				{
					if (!m_td->is_pmt)
						fprintf(fptr, ",%lf,%lf", vmin[k], cmin[k]);
					else
						fprintf(fptr, ",%lf,%lf,%lf", vmin[k], cmin[k], pmin[k]);
				}
			}
			for (int k = 0; k < num_of_groups; k++)
			{
				fprintf(fptr, ",%lf", groups[k].min);
			}
			fprintf(fptr, "\n");
		}
		fclose(fptr);
	}
#endif

#ifdef _WIN32
	return threadarg;
#else
	pthread_exit(NULL);
#endif
}

int bcu_monitor_perpare(struct options_setting *setting)
{
	int ret;

	mprintf(3, "bcu_monitor_perpare: creating monitor thread\n");

	memset(&monitor_td, 0, sizeof(monitor_td));

#ifdef _WIN32
	monitor_td.h_mutex = CreateMutex(NULL, FALSE, NULL);
#else
	if (pthread_mutex_init(&monitor_td.mutex, NULL))
		return -LIBBCU_ERR_MUTEX_LOCK_INIT;
#endif

#ifdef _WIN32
	WaitForSingleObject(monitor_td.h_mutex, INFINITE);
#else
	pthread_mutex_lock(&monitor_td.mutex);
#endif

	monitor_td.is_dump = setting->dump;
	if (setting->dumpname)
		strcpy(monitor_td.dumpname, setting->dumpname);
	monitor_td.is_hwfilter = setting->use_hwfilter;
	monitor_td.is_pmt = setting->pmt;
	monitor_td.is_rms = setting->use_rms;
	monitor_td.is_stats = setting->dump_statistics;
	monitor_td.is_unipolar = setting->use_unipolar;
	monitor_td.setting = setting;
#ifdef _WIN32
	ReleaseMutex(monitor_td.h_mutex);
#else
	pthread_mutex_unlock(&monitor_td.mutex);
#endif

#ifdef _WIN32
	//hThread = CreateThread(NULL, 0, bcu_monitor, (LPVOID)(1), NULL, &ThreadID);
	hThread = _beginthread(bcu_monitor, 0, (void*)&monitor_td);
	if (hThread < 0)
	{
		mprintf(2, "pthread_create error: error_code = %d\n", ret);
		return -LIBBCU_ERR_CREATE_MONITOR_THREAD;
	}
#else
	ret = pthread_create(&monitor_thread, NULL, bcu_monitor, (void *)&monitor_td);
	if (ret != 0) {
		mprintf(2, "pthread_create error: error_code = %d\n", ret);
		return -LIBBCU_ERR_CREATE_MONITOR_THREAD;
	}
#endif

	return 0;
}

int bcu_monitor_set_hotkey(char hotkey)
{
#ifdef _WIN32
	WaitForSingleObject(monitor_td.h_mutex, INFINITE);
#else
	pthread_mutex_lock(&monitor_td.mutex);
#endif

	monitor_td.hot_key = hotkey;

#ifdef _WIN32
	ReleaseMutex(monitor_td.h_mutex);
#else
	pthread_mutex_unlock(&monitor_td.mutex);
#endif
	while (monitor_td.hot_key != hotkey) {}

	return 0;
}

int bcu_monitor_getvalue(struct options_setting *setting, powers *power_info)
{
#ifdef _WIN32
	WaitForSingleObject(monitor_td.h_mutex, INFINITE);
#else
	pthread_mutex_lock(&monitor_td.mutex);
#endif

	memcpy(power_info, &monitor_td.monitor_powers, sizeof(powers));

#ifdef _WIN32
	ReleaseMutex(monitor_td.h_mutex);
#else
	pthread_mutex_unlock(&monitor_td.mutex);
#endif

	return 0;
}

int bcu_monitor_is_stop(void)
{
	return monitor_td.is_stop;
}

int bcu_monitor_get_err(void)
{
	return monitor_td.ret;
}

int bcu_monitor_unperpare(struct options_setting *setting)
{
#ifdef _WIN32
	CloseHandle(monitor_td.h_mutex);
#else
	pthread_mutex_destroy(&monitor_td.mutex);
#endif

	monitor_td.is_stop = 1;

#ifdef _WIN32
	WaitForSingleObject(hThread, INFINITE);
#else
	pthread_join(monitor_thread, NULL);
#endif

	return monitor_td.ret;
}

int bcu_return_rail_name_max_len(struct options_setting *setting)
{
	struct board_info* board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;

	return get_max_power_name_length(board);
}

void bcu_remove_all_ftdi_port(void)
{
	ft4232h_i2c_remove_all();
}


int bcu_eeprom(struct options_setting *setting, eeprom_informations *eeprom_info)
{
	void* head = NULL;
	void* end_point;
	int j = 0;

	struct board_info* board=get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;

	while(board->mappings[j].name != NULL)
	{
		if(board->mappings[j].type == ftdi_eeprom || board->mappings[j].type == i2c_eeprom)
		{
			mprintf(3, "\n>>>>>> Registered %s EEPROM on board >>>>>>\n", board->mappings[j].type == ftdi_eeprom ? "FTDI" : "AT24Cxx");
			strcpy(eeprom_info->type, board->mappings[j].type == ftdi_eeprom ? "FTDI" : "AT24Cxx");
			end_point = build_device_linkedlist_forward(&head, board->mappings[j].path);
			if (end_point == NULL)
			{
				mprintf(2, "eeprom: error building device linked list\n");
				return -LIBBCU_ERR_BUILD_DEVICE_LINK;
			}

			struct eeprom_device* eeprom = end_point;
			switch (setting->eeprom_function)
			{
			case PARSER_EEPROM_READ_AND_PRINT:
				if (bcu_ftdi_eeprom_return_info(eeprom, eeprom_info) < 0)
					return -LIBBCU_ERR_EEPROM_READ;
				break;
			case PARSER_EEPROM_READ_TO_FILE:
				break;
			case PARSER_EEPROM_WRITE_DEFAULT:
				if (bcu_ftdi_eeprom_write_default(eeprom, board->eeprom_data))
					return -LIBBCU_ERR_EEPROM_WRITE_DEFAULT;
				else
					mprintf(3, "Write %s default values to FTDI EEPROM successfully\n", setting->board);
				if (setting->eeprom_usr_sn)
					if (bcu_ftdi_eeprom_update_usr_sn(eeprom, setting->eeprom_usr_sn))
						return -LIBBCU_ERR_EEPROM_UPDATE_USRSN;
				if (setting->eeprom_board_rev[0] != 0)
					if (bcu_ftdi_eeprom_update_board_rev(eeprom, setting->eeprom_board_rev))
						return -LIBBCU_ERR_EEPROM_UPDATE_BREV;
				if (setting->eeprom_soc_rev[0] != 0)
					if (bcu_ftdi_eeprom_update_soc_rev(eeprom, setting->eeprom_soc_rev))
						return -LIBBCU_ERR_EEPROM_UPDATE_SREV;
				if (bcu_ftdi_eeprom_return_info(eeprom, eeprom_info) < 0)
					return -LIBBCU_ERR_EEPROM_READ;
				break;
			case PARSER_EEPROM_WRITE_FROM_FILE:
				break;
			case PARSER_EEPROM_UPDATE_USER_SN:
				if (bcu_ftdi_eeprom_update_usr_sn(eeprom, setting->eeprom_usr_sn))
					return -LIBBCU_ERR_EEPROM_UPDATE_USRSN;
				if (bcu_ftdi_eeprom_return_info(eeprom, eeprom_info) < 0)
					return -LIBBCU_ERR_EEPROM_READ;
				break;
			case PARSER_EEPROM_ERASE:
				if (bcu_ftdi_eeprom_erase(eeprom))
					return -LIBBCU_ERR_EEPROM_ERASE;
				break;
			default:
				return -LIBBCU_ERR_EEPROM_NO_OPT;
				break;
			}
		}
		j++;
	}

	return 0;
}

int bcu_gpio(struct options_setting *setting)
{
	switch (setting->get_or_set)
	{
	case 1:
		return set_gpio_level(setting);
	case 2:
		return get_gpio_level(setting);
	default:
		return -LIBBCU_ERR_NO_GET_OR_SET;
	}
}

int bcu_bootmode(struct options_setting *setting, int *bootconfiglen)
{
	switch (setting->get_or_set)
	{
	case 1:
		if (setting->boot_mode_hex != -1) {
			return set_boot_mode(setting);
		} else
			return -LIBBCU_ERR_NO_BOOT_MODE_OPT;
		return 0;
	case 2:
		return get_boot_mode(setting, bootconfiglen);
	default:
		return -LIBBCU_ERR_NO_GET_OR_SET;
	}
}

int bcu_lsftdi(int is_auto, char boardlist[][MAX_MAPPING_NAME_LENGTH], char location_id_str[][MAX_LOCATION_ID_LENGTH])
{
	int board_num = 0, b;
	char board_name[MAX_MAPPING_NAME_LENGTH];
	if (!is_auto)
		ft_list_devices(location_id_str, &board_num, LIST_DEVICE_MODE_OUTPUT);
	else
	{
		ft_list_devices(location_id_str, &board_num, LIST_DEVICE_MODE_OUTPUT);

		for (int j = 0; j < board_num; j++)
		{
			strcpy(GV_LOCATION_ID, location_id_str[j]);
			switch (find_board_by_eeprom(board_name, &b))
			{
			case 0:
				strcpy(boardlist[j], board_name);
				break;
			case -1:
				strcpy(boardlist[j], "unknown");
				break;
			case -2:
				strcpy(boardlist[j], "ERR: FTDI open fail");
				return -LIBBCU_ERR_OPEN_FTDI_CHANNEL;
			default:
				break;
			}
		}
	}

	return board_num;
}

int bcu_lsboard(char boardlist[][MAX_MAPPING_NAME_LENGTH])
{
	for (int i = 0; i < num_of_boards; i++)
		strcpy((boardlist[i]), board_list[i].name);

	return num_of_boards;
}

int bcu_lsbootmode(struct options_setting *setting, char bootmodelist[][MAX_MAPPING_NAME_LENGTH])
{
	int i = 0;
	struct board_info *board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;

	while (board->boot_modes[i].name != NULL)
	{
		strcpy((bootmodelist[i]), board->boot_modes[i].name);
		i++;
	}

	return i;
}

int bcu_lsgpio(struct options_setting *setting, char gpiolist[][MAX_MAPPING_NAME_LENGTH])
{
	int i = 0, j = 0;
	struct board_info *board = get_board(setting->board);
	if (board == NULL)
		return -LIBBCU_ERR_NO_THIS_BOARD;

	while (board->mappings[i].name != NULL)
	{
		if (board->mappings[i].type == gpio)
			strcpy((gpiolist[j++]), board->mappings[i].name);
		i++;
	}

	return j;
}

int bcu_get_yaml_file_path(char *yamlfilepath)
{
	get_yaml_file_path(yamlfilepath);

	return 0;
}

int bcu_get_yaml_file(struct options_setting *setting, char *yamlfilepath)
{
	int ret;

	switch (readConf(setting->board))
	{
	case 0:
		break;
	case -1:
		mprintf(2, "Trying to create new config file to %s ...\n", yamlfilepath);
		ret = writeConf();
		if (ret < 0)
			return ret;
		if (readConf(setting->board) < 0)
			return -LIBBCU_ERR_YML_READ;
		break;
	case -2:
		mprintf(2, "config file read error, please check or delete config file: %s and try again.\n", yamlfilepath);
		return -LIBBCU_ERR_YML_PARSER;
		break;
	case -3:
		return -LIBBCU_WARN_YML_OLD;
		break;
	default:
		break;
	}

	return 0;
}
