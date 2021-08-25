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

#include "parser.h"
#include "chip.h"
#include "board.h"

#define MAX_CHIP_SPEC_LENGTH 200

extern struct name_and_init_func chip_list[];
extern int num_of_chips;
extern struct board_info board_list[];
extern int num_of_boards;


int update_parameter_string(char* path, char* parameter_name, char* str)
{
	char tpath[MAX_PATH_LENGTH];
	char* ptr = strstr(path, parameter_name);
	if (ptr == NULL)
	{
		mprintf(2, "could not locate parameter %s\n", parameter_name);
		return -1;
	}
	char* equal_sign = strchr(ptr, '=');
	char* brk = strpbrk(equal_sign, ";}");
	if (equal_sign == NULL)
	{
		mprintf(2, "can not understand the specification \n");
		return -1;
	}
	strcpy(tpath, brk);
	equal_sign[1] = '\0';
	strcat(equal_sign, str);
	strcat(equal_sign, tpath);
	//int length = (int)(brk - equal_sign - 1);
	//strncpy(result, equal_sign + 1, length);
	//*(result + length) = '\0';
	return 0;

}

int extract_parameter_string(char* chip_specification, char* parameter_name, char* result)
{
	char* ptr = strstr(chip_specification, parameter_name);
	if (ptr == NULL)
	{
		if(strcmp(parameter_name, "sensor2") != 0)
			mprintf(2, "could not locate parameter %s\n", parameter_name);
		return -1;
	}
	char* equal_sign = strchr(ptr, '=');
	char* brk = strpbrk(equal_sign, ";}");
	if (equal_sign == NULL)
	{
		mprintf(2, "can not understand the specification \n");
		return -1;
	}
	int length = (int)(brk - equal_sign - 1);
	strncpy(result, equal_sign + 1, length);
	*(result + length) = '\0';
	return 0;

}

int extract_parameter_value(char* chip_specification, char* parameter_name)
{
	char str[MAX_CHIP_SPEC_LENGTH];
	int status = extract_parameter_string(chip_specification, parameter_name, str);
	if (status == -1)
	{
		return -1;
	}
	if (str[0] == '0' && str[1] == 'x')
		return strtol(str + 2, NULL, 16);
	else
		return strtol(str, NULL, 10);
}

void get_chip_name(char* chip_specification, char* chip_name)
{
	char* ptr = strchr(chip_specification, '{');
	if (ptr == NULL)
		strcpy(chip_name, chip_specification);
	else
	{
		int length = (int)(ptr - chip_specification);
		strncpy(chip_name, chip_specification, length);
		*(chip_name + length) = '\0';
	}
}

void free_device_linkedlist_backward(struct device* ptr)
{
	void* delete_this;

	while (ptr != NULL)
	{
		delete_this = ptr;
		void* parent = ptr->parent;
		if (parent == NULL) //if we have reached the end, the the device must be ftdi, so we must close and free the ftdidevice;
		{
			//mprintf(3, "closing and freeing ftdi device\n");
			struct device* ft = delete_this;
			ft->free(ft);
			return;
		}
		ptr = ptr->parent;

		free(delete_this);
	}
	return;
}

void free_device_linkedlist_forward(struct device* ptr)
{
	void* delete_this;

	while (ptr != NULL)
	{
		delete_this = ptr;
		void* parent = ptr->parent;
		if (parent == NULL) //if the device has no parent, then the device must be ftdi, so we must close and free the ftdidevice;
		{
			// mprintf(3, "closing and freeing ftdi device\n");
			struct device* ft = delete_this;
			ft->free(ft);
		}
		ptr = ptr->child;

		if (parent != NULL)
			free(delete_this);
	}
	return;
}

int get_chip_specification_by_chipname(char* path, char* chip_specification, char* chipname)
{
	char now_chip_name[MAX_CHIP_SPEC_LENGTH];
	char remaining_path[MAX_PATH_LENGTH];

	if (path[0] == '/')
		path++; //ignore the first '/'
	if (strlen(path) < MAX_PATH_LENGTH)
		strcpy(remaining_path, path);

	strcpy(chip_specification, strtok(remaining_path, "/"));

	while (chip_specification != NULL)
	{
		get_chip_name(chip_specification, now_chip_name);
		if (strcmp(chipname, now_chip_name) == 0) {
			return 0;
		}

		strcpy(chip_specification, strtok(NULL, "/"));
	}
	return -1;
}

void* build_device_linkedlist_forward(void** head, char* path)
{
	//mprintf(3, "original path: %s\n", path );

	char remaining_path[MAX_PATH_LENGTH];
	char chip_name[MAX_CHIP_SPEC_LENGTH];
	struct device* current = NULL;
	struct device* previous = NULL;

	if (path[0] == '/')
		path++; //ignore the first '/'
	if (strlen(path) < MAX_PATH_LENGTH)
		strcpy(remaining_path, path);
	else {
		mprintf(2, "entered path exceeded maximum length of the buffer\n");
		return NULL;
	}
	char* chip_specification = strtok(remaining_path, "/");
	//mprintf(3, "chip_specification: %s\n", chip_specification);
	int found = 0;
	int is_head = 1;
	while (chip_specification != NULL)
	{
		get_chip_name(chip_specification, chip_name);
		found = 0;
		for (int i = 0; i < num_of_chips; i++)
		{
			if (strcmp(chip_list[i].name, chip_name) == 0)
			{
				found = 1;
				current = chip_list[i].create_funcptr(chip_specification, previous);
				if (current == NULL)
				{
					mprintf(2, "failed to create %s data structure\n", chip_name);
					return NULL;
				}
				if (is_head)
				{
					if (head != NULL)
						*head = current;
					is_head = 0;
				}

				if (previous != NULL)
					previous->child = current;

				previous = current;
			}
		}
		if (!found)
		{
			mprintf(2, "did not recognize chip '%s'\n", chip_name);
			free_device_linkedlist_backward(current);
			return NULL;
		}
		chip_specification = strtok(NULL, "/");
		//mprintf(4, "chip_specification: %s\n", chip_specification);
	}
	if (current)
		current->child = NULL;
	//mprintf(4, "finished\n");
	return current;
}

/*
given the previous linkedlist pointed by old_head;
this function compares the new path and the old path
and modifies the linkedlist so that the different part between the two path are rebuild
WITHOUT reinitializing the identical part between the two path, which could save a lot of
initialization time.

if the given previous linkedlist is null,
completely rebuild the linkedlist from scratch
*/
void* build_device_linkedlist_smart(void** new_head, char* new_path, void* old_head, char* old_path)
{
	if (old_head == NULL || old_path == NULL)// first time building
	{
		//mprintf(4, "building device linkedlist completely from scratch\n");
		return build_device_linkedlist_forward(new_head, new_path);
	}

	//mprintf(4, "attempting to partially rebuild device linked list \n");
	char new_remaining_path[MAX_PATH_LENGTH];
	char old_remaining_path[MAX_PATH_LENGTH];
	char chip_name[MAX_CHIP_SPEC_LENGTH];

	if (new_path[0] == '/')
		new_path++; //ignore the first '/'

	if (old_path[0] == '/')
		old_path++; //ignore the first '/'

	strcpy(new_remaining_path, new_path);
	strcpy(old_remaining_path, old_path);

	//first obtained the specifications from old path:
	int num_of_chips_in_old_path = 0;
	char* old_chip_specifications[MAX_CHIP_SPEC_LENGTH];
	int num_of_chips_in_new_path = 0;
	char* new_chip_specifications[MAX_CHIP_SPEC_LENGTH];
	for (int i = 0; i < 20; i++)//initialize so that valgrind does not complain
	{
		old_chip_specifications[i] = NULL;
		new_chip_specifications[i] = NULL;
	}
	old_chip_specifications[0] = strtok(old_remaining_path, "/");
	while (old_chip_specifications[num_of_chips_in_old_path] != NULL)
	{
		num_of_chips_in_old_path++;
		old_chip_specifications[num_of_chips_in_old_path] = strtok(NULL, "/");
	}
	new_chip_specifications[0] = strtok(new_remaining_path, "/");
	while (new_chip_specifications[num_of_chips_in_new_path] != NULL)
	{
		num_of_chips_in_new_path++;
		new_chip_specifications[num_of_chips_in_new_path] = strtok(NULL, "/");
	}

	int index = 0;

	struct device* current = old_head;
	struct device* previous = NULL;
	while (new_chip_specifications[index] != NULL && old_chip_specifications[index] != NULL
		&& strcmp(new_chip_specifications[index], old_chip_specifications[index]) == 0)
	{
		previous = current;
		current = current->child;
		index++;
	}
	/*
	not able to find a simple solution across all situation, i decided to handle this problem by cases
	after the while loop above; most important thing to realize:
	1,previous points to the last identical chip specification between two path, right before they diverge
	2,current and index points to the first difference between the two path.
	*/
	if (index == 0)//case 0: they are different since the very beginning
	{
		//mprintf(4, "completely different\n");
		//free all and completely rebuild
		free_device_linkedlist_forward(old_head);
		return build_device_linkedlist_forward(new_head, new_path);
	}
	else if (new_chip_specifications[index] == NULL && old_chip_specifications[index] == NULL)//case 1: they are identical
	{
		//mprintf(4, "same\n");
		//no need to free anything
		*new_head = old_head;
		return previous;
	}
	else if (new_chip_specifications[index] != NULL && old_chip_specifications[index] != NULL)//case 2: they differ before any of them ends
	{
		//mprintf(4, "different before end!\n");		
		free_device_linkedlist_forward(current);//free everything since the first difference
		*new_head = old_head;
		current = NULL;
		previous->child = NULL;
		//need reallocation in next step
	}
	else if (new_chip_specifications[index] == NULL && old_chip_specifications[index] != NULL)//case 3: the new one is a subset of old one
	{
		//mprintf(4, "new shorter\n");		
		free_device_linkedlist_forward(current);//free unwanted extra parts
		*new_head = old_head;
		return previous;

	}
	else if (new_chip_specifications[index] != NULL && old_chip_specifications[index] == NULL)//case 4 the old one is a subset of new one
	{
		//mprintf(4, "old shorter\n");		
		//no need to free anything
		*new_head = old_head;
		//need reallocation in next step
	}

	//reallocation
	while (new_chip_specifications[index] != NULL)
	{
		get_chip_name(new_chip_specifications[index], chip_name);
		int found = 0;
		for (int i = 0; i < num_of_chips; i++)
		{
			if (strcmp(chip_list[i].name, chip_name) == 0)
			{
				found = 1;
				current = chip_list[i].create_funcptr(new_chip_specifications[index], previous);
				if (current == NULL)
				{
					mprintf(2, "failed to create %s data structure\n", chip_name);
					return NULL;
				}
				current->child = NULL;
				if (previous != NULL) {
					previous->child = current;
					previous = current;
				}
			}

		}
		if (!found)
		{
			mprintf(2, "did not recognize chip '%s'\n", chip_name);
			free_device_linkedlist_backward(current);
			return NULL;
		}
		index++;
	}
	return current;
}

static int parse_group(int id, struct group* group_ptr, struct board_info *board)
{
	strcpy(group_ptr->name, board->power_groups[id].group_name);
	mprintf(3, "group name: %s\n", group_ptr->name);

	group_ptr->num_of_members = 0;
	if (strlen(board->power_groups[id].group_string) < (MAX_MAPPING_NAME_LENGTH * MAX_NUMBER_OF_POWER))
		strcpy(group_ptr->member_list, board->power_groups[id].group_string);
	else
	{
		mprintf(2, "entered group string exceeded maximum buffer size\n");
		return -1;
	}
	char group_member_string[MAX_MAPPING_NAME_LENGTH * MAX_NUMBER_OF_POWER];
	strcpy(group_member_string, group_ptr->member_list);
	// mprintf(4, "group string: %s\n", group_member_string);
	char* group_member_name = strtok(group_member_string, ",");
	while (group_member_name != NULL)
	{
		mprintf(3, "member: %s\n", group_member_name);
		int power_index = 0;
		int mapping_index = 0;
		int found = 0;
		while (board->mappings[mapping_index].name != NULL)
		{
			if (board->mappings[mapping_index].type == power)
			{
				if (strcmp(group_member_name, board->mappings[mapping_index].name) == 0)
				{
					found = 1;
					group_ptr->member_index[group_ptr->num_of_members] = power_index;
					group_ptr->num_of_members++;
					break;
				}
				power_index++;
			}

			mapping_index++;
		}
		if (found == 0)
			return -1;
		group_member_name = strtok(NULL, ",");
	}
	return 0;
}

int parse_groups(struct group* groups, struct board_info *board)
{
	int num_of_groups = 0;

	if (board->power_groups == NULL)
		return num_of_groups;

	while (board->power_groups[num_of_groups].group_name != NULL)
	{
		num_of_groups++;
	}

	for (int i = 0; i < num_of_groups; i++) {
		if (parse_group(i, &groups[i], board) == -1) {
			mprintf(2, "failed to understand the power groups setting\n");
			return -1;
		}
	}

	return num_of_groups;
}

void groups_init(struct group* groups, int num)
{
	for (int i = 0; i < num; i++)
	{
		groups[i].min = 99999;
		groups[i].max = 0;
		groups[i].sum = 0;
		groups[i].avg = 0;
		groups[i].avg_data_size = 0;
	}
}

void __str_replace(char* cp, int n, char* str)
{
	int lenofstr;
	char* tmp;
	lenofstr = strlen(str);
	if (lenofstr < n)
	{
		tmp = cp + n;
		while (*tmp)
		{
			*(tmp - (n - lenofstr)) = *tmp;
			tmp++;
		}
		*(tmp - (n - lenofstr)) = *tmp;
	}
	else
	{
		if (lenofstr > n)
		{
			tmp = cp;
			while (*tmp) tmp++;
			while (tmp >= cp + n)
			{
				*(tmp + (lenofstr - n)) = *tmp;
				tmp--;
			}
		}
	}
	strncpy(cp, str, lenofstr);
}

int str_replace(char *str, char *source, char *dest)
{
	int count = 0;
	char *p;

	p = strstr(str, source);
	while(p)
	{
		count++;
		__str_replace(p, strlen(source), dest);
		p = p+strlen(dest);
		p = strstr(p, source);
	}

	return count;
}

int compare_version(const char *v1, const char *v2)
{
	const char *p_v1 = v1;
	const char *p_v2 = v2;

	while (*p_v1 && *p_v2) {
		char buf_v1[32] = {0};
		char buf_v2[32] = {0};

		char *i_v1 = strchr(p_v1, '.');
		char *i_v2 = strchr(p_v2, '.');

		if (!i_v1 || !i_v2) break;

		if (i_v1 != p_v1) {
			strncpy(buf_v1, p_v1, i_v1 - p_v1);
			p_v1 = i_v1;
		}
		else
			p_v1++;

		if (i_v2 != p_v2) {
			strncpy(buf_v2, p_v2, i_v2 - p_v2);
			p_v2 = i_v2;
		}
		else
			p_v2++;

		int order = atoi(buf_v1) - atoi(buf_v2);
		if (order != 0)
			return order < 0 ? -1 : 1;
	}

	double res = atof(p_v1) - atof(p_v2);

	if (res < 0) return -1;
	if (res > 0) return 1;
	return 0;
}
