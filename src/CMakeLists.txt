cmake_minimum_required(VERSION 3.4)
# # build with static lib
# ADD_EXECUTABLE(main main.c)
# INCLUDE_DIRECTORIES(../lib/libbcu/include)
# TARGET_LINK_LIBRARIES(main bcu_static)

# build with shared lib
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBFTDI REQUIRED libftdi1)
pkg_check_modules(LIBYAML REQUIRED yaml-0.1)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
include_directories(${LIBUSB_INCLUDE_DIRS} ${LIBFTDI_INCLUDE_DIRS}include ${PROJECT_SOURCE_DIR}/libftdi/src ${LIBYAML_INCLUDEDIR})
INCLUDE_DIRECTORIES(include)
link_directories(${LIBUSB_LIBRARY_DIRS} ${LIBFTDI_LIBRARY_DIRS} ${LIBYAML_LIBRARY_DIRS})
ADD_EXECUTABLE(bcu main.c)
INCLUDE_DIRECTORIES(../lib/libbcu/include)
# LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/build/lib/libbcu ${PROJECT_SOURCE_DIR}/lib/libbcu)
TARGET_LINK_LIBRARIES(bcu bcu_static ${LIBUSB_LIBRARIES} ${LIBFTDI_LIBRARIES} ${LIBYAML_LIBRARIES})
# TARGET_LINK_LIBRARIES(main bcu ${LIBUSB_LIBRARIES} ${LIBFTDI_LIBRARIES} ${LIBYAML_LIBRARIES})