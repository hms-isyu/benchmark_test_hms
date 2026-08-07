#
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2026 HMS Industrial Networks GmbH & Co. KG
# Author: Isaac L. L. Yuki 
#

cmake_minimum_required(VERSION 3.22)

if(NOT CMAKE_C_STANDARD OR CMAKE_C_STANDARD LESS 11)
    message(FATAL_ERROR "BMTH Generated code requires C11 or higher")
endif()

set(CURRENT_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
set(BMTH_PORT_PATH ${CURRENT_MODULE_PATH}/port)

project(bmth)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

add_library(bmth INTERFACE)

target_include_directories(bmth INTERFACE
    ${CURRENT_MODULE_PATH}
)

target_sources(bmth INTERFACE
    ${CURRENT_MODULE_PATH}/benchmark_tools_hms.c
)

target_link_directories(bmth INTERFACE
)

if(CONFIG_BMTH_EXTERN_PORT_LAYER)
    target_include_directories(bmth INTERFACE
        ${BMTH_PORT_PATH}/extern
    )
    target_sources(bmth INTERFACE
        ${BMTH_PORT_PATH}/extern/bmth_port.c
    )
endif()

if(CONFIG_BMTH_FRDM_MCXN947)
    target_sources(bmth INTERFACE
        ${CURRENT_MODULE_PATH}/cores/bmth_m33.c
    )
    if(CONFIG_BMTH_ZEPHYR)
        target_include_directories(bmth INTERFACE
            ${BMTH_PORT_PATH}/frdm_mcxn947/zephyr
        )
        target_sources(bmth INTERFACE
            ${BMTH_PORT_PATH}/frdm_mcxn947/zephyr/bmth_port.c
        )
    elseif(CONFIG_BMTH_ETX)
        target_include_directories(bmth INTERFACE
            ${BMTH_PORT_PATH}/frdm_mcxn947/etx
        )
        target_sources(bmth INTERFACE
            ${BMTH_PORT_PATH}/frdm_mcxn947/etx/bmth_port.c
        )
    else()
        message(FATAL_ERROR "BMTH_FRDM_MCXN947 requires some port to be enabled")
    endif()
endif()
