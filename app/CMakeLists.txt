#-------------------------------------------------------------------------------
# Zephyr Example Application
#
# Copyright (c) 2021 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

#set(SHIELD st7735r_ada_160x128)

project(app LANGUAGES C)

target_sources(app PRIVATE src/main.c)
#arget_sources(app PRIVATE src/lv_scale.c)

#set(LVGL_DIR ${ZEPHYR_LVGL_MODULE_DIR})