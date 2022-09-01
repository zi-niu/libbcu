#ifndef LEGACY_BCU_H
#define LEGACY_BCU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "libbcu.h"

#define NO_BOARD_AND_ID		1
#define NO_BOARD			2
#define NO_ID				3
#define NO_USE_AUTO_FIND	4

int legacy_parse_board_id_options(int argc, char** argv, struct options_setting* setting);
int legacy_parse_options(char* cmd, int argc, char** argv, struct options_setting* setting);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef LEGACY_BCU_H */