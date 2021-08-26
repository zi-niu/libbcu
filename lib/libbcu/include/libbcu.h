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

#ifndef HELLO_H
#define HELLO_H
#include <stdio.h>

#define MAX_PATH_LENGTH			800	//maximum path length allowed
#define MAX_NUMBER_OF_POWER		100	//maximum number of power types
#define MAX_NUMBER_OF_GROUP		10	//maximum number of power groups
#define MAX_NUMBER_OF_BOOTMODE		20	//maximum number of boot mode
#define MAX_NUMBER_OF_GPIO		50	//maximum number of gpio pins
#define MAX_NUMBER_OF_BOARD		64	//maximum number of gpio pins
#define MAX_MAPPING_NAME_LENGTH		30	//maximum length of the name of the mapping
#define MAX_BOOT_CONFIG_BYTE		5
#define MAX_NUMBER_OF_USB_DEVICES	127
#define MAX_LOCATION_ID_LENGTH		21

#define PARSER_EEPROM_READ_AND_PRINT	1
#define PARSER_EEPROM_READ_TO_FILE	2
#define PARSER_EEPROM_WRITE_DEFAULT	3
#define PARSER_EEPROM_WRITE_FROM_FILE	4
#define PARSER_EEPROM_UPDATE_USER_SN	5
#define PARSER_EEPROM_ERASE		6

#define LSBOOTMODE_NSHOWID	0
#define LSBOOTMODE_SHOWID	1

#define LIBBCU_OK				(0)
#define LIBBCU_ERR_NO_THIS_BOARD		(1)
#define LIBBCU_ERR_BUILD_DEVICE_LINK		(2)
#define LIBBCU_ERR_OPEN_FTDI_CHANNEL		(3)
#define LIBBCU_ERR_NO_BOOT_MODE_OPT		(4)
#define LIBBCU_ERR_NO_BOOT_MODE_CFG		(5)
#define LIBBCU_ERR_NO_BOOT_CFG_CFG		(6)
#define LIBBCU_ERR_BOOT_MODE_OFFSET		(7)
#define LIBBCU_ERR_SET_GPIO_FAIL		(8)
#define LIBBCU_ERR_SET_BOOTMODE_FAIL		(9)
#define LIBBCU_ERR_INIT_FAIL			(10)
#define LIBBCU_ERR_RESET_FAIL			(11)
#define LIBBCU_ERR_ONOFF_FAIL			(12)
#define LIBBCU_ERR_BOOT_FROM_SWITCH		(14)
#define LIBBCU_ERR_NO_REMOTE_EN			(15)
#define LIBBCU_ERR_GET_BOOT_CFG			(16)
#define LIBBCU_ERR_GET_BOOT_MODE		(17)
#define LIBBCU_ERR_NO_GET_OR_SET		(18)
#define LIBBCU_ERR_NO_SET_VAL_OPT		(19)
#define LIBBCU_ERR_INVALID_GPIO_NAME_OPT	(20)
#define LIBBCU_ERR_FIND_GPIO_PATH		(21)
#define LIBBCU_ERR_GET_GPIO_LEVEL		(22)
#define LIBBCU_ERR_EEPROM_READ			(23)
#define LIBBCU_ERR_EEPROM_WRITE_DEFAULT		(24)
#define LIBBCU_ERR_EEPROM_UPDATE_USRSN		(25)
#define LIBBCU_ERR_EEPROM_UPDATE_BREV		(26)
#define LIBBCU_ERR_EEPROM_UPDATE_SREV		(27)
#define LIBBCU_ERR_EEPROM_ERASE			(28)
#define LIBBCU_ERR_EEPROM_NO_OPT		(29)
#define LIBBCU_ERR_UNSPPORTED_GPIO		(30)
#define LIBBCU_ERR_CREATE_MONITOR_THREAD	(31)
#define LIBBCU_ERR_MUTEX_LOCK_INIT		(32)
#define LIBBCU_ERR_MOT_OPEN_FILE		(33)
#define LIBBCU_ERR_MOT_GROUP_PARSE		(34)
#define LIBBCU_ERR_MOT_PAC1934_CANNOT_ACCESS	(35)
#define LIBBCU_ERR_MOT_NO_SR			(36)
#define LIBBCU_ERR_YML_OPEN_FILE		(37)
#define LIBBCU_ERR_YML_PARSER			(38)
#define LIBBCU_WARN_YML_OLD			(39)
#define LIBBCU_ERR_YML_READ			(40)

struct options_setting {
	int debug;
	char board[100]; //indicating the model of the board, i.e. i.MX8QMEVK
	int auto_find_board;
	int delay;
	int hold;
	int output_state;
	char boot_mode_name[MAX_MAPPING_NAME_LENGTH];
	int boot_mode_hex;
	int boot_config_hex[MAX_BOOT_CONFIG_BYTE];
	int get_or_set;
	int active_low;
	char path[MAX_PATH_LENGTH];
	char gpio_name[MAX_MAPPING_NAME_LENGTH];
	char location_id_string[MAX_LOCATION_ID_LENGTH];
	int dump;
	char *dumpname;
	int force;
	int pmt;
	int nodisplay;
	int refreshms;
	int use_rms;
	int rangefixed;
	int use_hwfilter;
	int use_unipolar;
	int dump_statistics;
	int eeprom_function;
	short eeprom_usr_sn;
	unsigned char eeprom_board_rev[3];
	unsigned char eeprom_soc_rev[3];
	int download_doc;
	char groups[MAX_NUMBER_OF_POWER * MAX_MAPPING_NAME_LENGTH];
};

typedef struct monitor_rail_power {
	char rail_name[MAX_MAPPING_NAME_LENGTH];
	double v_now;
	double v_avg;
	double v_min;
	double v_max;
	double c_now;
	double c_avg;
	double c_min;
	double c_max;
	double p_now;
	double p_avg;
	double p_min;
	double p_max;
} rail_info;

typedef struct monitor_group_power {
	char group_name[MAX_MAPPING_NAME_LENGTH];
	double p_now;
	double p_avg;
	double p_min;
	double p_max;
} group_info;

typedef struct monitor_power {
	rail_info rail_infos[MAX_NUMBER_OF_POWER];
	group_info group_infos[MAX_NUMBER_OF_GROUP];
	int rail_num;
	int group_num;
} powers;

struct monitor_thread_data
{
	int thread_id;
	int ret;
	int is_dump;
	char dumpname[50];
	int is_pmt;
	int is_stats;
	int is_rms;
	int is_hwfilter;
	int is_unipolar;
	int is_stop;
	char hot_key;
	powers monitor_powers;
	pthread_mutex_t mutex;
	struct options_setting *setting;
};

typedef struct bcu_eeprom_data
{
	char type[10];
	char ftdi_sn[10];
	char board_id[30];
	char board_rev[5];
	char soc_id[30];
	char soc_rev[5];
	char pmic_id[30];
	char pmic_rev[5];
	int rail_number;
	int user_sn;
} eeprom_informations;

char *bcu_get_err_str(int err_num);
void bcu_update_location_id(char *location_id);
void bcu_update_debug_level(int debug_level);
int bcu_get_boot_mode_hex(struct options_setting *setting);
int bcu_reset_time_ms(struct options_setting *setting);
int bcu_reset(struct options_setting *setting);
int bcu_onoff(struct options_setting *setting, int delay_us);
int bcu_init(struct options_setting *setting);
int bcu_deinit(struct options_setting *setting);
int bcu_monitor_perpare(struct options_setting *setting);
int bcu_monitor_set_hotkey(struct options_setting *setting, char hotkey);
int bcu_monitor_getvalue(struct options_setting *setting, powers *power_info);
int bcu_monitor_is_stop(void);
int bcu_monitor_get_err(void);
int bcu_monitor_unperpare(struct options_setting *setting);
int bcu_eeprom(struct options_setting *setting, eeprom_informations *eeprom_info);
int bcu_check_gpio_name(struct options_setting *setting);
int bcu_gpio(struct options_setting *setting);
int bcu_bootmode(struct options_setting *setting, int *bootcfglen);
int bcu_lsftdi(int is_auto, char boardlist[][MAX_MAPPING_NAME_LENGTH], char location_id_str[][MAX_LOCATION_ID_LENGTH]);
int bcu_lsboard(char boardlist[][MAX_MAPPING_NAME_LENGTH]);
int bcu_lsbootmode(struct options_setting *setting, char bootmodelist[][MAX_MAPPING_NAME_LENGTH]);
int bcu_lsgpio(struct options_setting *setting, char gpiolist[][MAX_MAPPING_NAME_LENGTH]);
int bcu_get_yaml_file_path(char *yamlfilepath);
int bcu_get_yaml_file(struct options_setting *setting, char *yamlfilepath);
void bcu_remove_all_ftdi_port(void);
int bcu_return_rail_name_max_len(struct options_setting *setting);

#endif