cmake_minimum_required(VERSION 3.10)

project(lpchecksum)

set(LPCHECKSUM_SOURCES
    "src/checksum_ivt.c"
    "src/lpcc_log.c"
    "src/main.c"
)

set(LPCHECKSUM_INCLUDES
    "include"
)

set(LPCHECKSUM_LIBRARIES
    "elf"
)

include_directories(${LPCHECKSUM_INCLUDES})
link_libraries(${LPCHECKSUM_LIBRARIES})

add_executable(${PROJECT_NAME} ${LPCHECKSUM_SOURCES})