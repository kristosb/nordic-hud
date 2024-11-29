/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>

#include <stdio.h>
#include <string.h>
#include <lvgl.h>

//#include <app/drivers/blink.h>

#include <app/lib/lv_compass.h>
#include <app/lib/lv_pitch_ladder.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

/* size of stack area used by each thread */
#define STACKSIZE 1024
#define PRIORITY 7
#define SENSING_SLEEP_MS 100
#define DISPLAY_SLEEP_MS 101

K_MUTEX_DEFINE(gyro_data_mutex);

typedef struct {
    lv_obj_t     ** object; 
    short         * value;
    short           step;
    short           max;
    short           min;
} param_t;

typedef struct {
    lv_obj_t * screen;
    int        count;
    param_t  * params;
} screens_t;

static lv_obj_t   * compass_obj;
static lv_obj_t   * pitch_ladder_obj;
static short        compass_value;
static short        pitch_ladder_pitch;
static param_t screen0_elements [] = {
	{ .object = &compass_obj,  .value = &compass_value, .step = 1, .max = 360, .min = 0},
	{ .object = &pitch_ladder_obj,  .value = &pitch_ladder_pitch, .step = 1, .max = 360, .min = 0}
};
static screens_t screens [] = {
    { .screen = NULL, .count = 1, .params = screen0_elements }
};

struct sensor_value gyr[3];

int sensing(void)
{
	const struct device *gyro_dev = DEVICE_DT_GET(DT_NODELABEL(bno055_l));
	//printk("sensing begin\n");
	if (!device_is_ready(gyro_dev)) {
		//printk("Device %s is not ready\n", gyro_dev->name);
		return 0;
	}
	
	while (1) {
		k_msleep(SENSING_SLEEP_MS);
		read_gyro_data(gyro_dev);
	}
}

void read_gyro_data(struct device * gyro_dev)
{
	k_mutex_lock(&gyro_data_mutex, K_FOREVER);
	sensor_sample_fetch(gyro_dev);
	sensor_channel_get(gyro_dev, SENSOR_CHAN_GYRO_XYZ, gyr);
	k_mutex_unlock(&gyro_data_mutex);
}

void display_gyro_data()
{
	k_mutex_lock(&gyro_data_mutex, K_FOREVER);
	lv_pitch_ladder_set_angles(pitch_ladder_obj, gyr[1].val1 , gyr[2].val1*10);
	lv_comapss_angle(compass_obj, gyr[0].val1);
	k_mutex_unlock(&gyro_data_mutex);
}

int display(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
	//printk("Zephyr tinyHUD Application %s\n", APP_VERSION_STRING);
	//printk("display begin\n");

    if (display_dev == NULL) {
        LOG_ERR("Display device not found.");
        return 0;
    }
	
    screens[0].screen = lv_obj_create(NULL);
	//printk("Screen size: %d, %d", lv_obj_get_height(screens[0].screen), lv_obj_get_width(screens[0].screen));
	lv_scr_load(screens[0].screen);
	pitch_ladder_obj = lv_pitch_ladder_create(lv_scr_act());
	compass_obj = lv_compass_create(lv_scr_act());

	display_blanking_off(display_dev);

	while (1) {
		display_gyro_data();
		lv_task_handler();
		k_msleep(DISPLAY_SLEEP_MS);
	}
}

K_THREAD_DEFINE(sensing_id, STACKSIZE, sensing, NULL, NULL, NULL, PRIORITY, 0, 500);
K_THREAD_DEFINE(display_id, 8192, display, NULL, NULL, NULL, PRIORITY, 0, 500);

