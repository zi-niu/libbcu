/*
* Copyright 2019-2021 NXP.
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
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "board.h"
#include "libbcu.h"

//x select channel of pca9548 chip, y select which sensor it is measuring 
//rsense use mOhm
#define IMX8XXL_POWER_PATH(channel, sensor, rsense1, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0x60;val_bitmask=0x40}/pca9548{channel="#channel";addr=0x70}/pac1934{group="#channel";sensor="#sensor";addr=0x10;rsense1="#rsense1";rsense2="#rsense2"}"

#define IMX8XXL_EXP_PATH(port,bitmask) "/ft4232h_i2c{channel=1;dir_bitmask=0x60;val_bitmask=0x40}/pca9548{channel=0;addr=0x70}/pca6416a{addr=0x20;port="#port";pin_bitmask="#bitmask";opendrain=1;}"
struct mapping imx8xxl[] = {
	{"on_board_5v0",power,IMX8XXL_POWER_PATH(1,1,10,10), 0x00},
	{"vdd_usb_3v3",power,IMX8XXL_POWER_PATH(1,2,1650,1650), 0x00},
	{"3v3_io",power,IMX8XXL_POWER_PATH(1,3, 1650, 1650), 0x00},
	{"3v3_enet",power,IMX8XXL_POWER_PATH(1,4, 3300,3300), 0x00},
	{"3v3_pmic_in",power,IMX8XXL_POWER_PATH(2,1,10,10), 0x00},
	{"on_board_3v3", power, IMX8XXL_POWER_PATH(2,2, 10, 10), 0x00},
	{"vdd_snvs_4p2",power,IMX8XXL_POWER_PATH(2,3,2000,2000), 0x00},
	{"vdd_main",power,IMX8XXL_POWER_PATH(2,4, 64, 69800), 0x00},
	{"vdd_memc",power,IMX8XXL_POWER_PATH(3,1, 54, 17400), 0x00},
	{"vdd_ddr_vddq",power,IMX8XXL_POWER_PATH(3,2, 514, 1430000), 0x00},
	{"ddr_vdd2",power,IMX8XXL_POWER_PATH(3,3, 10, 10), 0x00},
	{"vdd_enet0_1p8_3p3",power,IMX8XXL_POWER_PATH(3,4, 3320, 3320), 0x00},
	{"vdd_ana",power,IMX8XXL_POWER_PATH(4,1, 4, 97600), 0x00},
	{"ddr_vdd1",power,IMX8XXL_POWER_PATH(4,2, 10, 10), 0x00},
	{"1v8_1",power,IMX8XXL_POWER_PATH(4,3, 600, 600), 0x00},
	{"1v8_2",power,IMX8XXL_POWER_PATH(4,4, 1000, 1000), 0x00},
	{"1v8_3",power,IMX8XXL_POWER_PATH(5,1, 1000, 1000), 0x00},
	{"1v8_4",power,IMX8XXL_POWER_PATH(5,2, 680, 680), 0x00},
	{"1v8_5",power,IMX8XXL_POWER_PATH(5,3, 440, 440), 0x00},
	{"on_board_1v8",power,IMX8XXL_POWER_PATH(5,4,10, 10), 0x00},
	{"1v8_6",power,IMX8XXL_POWER_PATH(6,1, 10, 10), 0x00},
	{"ana1",power,IMX8XXL_POWER_PATH(6,2, 500, 500), 0x00},
	{"emmc0",power,IMX8XXL_POWER_PATH(6,3,3000,3000), 0x00},
	{"on_board_12v0",power,IMX8XXL_POWER_PATH(6,4, 10, 10), 0x00},

	{"boot_mode",gpio,IMX8XXL_EXP_PATH(0,0x07), 0x80},
	{"sd_pwr",gpio, IMX8XXL_EXP_PATH(0,0x08), 0x51},
	{"sd_wp",gpio, IMX8XXL_EXP_PATH(0,0x10), 0x61},
	{"sd_cd",gpio, IMX8XXL_EXP_PATH(0,0x20), 0x71},
	{"exp5_p06", gpio, IMX8XXL_EXP_PATH(0, 0x40), 0x00},
	{"exp5_p07", gpio, IMX8XXL_EXP_PATH(0, 0x80), 0x00},
	{"SR_vdd_main",gpio, IMX8XXL_EXP_PATH(1,0x01), 0x00},
	{"SR_vdd_memc",gpio, IMX8XXL_EXP_PATH(1,0x02), 0x00},
	{"SR_vdd_ddr_vddq",gpio, IMX8XXL_EXP_PATH(1,0x04), 0x00},
	{"SR_vdd_ana",gpio, IMX8XXL_EXP_PATH(1,0x08), 0x00},
	{"exp5_p14", gpio, IMX8XXL_EXP_PATH(1,0x10), 0x00},
	{"exp5_p15", gpio, IMX8XXL_EXP_PATH(1,0x20), 0x00},
	{"testmod_sel",gpio, IMX8XXL_EXP_PATH(1,0x40), 0x00},
	{"bootmode_sel",gpio, IMX8XXL_EXP_PATH(1,0x80), 0x90},

	{"reset",gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x11},
	{"jtag_sel",gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x01},
	{"onoff", gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x21},
	{"remote_en",gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x40},
	{"ft_reset", gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x31},
	{"pmic_stby", gpio, "/ft4232h_gpio{channel=3;pin_bitmask=0x20}", 0x00},

	{"at24cxx", i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0x60;val_bitmask=0x40}/at24cxx{addr=0x57;type=0x0;}", 0x00},
	// {"93lcx6", ftdi_eeprom , "/ft4232h_eeprom{uasize=0xFF}", 0x00},

	{NULL, 0, NULL, 0}//null terminated
};

struct mapping imx8xxl_board_c1[] = {
	{"on_board_5v0",	power, IMX8XXL_POWER_PATH(1, 1, 10, 10), 0x00},
	{"vdd_usb_3v3",		power, IMX8XXL_POWER_PATH(1, 2, 1650, 1650), 0x00},
	{"3v3_io",		power, IMX8XXL_POWER_PATH(1, 3, 1650, 1650), 0x00},
	{"3v3_enet",		power, IMX8XXL_POWER_PATH(1, 4, 3300, 3300), 0x00},
	{"3v3_pmic_in",		power, IMX8XXL_POWER_PATH(2, 1, 10, 10), 0x00},
	{"on_board_3v3",	power, IMX8XXL_POWER_PATH(2, 2, 10, 10), 0x00},
	{"vdd_snvs_4p2",	power, IMX8XXL_POWER_PATH(2, 3, 10000, 10000), 0x00},
	{"vdd_main",		power, IMX8XXL_POWER_PATH(2, 4, 64, 2000), 0x00},
	{"vdd_memc",		power, IMX8XXL_POWER_PATH(3, 1, 54, 2000), 0x00},
	{"vdd_ddr_vddq",	power, IMX8XXL_POWER_PATH(3, 2, 54, 2000), 0x00},
	{"ddr_vdd2",		power, IMX8XXL_POWER_PATH(3, 3, 50, 50), 0x00},
	{"vdd_enet0_1p8_3p3",	power, IMX8XXL_POWER_PATH(3, 4, 3320, 3320), 0x00},
	{"vdd_ana",		power, IMX8XXL_POWER_PATH(4, 1, 4, 2000), 0x00},
	{"ddr_vdd1",		power, IMX8XXL_POWER_PATH(4, 2, 50, 50), 0x00},
	{"1v8_1",		power, IMX8XXL_POWER_PATH(4, 3, 600, 600), 0x00},
	{"1v8_2",		power, IMX8XXL_POWER_PATH(4, 4, 1000, 1000), 0x00},
	{"1v8_3",		power, IMX8XXL_POWER_PATH(5, 1, 1000, 1000), 0x00},
	{"1v8_4",		power, IMX8XXL_POWER_PATH(5, 2, 680, 680), 0x00},
	{"1v8_5",		power, IMX8XXL_POWER_PATH(5, 3, 440, 440), 0x00},
	{"on_board_1v8",	power, IMX8XXL_POWER_PATH(5, 4, 10, 10), 0x00},
	{"1v8_6",		power, IMX8XXL_POWER_PATH(6, 1, 10, 10), 0x00},
	{"ana1",		power, IMX8XXL_POWER_PATH(6, 2, 500, 500), 0x00},
	{"emmc0",		power, IMX8XXL_POWER_PATH(6, 3, 3000, 3000), 0x00},
	{"on_board_12v0",	power, IMX8XXL_POWER_PATH(6, 4, 10, 10), 0x00},

	{"boot_mode",		gpio, IMX8XXL_EXP_PATH(0, 0x07), 0x80},
	{"sd_pwr",		gpio, IMX8XXL_EXP_PATH(0, 0x08), 0x51},
	{"sd_wp",		gpio, IMX8XXL_EXP_PATH(0, 0x10), 0x61},
	{"sd_cd",		gpio, IMX8XXL_EXP_PATH(0, 0x20), 0x71},
	{"exp5_p06",		gpio, IMX8XXL_EXP_PATH(0, 0x40), 0x00},
	{"exp5_p07",		gpio, IMX8XXL_EXP_PATH(0, 0x80), 0x00},
	{"SR_vdd_main",		gpio, IMX8XXL_EXP_PATH(1, 0x01), 0x00},
	{"SR_vdd_memc",		gpio, IMX8XXL_EXP_PATH(1, 0x02), 0x00},
	{"SR_vdd_ddr_vddq",	gpio, IMX8XXL_EXP_PATH(1, 0x04), 0x00},
	{"SR_vdd_ana",		gpio, IMX8XXL_EXP_PATH(1, 0x08), 0x00},
	{"exp5_p14",		gpio, IMX8XXL_EXP_PATH(1, 0x10), 0x00},
	{"exp5_p15",		gpio, IMX8XXL_EXP_PATH(1, 0x20), 0x00},
	{"testmod_sel",		gpio, IMX8XXL_EXP_PATH(1, 0x40), 0x00},
	{"bootmode_sel",	gpio, IMX8XXL_EXP_PATH(1, 0x80), 0x90},

	{"reset",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x11},
	{"jtag_sel",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x01},
	{"onoff",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x21},
	{"remote_en",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x40},
	{"ft_reset",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x31},
	{"pmic_stby",		gpio, "/ft4232h_gpio{channel=3;pin_bitmask=0x20}", 0x00},

	{"at24cxx",		i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0x60;val_bitmask=0x40}/at24cxx{addr=0x57;type=0x0;}", 0x00},

	{NULL, 0, NULL, 0} //null terminated
};

struct board_power_group imx8xxl_power_groups[] = {
	{"GROUP_SOC", "vdd_main,vdd_usb_3v3,3v3_enet,vdd_snvs_4p2,vdd_memc,vdd_ddr_vddq,vdd_ana,1v8_1,1v8_4,1v8_5"},
	{"GROUP_SOC_FULL", "vdd_main,vdd_usb_3v3,3v3_enet,vdd_snvs_4p2,vdd_memc,vdd_ddr_vddq,vdd_ana,1v8_1,1v8_4,1v8_5,3v3_io,vdd_enet0_1p8_3p3,1v8_2,1v8_3"},
	{"GROUP_DRAM", "ddr_vdd1,ddr_vdd2"},
	{"GROUP_PLATFORM", "on_board_12v0"},
	{NULL, 0}
};

struct mapping imx8dxl_ddr3[] = {
	{"on_board_12v0",power,IMX8XXL_POWER_PATH(1,1,10,10), 0x00},
	{"3v3_MICRO",power,IMX8XXL_POWER_PATH(1,2,10,10), 0x00},
	{"vdd_main",power,IMX8XXL_POWER_PATH(1,3, 50, 50), 0x00},
	{"vdd_memc",power,IMX8XXL_POWER_PATH(1,4, 50,50), 0x00},

	{"vdd_ddr_vddq",power,IMX8XXL_POWER_PATH(2,1,120,120), 0x00},
	{"vdd_ddr", power, IMX8XXL_POWER_PATH(2,2, 10, 10), 0x00},
	{"vdd_vtt",power,IMX8XXL_POWER_PATH(2,3,10,10), 0x00},
	{"vdd_ddr_pll",power,IMX8XXL_POWER_PATH(2,4, 1000, 1000), 0x00},

	{"boot_mode",gpio,IMX8XXL_EXP_PATH(0,0x07), 0x80},
	{"sd_pwr",gpio, IMX8XXL_EXP_PATH(0,0x08), 0x51},
	{"sd_wp",gpio, IMX8XXL_EXP_PATH(0,0x10), 0x61},
	{"sd_cd",gpio, IMX8XXL_EXP_PATH(0,0x20), 0x71},
	{"exp5_p06", gpio, IMX8XXL_EXP_PATH(0, 0x40), 0x00},
	{"exp5_p07", gpio, IMX8XXL_EXP_PATH(0, 0x80), 0x00},
	{"exp5_p14", gpio, IMX8XXL_EXP_PATH(1,0x10), 0x00},
	{"exp5_p15", gpio, IMX8XXL_EXP_PATH(1,0x20), 0x00},
	{"testmod_sel",gpio, IMX8XXL_EXP_PATH(,0x40), 0x00},
	{"bootmode_sel",gpio, IMX8XXL_EXP_PATH(1,0x80), 0x90},

	{"reset",gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x11},
	{"jtag_sel",gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x01},
	{"onoff", gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x21},
	{"remote_en",gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x40},
	{"ft_reset", gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x31},

	// you put all the pin in the imx8xxl board here
	//"sd_wp", gpio, "..."
	{NULL, 0, NULL, 0}//null terminated
};

struct boot_mode imx8xxl_boot_modes[] = {
	{"efuse", 0x00},
	{"usb", 0x01},
	{"emmc", 0x02},
	{"sd",0x03},
	{"nand", 0x04},
	{"m4_infinite_loop",0x05},
	{"spi", 0x06},
	{"dft_burnin_mode",0x07},
	NULL, 0
};

struct board_links imx8xxlevk_board_links[] = {
	"",
	""
};

#define IMX8MPEVK_POWER_SWITCH_PATH(group, sensor1, addr, rsense1, sensor2, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor1";addr="#addr";rsense1="#rsense1";sensor2="#sensor2";rsense2="#rsense2"}"
#define IMX8MPEVK_POWER_PATH(group, sensor, addr, rsense1, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor";addr="#addr";rsense1="#rsense1";rsense2="#rsense2"}"
#define IMX8MPEVK_GPIO_EXTENDER_PATH "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}"
struct mapping imx8mpevkpwr_board_a0[] = {
	{"vdd_arm", power, IMX8MPEVK_POWER_SWITCH_PATH(1, 1, 0x11, 20, 2, 5010), 0x00},
	{"nvcc_dram_1v1", power, IMX8MPEVK_POWER_SWITCH_PATH(1, 3, 0x11, 50, 4, 10050), 0x00},
	{"vsys_5v", power, IMX8MPEVK_POWER_SWITCH_PATH(2, 1, 0x12, 20, 2, 2020), 0x00},
	{"vdd_soc", power, IMX8MPEVK_POWER_SWITCH_PATH(2, 3, 0x12, 10, 4, 2010), 0x00},
	{"lpd4_vddq", power, IMX8MPEVK_POWER_SWITCH_PATH(3, 1, 0x13, 50, 2, 10050), 0x00},
	{"lpd4_vdd2", power, IMX8MPEVK_POWER_PATH(3, 3, 0x13, 50, 50), 0x00},
	{"nvcc_sd2", power, IMX8MPEVK_POWER_PATH(3, 4, 0x13, 1000, 1000), 0x00},
	{"vdd_lvds_1V8", power, IMX8MPEVK_POWER_PATH(4, 1, 0x14, 1000, 1000), 0x00},
	{"vdd_hdmi_1v8", power, IMX8MPEVK_POWER_PATH(4, 2, 0x14, 2000, 2000), 0x00},
	{"nvcc_snvs_1v8", power, IMX8MPEVK_POWER_PATH(4, 3, 0x14, 10000, 509000), 0x00},
	{"vdd_earc_1v8", power, IMX8MPEVK_POWER_PATH(4, 4, 0x14, 2000, 2000), 0x00},
	{"vdd_usb_1v8", power, IMX8MPEVK_POWER_PATH(5, 1, 0x15, 2000, 2000), 0x00},
	{"vdd_pci_1v8", power, IMX8MPEVK_POWER_PATH(5, 2, 0x15, 1000, 1000), 0x00},
	{"vdd_mipi_1v8", power, IMX8MPEVK_POWER_PATH(5, 3, 0x15, 2000, 2000), 0x00},
	{"vdd_pll_ana_1v8", power, IMX8MPEVK_POWER_PATH(5, 4, 0x15, 258, 10250), 0x00},
	{"nvcc_sd1", power, IMX8MPEVK_POWER_PATH(6, 1, 0x16, 100, 100), 0x00},
	{"lpd4_vdd1", power, IMX8MPEVK_POWER_PATH(6, 2, 0x16, 4990, 4990), 0x00},
	{"cpu_vdd_1v8", power, IMX8MPEVK_POWER_PATH(6, 3, 0x16, 100, 100), 0x00},
	{"bb_vdd_1v8", power, IMX8MPEVK_POWER_PATH(6, 4, 0x16, 50, 50), 0x00},
	{"vdd_pll_ana_0v8", power, IMX8MPEVK_POWER_PATH(7, 1, 0x17, 1000, 1000), 0x00},
	{"vdd_pci_0v8", power, IMX8MPEVK_POWER_PATH(7, 2, 0x17, 1000, 1000), 0x00},
	{"vdd_mipi_0v8", power, IMX8MPEVK_POWER_PATH(7, 3, 0x17, 1000, 1000), 0x00},
	{"vdd_hdmi_0v8", power, IMX8MPEVK_POWER_PATH(7, 4, 0x17, 1000, 1000), 0x00},
	{"vdd_usb_3v3", power, IMX8MPEVK_POWER_PATH(8, 1, 0x18, 1000, 1000), 0x00},
	{"vdd_usb_0v8", power, IMX8MPEVK_POWER_PATH(8, 2, 0x18, 400, 400), 0x00},
	{"vdd_sd1_3v3", power, IMX8MPEVK_POWER_PATH(8, 3, 0x18, 250, 250), 0x00},
	{"bb_vdd_3v3", power, IMX8MPEVK_POWER_PATH(8, 4, 0x18, 50, 50), 0x00},

	{"boot_mode", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x0F;opendrain=0;}", 0x30},
	{"SR_vsys_5v", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"SR_nvcc_snvs_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"SR_vdd_soc", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"SR_vdd_arm", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x80;opendrain=0;}", 0x00},
	{"remote_en", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"SR_vdd_pll_ana_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_nvcc_dram_1v1", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_lpd4_vddq", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"ft_io8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"ft_io9", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"ft_io10", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"ft_io11", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"reset_b",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x10}", 0x00},
	{"reset",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x21},
	{"io_nrst",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x11},
	{"onoff",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x00},
	{"io_nint",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x08}", 0x00},

	{"ft_gpio1",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},
	{"ft_gpio2",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x00},
	{"ft_gpio3",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x00},
	{"ft_gpio4",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x80}", 0x00},

	{"at24cxx", i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/at24cxx{addr=0x50;type=0x1;}", 0x00},

	{NULL, 0, NULL, 0}//null terminated
};

struct mapping imx8mpevkpwr_board_a1[] = {
	{"vdd_arm", power, IMX8MPEVK_POWER_SWITCH_PATH(1, 1, 0x11, 20, 2, 5010), 0x00},
	{"nvcc_dram_1v1", power, IMX8MPEVK_POWER_SWITCH_PATH(1, 3, 0x11, 50, 4, 2050), 0x00},
	{"vsys_5v", power, IMX8MPEVK_POWER_SWITCH_PATH(2, 1, 0x12, 20, 2, 420), 0x00},
	{"vdd_soc", power, IMX8MPEVK_POWER_SWITCH_PATH(2, 3, 0x12, 10, 4, 1010), 0x00},
	{"lpd4_vddq", power, IMX8MPEVK_POWER_SWITCH_PATH(3, 1, 0x13, 50, 2, 2050), 0x00},
	{"lpd4_vdd2", power, IMX8MPEVK_POWER_PATH(3, 3, 0x13, 50, 50), 0x00},
	{"nvcc_sd2", power, IMX8MPEVK_POWER_PATH(3, 4, 0x13, 1000, 1000), 0x00},
	{"vdd_lvds_1V8", power, IMX8MPEVK_POWER_PATH(4, 1, 0x14, 1000, 1000), 0x00},
	{"vdd_hdmi_1v8", power, IMX8MPEVK_POWER_PATH(4, 2, 0x14, 2000, 2000), 0x00},
	{"nvcc_snvs_1v8", power, IMX8MPEVK_POWER_PATH(4, 3, 0x14, 10000, 509000), 0x00},
	{"vdd_earc_1v8", power, IMX8MPEVK_POWER_PATH(4, 4, 0x14, 2000, 2000), 0x00},
	{"vdd_usb_1v8", power, IMX8MPEVK_POWER_PATH(5, 1, 0x15, 2000, 2000), 0x00},
	{"vdd_pci_1v8", power, IMX8MPEVK_POWER_PATH(5, 2, 0x15, 1000, 1000), 0x00},
	{"vdd_mipi_1v8", power, IMX8MPEVK_POWER_PATH(5, 3, 0x15, 2000, 2000), 0x00},
	{"vdd_pll_ana_1v8", power, IMX8MPEVK_POWER_PATH(5, 4, 0x15, 1008, 11000), 0x00},
	{"nvcc_sd1", power, IMX8MPEVK_POWER_PATH(6, 1, 0x16, 100, 100), 0x00},
	{"lpd4_vdd1", power, IMX8MPEVK_POWER_PATH(6, 2, 0x16, 4990, 4990), 0x00},
	{"cpu_vdd_1v8", power, IMX8MPEVK_POWER_PATH(6, 3, 0x16, 100, 100), 0x00},
	{"bb_vdd_1v8", power, IMX8MPEVK_POWER_PATH(6, 4, 0x16, 50, 50), 0x00},
	{"vdd_pll_ana_0v8", power, IMX8MPEVK_POWER_PATH(7, 1, 0x17, 1000, 1000), 0x00},
	{"vdd_pci_0v8", power, IMX8MPEVK_POWER_PATH(7, 2, 0x17, 400, 400), 0x00},
	{"vdd_mipi_0v8", power, IMX8MPEVK_POWER_PATH(7, 3, 0x17, 1000, 1000), 0x00},
	{"vdd_hdmi_0v8", power, IMX8MPEVK_POWER_PATH(7, 4, 0x17, 1000, 1000), 0x00},
	{"vdd_usb_3v3", power, IMX8MPEVK_POWER_PATH(8, 1, 0x18, 1000, 1000), 0x00},
	{"vdd_usb_0v8", power, IMX8MPEVK_POWER_PATH(8, 2, 0x18, 400, 400), 0x00},
	{"vdd_sd1_3v3", power, IMX8MPEVK_POWER_PATH(8, 3, 0x18, 250, 250), 0x00},
	{"bb_vdd_3v3", power, IMX8MPEVK_POWER_PATH(8, 4, 0x18, 50, 50), 0x00},

	{"boot_mode", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x0F;opendrain=0;}", 0x30},
	{"SR_vsys_5v", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"SR_nvcc_snvs_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"SR_vdd_soc", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"SR_vdd_arm", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x80;opendrain=0;}", 0x00},
	{"remote_en", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"SR_vdd_pll_ana_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_nvcc_dram_1v1", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_lpd4_vddq", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"ft_io8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"ft_io9", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"ft_io10", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"ft_io11", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"reset_b",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x10}", 0x00},
	{"reset",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x21},
	{"io_nrst",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x11},
	{"onoff",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x00},
	{"io_nint",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x08}", 0x00},

	{"ft_gpio1",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},
	{"ft_gpio2",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x00},
	{"ft_gpio3",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x00},
	{"ft_gpio4",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x80}", 0x00},

	{"at24cxx", i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/at24cxx{addr=0x50;type=0x1;}", 0x00},

	{NULL, 0, NULL, 0}//null terminated
};

struct board_power_group imx8mpevkpwr_power_groups[] = {
	{"GROUP_SOC", "vdd_arm,vdd_soc,nvcc_snvs_1v8,vdd_pll_ana_0v8,vdd_pll_ana_1v8,nvcc_dram_1v1,vdd_hdmi_0v8,vdd_hdmi_1v8,vdd_mipi_0v8,vdd_mipi_1v8,vdd_pci_0v8,vdd_pci_1v8,vdd_usb_0v8,vdd_usb_1v8,vdd_usb_3v3,vdd_lvds_1V8,vdd_earc_1v8"},
	{"GROUP_SOC_FULL", "vdd_arm,vdd_soc,nvcc_snvs_1v8,vdd_pll_ana_0v8,vdd_pll_ana_1v8,nvcc_dram_1v1,vdd_hdmi_0v8,vdd_hdmi_1v8,vdd_mipi_0v8,vdd_mipi_1v8,vdd_pci_0v8,vdd_pci_1v8,vdd_usb_0v8,vdd_usb_1v8,vdd_usb_3v3,vdd_lvds_1V8,vdd_earc_1v8,cpu_vdd_1v8,nvcc_sd1,nvcc_sd2"},
	{"GROUP_DRAM", "lpd4_vdd1,lpd4_vdd2,lpd4_vddq"},
	{"GROUP_PLATFORM", "vsys_5v"},
	{NULL, 0}
};

struct mapping imx8mpevk_board[] = {
	{"boot_mode", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x0F;opendrain=0;}", 0x30},
	{"SR_vsys_5v", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"SR_nvcc_snvs_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"SR_vdd_soc", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"SR_vdd_arm", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x80;opendrain=0;}", 0x00},
	{"remote_en", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"SR_vdd_pll_ana_1v8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_nvcc_dram_1v1", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_lpd4_vddq", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"ft_io8", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"ft_io9", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"ft_io10", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"ft_io11", gpio, IMX8MPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"reset_b",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x10}", 0x00},
	{"reset",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x21},
	{"io_nrst",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x11},
	{"onoff",gpio,"/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x00},
	{"io_nint",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x08}", 0x00},

	{"ft_gpio1",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},
	{"ft_gpio2",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x00},
	{"ft_gpio3",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x00},
	{"ft_gpio4",gpio,"/ft4232h_gpio{channel=1;pin_bitmask=0x80}", 0x00},

	{NULL, 0, NULL, 0}//null terminated
};

#define IMX8MPVBD_POWER_SWITCH_PATH(channel, group, sensor1, addr, rsense1, sensor2, rsense2) "/ft4232h_i2c{channel="#channel";dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor1";addr="#addr";rsense1="#rsense1";sensor2="#sensor2";rsense2="#rsense2"}"
#define IMX8MPVBD_POWER_PATH(channel, group, sensor, addr, rsense1, rsense2) "/ft4232h_i2c{channel="#channel";dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor";addr="#addr";rsense1="#rsense1";rsense2="#rsense2"}"
#define IMX8MPVBD_GPIO_EXTENDER_PATH(channel, addr, port, pin_bitmask) "/ft4232h_i2c{channel="#channel";dir_bitmask=0xF0;val_bitmask=0x00}/pcal6524h{addr="#addr";port="#port";pin_bitmask="#pin_bitmask";opendrain=0;}"
struct mapping imx8mpddr4_board[] = {
	{"nvcc_snvs_1v8_cpu", power, IMX8MPVBD_POWER_PATH(0, 1, 1, 0x10, 100, 100), 0x00},
	{"vdda_1v8_3", power, IMX8MPVBD_POWER_PATH(0, 1, 2, 0x10, 100, 100), 0x00},
	{"nvcc_sd2", power, IMX8MPVBD_POWER_PATH(0, 1, 3, 0x10, 100, 100), 0x00},
	{"vdda_1v8_4", power, IMX8MPVBD_POWER_PATH(0, 1, 4, 0x10, 100, 100), 0x00},
	{"vdd_arm", power, IMX8MPVBD_POWER_SWITCH_PATH(0, 2, 1, 0x11, 20, 2, 2020), 0x00},
	{"vpp_2v5", power, IMX8MPVBD_POWER_PATH(0, 2, 3, 0x11, 100, 100), 0x00},
	{"vdda_1v8_2", power, IMX8MPVBD_POWER_SWITCH_PATH(0, 3, 2, 0x14, 1000, 1, 11000), 0x00},
	{"vdda_1v8_0", power, IMX8MPVBD_POWER_SWITCH_PATH(0, 3, 4, 0x14, 1000, 3, 11000), 0x00},
	{"vdda_1v8_1", power, IMX8MPVBD_POWER_SWITCH_PATH(0, 4, 2, 0x16, 1000, 1, 11000), 0x00},
	{"nvcc_dram_1v2", power, IMX8MPVBD_POWER_SWITCH_PATH(0, 4, 4, 0x16, 50, 3, 2000), 0x00},
	{"vdd08_phy_pll_1", power, IMX8MPVBD_POWER_SWITCH_PATH(1, 5, 2, 0x12, 1000, 1, 11000), 0x00},
	{"vdd_3v3", power, IMX8MPVBD_POWER_PATH(1, 5, 3, 0x12, 100, 100), 0x00},
	{"vdd_3v3_cpu", power, IMX8MPVBD_POWER_PATH(1, 5, 4, 0x12, 100, 100), 0x00},
	{"vdd_1v8_cpu_0", power, IMX8MPVBD_POWER_PATH(1, 6, 1, 0x13, 100, 100), 0x00},
	{"vdd_1v8_cpu_1", power, IMX8MPVBD_POWER_PATH(1, 6, 2, 0x13, 100, 100), 0x00},
	{"vdd_1v8", power, IMX8MPVBD_POWER_PATH(1, 6, 3, 0x13, 100, 100), 0x00},
	{"dcdc_5v_pmic", power, IMX8MPVBD_POWER_PATH(1, 6, 4, 0x13, 10, 10), 0x00},
	{"vdd08_phy_pll_0", power, IMX8MPVBD_POWER_PATH(1, 7, 1, 0x15, 100, 100), 0x00},
	{"vdd_nand_cpu", power, IMX8MPVBD_POWER_PATH(1, 7, 2, 0x15, 100, 100), 0x00},
	{"vdd_soc", power, IMX8MPVBD_POWER_SWITCH_PATH(1, 8, 3, 0x17, 10, 1, 1010), 0x00},
	{"vdd08_phy_pll_2", power, IMX8MPVBD_POWER_SWITCH_PATH(1, 8, 2, 0x12, 1000, 4, 11000), 0x00},

	{"SR_vdd_arm", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x01), 0x00},
	{"SR_nvcc_dram_1v2", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x02), 0x00},
	{"SR_vdda_1v8_1", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x04), 0x00},
	{"SR_vdda_1v8_2", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x08), 0x00},
	{"SR_vdda_1v8_0", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x10), 0x00},
	{"SR_vdd_soc", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x20), 0x00},
	{"SR_vdd08_phy_pll_2", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x40), 0x00},
	{"SR_vdd08_phy_pll_1", gpio, IMX8MPVBD_GPIO_EXTENDER_PATH(0, 0x22, 1, 0x80), 0x00},

	// {"at24cxx", i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/at24cxx{addr=0x50;}", 0x00},

	{NULL, 0, NULL, 0}//null terminated
};

struct boot_mode imx8mpevk_board_boot_modes[] = {
	{"fuse", 0x00},
	{"usb", 0x01},
	{"emmc", 0x02},
	{"sd",0x03},
	{"nand_256", 0x04},
	{"nand_512",0x05},
	{"qspi_3b_read", 0x06},
	{"qspi_hyperflash",0x07},
	{"ecspi",0x08},
	{"infinite_loop",0x0E},
	{NULL, 0}
};

struct board_links imx8mpevk_board_links[] = {
	"http://shlinux22.ap.freescale.net/internal-only/Linux_IMX_Core/latest/common_bsp/imx-boot/imx-boot-imx8mpevk-sd.bin-flash_evk",
	""
};

#define IMX8ULPEVK_POWER_SWITCH_PATH(group, sensor1, addr, rsense1, sensor2, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0xF0}/pac1934{group="#group";sensor="#sensor1";addr="#addr";rsense1="#rsense1";sensor2="#sensor2";rsense2="#rsense2"}"
#define IMX8ULPEVK_POWER_PATH(group, sensor, addr, rsense1, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0xF0}/pac1934{group="#group";sensor="#sensor";addr="#addr";rsense1="#rsense1";rsense2="#rsense2"}"
#define IMX8ULPEVK_GPIO_EXTENDER_PATH "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0xF0}"
struct mapping imx8ulpevk_board[] = {
	{"buck4_cpu_1v1", power, IMX8ULPEVK_POWER_SWITCH_PATH(1, 1, 0x10, 400, 2, 400), 0x00},
	{"buck1_cpu_1v8", power, IMX8ULPEVK_POWER_SWITCH_PATH(1, 3, 0x10, 100, 4, 10000), 0x00},
	{"buck2_cpu_1v0", power, IMX8ULPEVK_POWER_SWITCH_PATH(2, 1, 0x11, 50, 2, 10000), 0x00},
	{"buck3_cpu_1v0", power, IMX8ULPEVK_POWER_SWITCH_PATH(2, 3, 0x11, 20, 4, 10000), 0x00},
	{"ldo5_cpu_3v0", power, IMX8ULPEVK_POWER_PATH(3, 1, 0x12, 250000, 250000), 0x00},
	{"ldo1_cpu_1v1", power, IMX8ULPEVK_POWER_PATH(3, 2, 0x12, 100, 100), 0x00},
	{"buck1_lsw1_cpu_1v8", power, IMX8ULPEVK_POWER_PATH(3, 3, 0x12, 100, 100), 0x00},
	{"buck1_lsw4_cpu_1v8", power, IMX8ULPEVK_POWER_PATH(3, 4, 0x12, 100, 100), 0x00},
	{"buck1_lsw2_cpu_1v8", power, IMX8ULPEVK_POWER_PATH(4, 1, 0x13, 100, 100), 0x00},
	{"buck1_lsw3_cpu_1v8", power, IMX8ULPEVK_POWER_PATH(4, 2, 0x13, 100, 100), 0x00},
	{"ldo4_cpu_1v8", power, IMX8ULPEVK_POWER_PATH(4, 3, 0x13, 100, 100), 0x00},
	{"ldo2_cpu_3v3", power, IMX8ULPEVK_POWER_PATH(4, 4, 0x13, 100, 100), 0x00},
	{"vsys_5v0_4v2", power, IMX8ULPEVK_POWER_SWITCH_PATH(5, 1, 0x14, 20, 2, 10000), 0x00},
	{"ldo1_cpu_1v1_0v6", power, IMX8ULPEVK_POWER_PATH(5, 3, 0x14, 50, 50), 0x00},
	{"buck4_dram_1v1", power, IMX8ULPEVK_POWER_PATH(5, 4, 0x14, 50, 50), 0x00},

	{"boot_mode", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0xC0;opendrain=0;}", 0x30},
	{"boot_cfg0", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=0;pin_bitmask=0x3F;opendrain=0;}", 0x00},
	{"remote_en", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"reset", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x02;opendrain=0;}", 0x10},
	{"onoff", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x04;opendrain=0;}", 0x20},
	{"reset0_b", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"reset1_b", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"cpu_power_mode0", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"cpu_power_mode1", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"cpu_power_mode2", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x20;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"SR_buck1_cpu_1v8", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x21;port=0;pin_bitmask=0x01;opendrain=0;}", 0x00},
	{"SR_buck4_cpu_1v1", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x21;port=0;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_buck2_cpu_1v0", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x21;port=0;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_buck3_cpu_1v0", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x21;port=0;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"SR_vsys_5v0_4v2", gpio, IMX8ULPEVK_GPIO_EXTENDER_PATH"/pca6416a{addr=0x21;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},

	{"ft_int_b", gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x08}", 0x00},
	{"ft_reset_boot_mode", gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},

	// {"at24cxx", i2c_eeprom, "/ft4232h_i2c{channel=1;dir_bitmask=0xF8;val_bitmask=0x00}/at24cxx{addr=0x53;type=0x1;}", 0x00},
	{"93lcx6", ftdi_eeprom, "/ft4232h_eeprom{uasize=0xFF}", 0x00},

	{NULL, 0, NULL, 0} //null terminated
};

struct boot_mode imx8ulpevk_board_boot_modes[] = {
	{"fuse",	0x00},
	{"usb",		0x01},
	{"emmc_s",	0x02},
	{"emmc_nor_lp",	0x02},
	{"emmc_nor",	0x02},
	{"nand_nor",	0x02},
	{"nor_s",	0x02},
	{"nor_nor_lp",	0x02},
	{"nor_nor",	0x02},
	{NULL,		0}
};

struct boot_config imx8ulpevk_board_boot_config[] = {
	{"fuse",	{0x00}},
	{"usb",		{0x00}},
	{"emmc_s",	{0x00}},
	{"emmc_nor_lp",	{0x01}},
	{"emmc_nor",	{0x02}},
	{"nand_nor",	{0x12}},
	{"nor_s",	{0x20}},
	{"nor_nor_lp",	{0x21}},
	{"nor_nor",	{0x22}},
	{NULL,		{0}}
};

struct board_power_group imx8ulpevkpwr_power_groups[] = {
	{"GROUP_SOC_FULL", "buck1_cpu_1v8,buck2_cpu_1v0,buck3_cpu_1v0,buck4_cpu_1v1,ldo1_cpu_1v1,ldo1_cpu_1v1_0v6,buck1_lsw2_cpu_1v8,buck1_lsw3_cpu_1v8,ldo5_cpu_3v0,ldo2_cpu_3v3,buck1_lsw1_cpu_1v8,ldo4_cpu_1v8,buck1_lsw4_cpu_1v8"},
	{"GROUP_PLATFORM", "vsys_5v0_4v2"},
	{NULL, 0}
};

#define VAL_BOARD_1_POWER_SWITCH_PATH(group, sensor1, addr, rsense1, sensor2, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor1";addr="#addr";rsense1="#rsense1";sensor2="#sensor2";rsense2="#rsense2"}"
#define VAL_BOARD_1_POWER_PATH(group, sensor, addr, rsense1, rsense2) "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}/pac1934{group="#group";sensor="#sensor";addr="#addr";rsense1="#rsense1";rsense2="#rsense2"}"
#define VAL_BOARD_1_GPIO_EXTENDER_PATH "/ft4232h_i2c{channel=1;dir_bitmask=0xF0;val_bitmask=0x00}"
struct mapping val_board_1[] = {
	{"vbus_in_dcdc",	power, VAL_BOARD_1_POWER_PATH(1, 1, 0x10, 10, 10), 0x00},
	{"dcdc_5v",		power, VAL_BOARD_1_POWER_PATH(1, 2, 0x10, 10, 10), 0x00},
	{"vdd_5v",		power, VAL_BOARD_1_POWER_PATH(1, 3, 0x10, 20, 20), 0x00},
	{"vddext_3v3",		power, VAL_BOARD_1_POWER_PATH(1, 4, 0x10, 20, 20), 0x00},
	{"vdd_arm",		power, VAL_BOARD_1_POWER_SWITCH_PATH(2, 1, 0x11, 20, 2, 5010), 0x00},
	{"nvcc_dram_1v1",	power, VAL_BOARD_1_POWER_SWITCH_PATH(2, 3, 0x11, 50, 4, 2050), 0x00},
	{"vsys_5v",		power, VAL_BOARD_1_POWER_SWITCH_PATH(3, 1, 0x12, 20, 2, 420), 0x00},
	{"vdd_soc",		power, VAL_BOARD_1_POWER_SWITCH_PATH(3, 3, 0x12, 20, 4, 1020), 0x00},
	{"vqspi_1v8",		power, VAL_BOARD_1_POWER_PATH(4, 1, 0x13, 1000, 1000), 0x00},
	{"lpd4_vddq",		power, VAL_BOARD_1_POWER_PATH(4, 2, 0x13, 100, 100), 0x00},
	{"lpd4_vdd2",		power, VAL_BOARD_1_POWER_PATH(4, 3, 0x13, 50, 50), 0x00},
	{"nvcc_sd2",		power, VAL_BOARD_1_POWER_PATH(4, 4, 0x13, 1000, 1000), 0x00},
	{"vdd_phy_0v9",		power, VAL_BOARD_1_POWER_PATH(5, 1, 0x14, 1000, 1000), 0x00},
	{"vdd_phy_1v2",		power, VAL_BOARD_1_POWER_PATH(5, 2, 0x14, 1000, 1000), 0x00},
	{"nvcc_snvs_1v8",	power, VAL_BOARD_1_POWER_PATH(5, 3, 0x14, 10000, 509000), 0x00},
	{"vdd_snvs_0v8",	power, VAL_BOARD_1_POWER_PATH(5, 4, 0x14, 10000, 509000), 0x00},
	{"vdd_usb_1v8",		power, VAL_BOARD_1_POWER_PATH(6, 1, 0x15, 2000, 2000), 0x00},
	{"vdd_pci_1v8",		power, VAL_BOARD_1_POWER_PATH(6, 2, 0x15, 1000, 1000), 0x00},
	{"vdd_mipi_1v8",	power, VAL_BOARD_1_POWER_PATH(6, 3, 0x15, 2000, 2000), 0x00},
	{"vdd_pll_ana_1v8",	power, VAL_BOARD_1_POWER_PATH(6, 4, 0x15, 1000, 1000), 0x00},
	{"nvcc_sd1",		power, VAL_BOARD_1_POWER_PATH(7, 1, 0x16, 100, 100), 0x00},
	{"lpd4_vdd1",		power, VAL_BOARD_1_POWER_PATH(7, 2, 0x16, 4990, 4990), 0x00},
	{"cpu_vdd_1v8",		power, VAL_BOARD_1_POWER_PATH(7, 3, 0x16, 100, 100), 0x00},
	{"bb_vdd_1v8",		power, VAL_BOARD_1_POWER_PATH(7, 4, 0x16, 50, 50), 0x00},
	{"vdd_pll_ana_0v8",	power, VAL_BOARD_1_POWER_PATH(8, 1, 0x17, 1000, 1000), 0x00},
	{"vdd_pci_0v8",		power, VAL_BOARD_1_POWER_PATH(8, 2, 0x17, 400, 400), 0x00},
	{"cpu_vdd_3v3",		power, VAL_BOARD_1_POWER_PATH(8, 3, 0x17, 1000, 1000), 0x00},
	{"vccq_sd1",		power, VAL_BOARD_1_POWER_PATH(8, 4, 0x17, 100, 100), 0x00},
	{"vdd_usb_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 1, 0x18, 1000, 1000), 0x00},
	{"vdd_usb_0v8",		power, VAL_BOARD_1_POWER_PATH(9, 2, 0x18, 400, 400), 0x00},
	{"vdd_sd1_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 3, 0x18, 250, 250), 0x00},
	{"bb_vdd_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 4, 0x18, 50, 50), 0x00},
	{"vdd_gpu",		power, VAL_BOARD_1_POWER_SWITCH_PATH(10, 1, 0x19, 100, 3, 10100), 0x00},
	{"vdd_vpu",		power, VAL_BOARD_1_POWER_SWITCH_PATH(10, 2, 0x19, 50, 4, 10050), 0x00},
	{"vdd_dram",		power, VAL_BOARD_1_POWER_SWITCH_PATH(11, 1, 0x1A, 50, 2, 10050), 0x00},
	{"vdd_dram_pll_0v8",	power, VAL_BOARD_1_POWER_PATH(11, 3, 0x1A, 250, 250), 0x00},
	{"nvcc_enet",		power, VAL_BOARD_1_POWER_PATH(11, 4, 0x1A, 100, 100), 0x00},

	{"boot_mode",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x0F;opendrain=0;}", 0x30},
	{"SR_vsys_5v",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"SR_nvcc_snvs_1v8",	gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"SR_vdd_snvs_0v8",     gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00}, // shares SR_ pin with SR_nvcc_snvs_1v8
	{"SR_vdd_soc",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"SR_vdd_vpu",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x80;opendrain=0;}", 0x00},
	{"remote_en",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"SR_vdd_gpu",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_vdd_dram",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_vdd_arm",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"SR_nvcc_dram_1v1",	gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"ft_io9",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"ft_io10",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"ft_io11",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"boot_cfg0",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x23;port=0;pin_bitmask=0xFF;opendrain=0;}", 0x00},
	{"boot_cfg1",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x23;port=1;pin_bitmask=0xFF;opendrain=0;}", 0x00},

	{"reset_b",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x10}", 0x00},
	{"reset",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x21},
	{"io_nrst",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x11},
	{"onoff",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x00},

	{"ft_gpio1",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},
	{"ft_gpio2",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x00},
	{"ft_gpio3",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x00},
	{"io_nint",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x80}", 0x00},

	{"93lcx6",		ftdi_eeprom, "/ft4232h_eeprom{uasize=0xFF}", 0x00},

	{NULL, 0, NULL, 0} //null terminated
};

struct board_power_group val_board_1_power_groups[] = {
	{"GROUP_SOC",		"nvcc_snvs_1v8,vdd_snvs_0v8,vdd_soc,vdd_pll_ana_0v8,vdd_pci_0v8,vdd_usb_0v8,vdd_vpu,vdd_gpu,vdd_dram,vdd_dram_pll_0v8,vdd_phy_0v9,vdd_arm,vdd_pll_ana_1v8,vdd_usb_1v8,vdd_pci_1v8,vdd_mipi_1v8,nvcc_dram_1v1,vdd_usb_3v3,vdd_phy_1v2"},
	{"GROUP_SOC_FULL",	"nvcc_snvs_1v8,vdd_snvs_0v8,vdd_soc,vdd_pll_ana_0v8,vdd_pci_0v8,vdd_usb_0v8,vdd_vpu,vdd_gpu,vdd_dram,vdd_dram_pll_0v8,vdd_phy_0v9,vdd_arm,vdd_pll_ana_1v8,vdd_usb_1v8,vdd_pci_1v8,vdd_mipi_1v8,nvcc_dram_1v1,vdd_usb_3v3,vdd_phy_1v2,cpu_vdd_1v8,nvcc_sd1,nvcc_sd2,nvcc_enet,cpu_vdd_3v3"},
	{"GROUP_DRAM",		"lpd4_vdd1,lpd4_vdd2,lpd4_vddq"},
	{"GROUP_PLATFORM",	"vbus_in_dcdc"},
	{NULL, 0}
};

struct boot_mode val_board_1_boot_modes[] = {
	{"usb",		0x05},
	{"emmc",	0x06},
	{"sd",		0x06},
	{NULL,		0}
};

struct boot_config val_board_1_boot_config[] = {
	{"usb",		{0x00, 0x00}},
	{"emmc",	{0x20, 0x28}},
	{"sd",		{0x10, 0x14}},
	{NULL,		{0}}
};

struct mapping val_board_2[] = {
	{"vbus_in_dcdc",	power, VAL_BOARD_1_POWER_PATH(1, 1, 0x10, 10, 10), 0x00},
	{"dcdc_5v",		power, VAL_BOARD_1_POWER_PATH(1, 2, 0x10, 10, 10), 0x00},
	{"vdd_5v",		power, VAL_BOARD_1_POWER_PATH(1, 3, 0x10, 20, 20), 0x00},
	{"vddext_3v3",		power, VAL_BOARD_1_POWER_PATH(1, 4, 0x10, 20, 20), 0x00},
	{"vdd_arm",		power, VAL_BOARD_1_POWER_SWITCH_PATH(2, 1, 0x11, 20, 2, 5010), 0x00},
	{"nvcc_dram_1v1",	power, VAL_BOARD_1_POWER_SWITCH_PATH(2, 3, 0x11, 50, 4, 2050), 0x00},
	{"vsys_5v",		power, VAL_BOARD_1_POWER_SWITCH_PATH(3, 1, 0x12, 20, 2, 420), 0x00},
	{"vdd_soc",		power, VAL_BOARD_1_POWER_SWITCH_PATH(3, 3, 0x12, 20, 4, 1020), 0x00},
	{"vqspi_1v8",		power, VAL_BOARD_1_POWER_PATH(4, 1, 0x13, 1000, 1000), 0x00},
	{"lpd4_vddq",		power, VAL_BOARD_1_POWER_PATH(4, 2, 0x13, 100, 100), 0x00},
	{"lpd4_vdd2",		power, VAL_BOARD_1_POWER_PATH(4, 3, 0x13, 50, 50), 0x00},
	{"nvcc_sd2",		power, VAL_BOARD_1_POWER_PATH(4, 4, 0x13, 1000, 1000), 0x00},
	{"vdd_phy_0v9",		power, VAL_BOARD_1_POWER_PATH(5, 1, 0x14, 1000, 1000), 0x00},
	{"vdd_phy_1v2",		power, VAL_BOARD_1_POWER_PATH(5, 2, 0x14, 1000, 1000), 0x00},
	{"nvcc_snvs_1v8",	power, VAL_BOARD_1_POWER_PATH(5, 3, 0x14, 10000, 509000), 0x00},
	{"vdd_snvs_0v8",	power, VAL_BOARD_1_POWER_PATH(5, 4, 0x14, 10000, 509000), 0x00},
	{"vdd_gpu",		power, VAL_BOARD_1_POWER_SWITCH_PATH(6, 1, 0x15, 50, 2, 10050), 0x00},
	{"vdd_mipi_1v8",	power, VAL_BOARD_1_POWER_PATH(6, 3, 0x15, 2000, 2000), 0x00},
	{"vdd_pll_ana_1v8",	power, VAL_BOARD_1_POWER_PATH(6, 4, 0x15, 200, 200), 0x00},
	{"nvcc_sd1",		power, VAL_BOARD_1_POWER_PATH(7, 1, 0x16, 100, 100), 0x00},
	{"lpd4_vdd1",		power, VAL_BOARD_1_POWER_PATH(7, 2, 0x16, 4990, 4990), 0x00},
	{"cpu_vdd_1v8",		power, VAL_BOARD_1_POWER_PATH(7, 3, 0x16, 100, 100), 0x00},
	{"bb_vdd_1v8",		power, VAL_BOARD_1_POWER_PATH(7, 4, 0x16, 50, 50), 0x00},
	{"vdd_pll_ana_0v8",	power, VAL_BOARD_1_POWER_PATH(8, 1, 0x17, 1000, 1000), 0x00},
	{"vdd_usb_1v8",		power, VAL_BOARD_1_POWER_PATH(8, 2, 0x17, 2000, 2000), 0x00},
	{"cpu_vdd_3v3",		power, VAL_BOARD_1_POWER_PATH(8, 3, 0x17, 100, 100), 0x00},
	{"vccq_sd1",		power, VAL_BOARD_1_POWER_PATH(8, 4, 0x17, 100, 100), 0x00},
	{"vdd_usb_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 1, 0x18, 1000, 1000), 0x00},
	{"vdd_usb_0v8",		power, VAL_BOARD_1_POWER_PATH(9, 2, 0x18, 400, 400), 0x00},
	{"vdd_sd1_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 3, 0x18, 250, 250), 0x00},
	{"bb_vdd_3v3",		power, VAL_BOARD_1_POWER_PATH(9, 4, 0x18, 50, 50), 0x00},
	{"vdd_dram",		power, VAL_BOARD_1_POWER_SWITCH_PATH(10, 1, 0x1A, 50, 2, 10050), 0x00},
	{"vdd_dram_pll_0v8",	power, VAL_BOARD_1_POWER_PATH(10, 3, 0x1A, 250, 250), 0x00},
	{"nvcc_enet",		power, VAL_BOARD_1_POWER_PATH(10, 4, 0x1A, 100, 100), 0x00},

	{"boot_mode",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x0F;opendrain=0;}", 0x30},
	{"SR_vsys_5v",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"SR_nvcc_snvs_1v8",	gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"SR_vdd_soc",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"SR_vdd_snvs_0v8",	gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=0;pin_bitmask=0x80;opendrain=0;}", 0x00},
	{"remote_en",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x01;opendrain=0;}", 0x41},
	{"SR_vdd_gpu",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x02;opendrain=0;}", 0x00},
	{"SR_vdd_dram",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x04;opendrain=0;}", 0x00},
	{"SR_vdd_arm",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x08;opendrain=0;}", 0x00},
	{"SR_nvcc_dram_1v1",	gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x10;opendrain=0;}", 0x00},
	{"ft_io9",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x20;opendrain=0;}", 0x00},
	{"ft_io10",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x40;opendrain=0;}", 0x00},
	{"ft_io11",		gpio, VAL_BOARD_1_GPIO_EXTENDER_PATH"/pcal6524h{addr=0x22;port=1;pin_bitmask=0x80;opendrain=0;}", 0x00},

	{"reset_b",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x10}", 0x00},
	{"reset",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x20}", 0x21},
	{"io_nrst",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x40}", 0x11},
	{"onoff",		gpio, "/ft4232h_gpio{channel=0;pin_bitmask=0x80}", 0x00},

	{"ft_gpio1",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x10}", 0x00},
	{"ft_gpio2",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x20}", 0x00},
	{"ft_gpio3",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x40}", 0x00},
	{"io_nint",		gpio, "/ft4232h_gpio{channel=1;pin_bitmask=0x80}", 0x00},

	{"93lcx6",		ftdi_eeprom, "/ft4232h_eeprom{uasize=0xFF}", 0x00},

	{NULL, 0, NULL, 0} //null terminated
};

struct board_power_group val_board_2_power_groups[] = {
	{"GROUP_SOC",		"nvcc_snvs_1v8,vdd_snvs_0v8,vdd_soc,vdd_pll_ana_0v8,vdd_usb_0v8,vdd_gpu,vdd_dram,vdd_dram_pll_0v8,vdd_phy_0v9,vdd_arm,vdd_pll_ana_1v8,vdd_usb_1v8,vdd_mipi_1v8,nvcc_dram_1v1,vdd_usb_3v3,vdd_phy_1v2"},
	{"GROUP_SOC_FULL",	"nvcc_snvs_1v8,vdd_snvs_0v8,vdd_soc,vdd_pll_ana_0v8,vdd_usb_0v8,vdd_gpu,vdd_dram,vdd_dram_pll_0v8,vdd_phy_0v9,vdd_arm,vdd_pll_ana_1v8,vdd_usb_1v8,vdd_mipi_1v8,nvcc_dram_1v1,vdd_usb_3v3,vdd_phy_1v2,cpu_vdd_1v8,nvcc_sd1,nvcc_sd2,nvcc_enet,cpu_vdd_3v3"},
	{"GROUP_DRAM",		"lpd4_vdd1,lpd4_vdd2,lpd4_vddq"},
	{"GROUP_PLATFORM",	"vbus_in_dcdc"},
	{NULL, 0}
};

struct boot_mode val_board_2_boot_modes[] = {
	{"fuse",	0x0},
	{"usb",		0x1},
	{"emmc",	0x2},
	{"sd",		0x3},
	{"nand_256",	0x4},
	{"nand_512",	0x5},
	{"qspi_3b",	0x6},
	{"qspi_hyper",	0x7},
	{"ecspi",	0x8},
	{"loop",	0xe},
	{"test",	0xf},
	{NULL,		0}
};

struct boot_mode null_boot_mode[] = {
	{NULL, 0}
};

struct board_links null_board_links[] = {
	"",
	""
};

struct board_info board_list[] =
{
	{"imx8dxlevk",		imx8xxl,		imx8xxl_boot_modes,		0,	NULL,				imx8xxl_power_groups,		imx8xxlevk_board_links,		&imx8dxlevk_ftdi_eeprom_user_area_info,		500},
	{"imx8dxlevkc1",	imx8xxl_board_c1,	imx8xxl_boot_modes,		0,	NULL,				imx8xxl_power_groups,		imx8xxlevk_board_links,		&imx8dxlevk_c1_ftdi_eeprom_user_area_info,	500},
	{"imx8dxl_ddr3_evk",	imx8dxl_ddr3,		imx8xxl_boot_modes,		0,	NULL,				NULL,				imx8xxlevk_board_links,		NULL,						500},
	{"imx8mpevkpwra0",	imx8mpevkpwr_board_a0,	imx8mpevk_board_boot_modes,	0,	NULL,				imx8mpevkpwr_power_groups,	imx8mpevk_board_links,		&imx8mpevkpwr_a0_ftdi_eeprom_user_area_info,	500},
	{"imx8mpevkpwra1",	imx8mpevkpwr_board_a1,	imx8mpevk_board_boot_modes,	0,	NULL,				imx8mpevkpwr_power_groups,	imx8mpevk_board_links,		&imx8mpevkpwr_a1_ftdi_eeprom_user_area_info,	500},
	{"imx8mpevk",		imx8mpevk_board,	imx8mpevk_board_boot_modes,	0,	NULL,				NULL,				imx8mpevk_board_links,		NULL,						500},
	{"imx8mpddr4",		imx8mpddr4_board,	null_boot_mode,			0,	NULL,				NULL,				null_board_links,		NULL,						500},
	{"imx8ulpevk",		imx8ulpevk_board,	imx8ulpevk_board_boot_modes,	1,	imx8ulpevk_board_boot_config,	imx8ulpevkpwr_power_groups,	null_board_links,		&imx8ulpevk_ftdi_eeprom_user_area_info,		9000},
	{"val_board_1",		val_board_1,		val_board_1_boot_modes,		2,	val_board_1_boot_config,	val_board_1_power_groups,	null_board_links,		&val_board_1_ftdi_eeprom_user_area_info,	500},
	{"val_board_2",		val_board_2,		val_board_2_boot_modes,		0,	NULL,				val_board_2_power_groups,	null_board_links,		&val_board_2_ftdi_eeprom_user_area_info,	500},
	//"imx9xxl",&imx9xxl_pins,
};
int num_of_boards = sizeof(board_list) / sizeof(struct board_info);

int have_gpio(char* gpio_name, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (strcmp(gpio_name, board->mappings[i].name) == 0)
		{
			return 0;
		}
		i++;
	}
	return -1;
}

void guess_board_name(char* board_name)
{
	char board_tmp[30] = "";
	int findflag = 0;

	strcpy(board_tmp, board_name);
	for (int j = 0; j < strlen(board_name); j++)
	{
		board_tmp[strlen(board_tmp) - j] = 0;
		for (int i = 0; i < num_of_boards; i++)
		{
			if (strstr(board_list[i].name, board_tmp) != NULL)
			{
				if (!findflag)
				{
					findflag = 1;
					mprintf(1, "do you mean: ");
				}
				mprintf(1, "%s ", board_list[i].name);
			}
		}
		if (findflag)
			break;
	}
	mprintf(1, "\n");
}

struct board_info *get_board(int is_auto, char* board_name)
{
	if (strlen(board_name) == 0)
	{
		if (is_auto == 0)
		{
			mprintf(3, "\nmissing option <--board=>\n");
			mprintf(3, "\nOr use option <--auto> to find the board automatically\n");
			mprintf(3, "NOTE: if other boards are also connected to the same host, <--auto> may break its ttyUSB function temporarily.\n\n");
			return NULL;
		}

		switch (bcu_find_board_by_eeprom(board_name))
		{
			case 0:
				printf("Auto recognized the board: %s\n", board_name);
				break;
			case -1:
			{
				printf("Can't auto recognize the board...Please try to add [-board=] option.\n");
				return NULL;
				// printf("For now, only 8MPLUSLPD4-CPU don't have eeprom. Assuming use \"imx8mpevk\"...\n");
				// printf("Please also notice if there is any other board connected to this host.\n");
				// printf("Try \"bcu lsftdi\" to find the right -id=...\n");
				// strcpy(setting.board, "imx8mpevk");
			}break;
			case -2:
			{
				printf("Can't open FTDI channel...Please try to add [-board=] option.\n");
				return NULL;
			}break;

			default:
				break;
		}
	}

	for (int i = 0; i < num_of_boards; i++)
	{
		if (strcmp(board_name, board_list[i].name) == 0)//board found
		{
			return &board_list[i];
		}
	}
	mprintf(3, "board model %s is not supported\n", board_name);

	guess_board_name(board_name);

	return NULL;
}


struct board_info *get_board_by_id(int id)
{
	if (id >= num_of_boards)
		return NULL;

	return &board_list[id];
}

int get_board_numer(void)
{
	return num_of_boards;
}

int get_item_location(char* item_name, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (strcmp(item_name, board->mappings[i].name) == 0)
		{
			return i;
		}
		i++;
	}
	//mprintf(3, "path not found\n");
	return -1;
}

int get_path(char* path, char* item_name, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (strcmp(item_name, board->mappings[i].name) == 0)
		{
			strcpy(path, board->mappings[i].path);
			return i;
		}
		i++;
	}
	//mprintf(3, "path not found\n");
	return -1;
}

int set_path(char* path, char* item_name, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (strcmp(item_name, board->mappings[i].name) == 0)
		{
			board->mappings[i].path = malloc(sizeof(char) * MAX_PATH_LENGTH);
			strcpy(board->mappings[i].path, path);
			return 0;
		}
		i++;
	}
	//mprintf(3, "gpio/power not found\n");
	return -1;
}

int get_gpio_info_by_initid(char* gpio_name, char* path, int initid, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (board->mappings[i].type == gpio && (board->mappings[i].initinfo >> 4) == initid)
		{
			strcpy(gpio_name, board->mappings[i].name);
			strcpy(path, board->mappings[i].path);
			return (board->mappings[i].initinfo) & 0x0F;
		}
		i++;
	}

	return -1;
}

int get_power_index_by_showid(int showid, struct board_info *board)
{
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (board->mappings[i].type == power && board->mappings[i].initinfo == showid)
		{
			return i;
		}
		i++;
	}

	return -1;
}

/*
 * get the maximum length of the power related variable name
 */
int get_max_power_name_length(struct board_info *board)
{
	size_t max = 0;
	int i = 0;
	while (board->mappings[i].name != NULL)
	{
		if (board->mappings[i].type == power && board->mappings[i].initinfo && strlen(board->mappings[i].name) > max)
		{
			max = (int)strlen(board->mappings[i].name);
		}
		i++;
	}
	return max;
}

int get_boot_mode_offset(unsigned char boot_mode_pin_bitmask)
{
	int offset = 0;
	unsigned char hex = boot_mode_pin_bitmask;
	while (hex % 2 == 0)
	{
		offset++;
		hex = hex >> 1;
	}
	if (offset > 8)
	{
		mprintf(2, "get_boot_mode_offset: something is wrong with the pin bitmask %x\n", boot_mode_pin_bitmask);
		return -1;
	}
	//mprintf(3, "offset is %d\n", offset);
	return offset;
}

char* get_boot_mode_name_from_hex(struct board_info *board, int boot_mode_hex)
{
	int i = 0;

	while (board->boot_modes[i].name != NULL)
	{
		if (board->boot_modes[i].boot_mode_hex == boot_mode_hex)
		{
			return board->boot_modes[i].name;
		}
		i++;
	}
	return NULL;
}

char* get_boot_config_name_from_hex(struct board_info *board, int *boot_config_hex, int boot_mode_hex)
{
	int i = 0;

	while (board->boot_modes[i].name != NULL)
	{
		if (board->boot_modes[i].boot_mode_hex == boot_mode_hex)
		{
			int j;
			for (j = 0; j < board->boot_cfg_byte_num; j++)
			{
				if (board->boot_configs[i].boot_config_hex[j] != boot_config_hex[j])
					break;
			}
			if (j >= board->boot_cfg_byte_num)
				return board->boot_configs[i].name;
		}
		i++;
	}
	return NULL;
}
