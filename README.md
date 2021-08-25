# libbcu build step

# cd libbcu/
# mkdir build
# cd build/
# cmake ..
# make

@OptiPlex:~/work/libbcu/build$ cmake ..
-- The C compiler identification is GNU 9.3.0
-- The CXX compiler identification is GNU 9.3.0
-- Check for working C compiler: /usr/bin/cc
-- Check for working C compiler: /usr/bin/cc -- works
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Detecting C compile features
-- Detecting C compile features - done
-- Check for working CXX compiler: /usr/bin/c++
-- Check for working CXX compiler: /usr/bin/c++ -- works
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Detecting CXX compile features
-- Detecting CXX compile features - done
creating /home//work/libbcu/lib/libbcu/include/libbcu_version.h
build not in appveyor
-- Found PkgConfig: /usr/bin/pkg-config (found version "0.29.1") 
-- Checking for module 'libftdi1'
--   Found libftdi1, version 1.4
-- Checking for module 'yaml-0.1'
--   Found yaml-0.1, version 0.2.2
-- Checking for module 'libusb-1.0'
--   Found libusb-1.0, version 1.0.23
-- Configuring done
-- Generating done
-- Build files have been written to: /home//work/libbcu/build
@OptiPlex:~/work/libbcu/build$ make
Scanning dependencies of target bcu_static
[  9%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/libbcu.c.o
[ 18%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/board.c.o
[ 27%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/chip.c.o
[ 36%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/eeprom.c.o
[ 45%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/parser.c.o
[ 54%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/port.c.o
[ 63%] Building C object lib/libbcu/CMakeFiles/bcu_static.dir/src/yml.c.o
[ 72%] Linking C static library libbcu.a
[ 72%] Built target bcu_static
Scanning dependencies of target bcu
[ 81%] Building C object CMakeFiles/bcu.dir/src/main.c.o
[ 90%] Building C object CMakeFiles/bcu.dir/src/options.c.o
[100%] Linking C executable bcu
[100%] Built target bcu
