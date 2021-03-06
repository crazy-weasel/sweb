cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS)
set(CMAKE_CXX_FLAGS)

include_directories(
    ../../common/include
    ../../arch/${ARCH}/include
    ../../arch/${ARCH}/../common/include
    ../../arch/x86/include/
    ../../arch/x86/common/include/
    ../../arch/x86/32/common/include/
    ../../arch/x86/32/include/
    ../../common/include/cache/
    ../../common/include/fs
    ../../common/include/fs/tests/
    ./
    ./tasks
)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-D USE_FILE_SYSTEM_ON_GUEST_OS=1 -D NO_USE_OF_MULTITHREADING=1")  ## add global macro
endif()

file(GLOB sweb_img_util_SOURCES *.cpp "tasks/*.cpp" "../../common/source/cache/*.cpp" "../../common/source/fs/*.cpp" "../../common/source/fs/device/*.cpp" "../../common/source/fs/inodes/*.cpp" "../../common/source/fs/tests/*.cpp" "../../common/source/fs/unixfs/*.cpp" "../../common/source/fs/minix/*.cpp")

add_executable(sweb-img-util ${sweb_img_util_SOURCES})
