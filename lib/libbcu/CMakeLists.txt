cmake_minimum_required(VERSION 3.4)

PROJECT(LIBBCU)

execute_process( COMMAND sh ${PROJECT_SOURCE_DIR}/create_version_h.sh ${PROJECT_SOURCE_DIR}/include )

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFTDI REQUIRED libftdi1)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
include_directories(${LIBUSB_INCLUDE_DIRS} ${LIBFTDI_INCLUDE_DIRS}include ${LIBYAML_INCLUDEDIR})
INCLUDE_DIRECTORIES(include)
link_directories(${LIBUSB_LIBRARY_DIRS} ${LIBFTDI_LIBRARY_DIRS} ${LIBYAML_LIBRARY_DIRS})

SET(LIBBCU_SRC src/libbcu.c src/board.c src/chip.c src/eeprom.c src/parser.c src/port.c src/yml.c)
ADD_LIBRARY(bcu_static STATIC ${LIBBCU_SRC})
SET_TARGET_PROPERTIES(bcu_static PROPERTIES OUTPUT_NAME "bcu")