/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 * Note: 
 * Tested on nRF Connect SDK Version : 2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include <zephyr/logging/log.h>

#include "control.h"
//#include "oled.h"

//LOG_MODULE_REGISTER(main, 3);

#define STACKSIZE 1024
#define PRIORITY 7


#define SLEEP_TIME_MS   10*60*1000




void main(void)
{
	int ret;
	//printk("hello");
	control_init();
	while (1) {

        k_msleep(SLEEP_TIME_MS);
	}
}
