/*
MIT License

Copyright (c) 2024 kristosb

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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

/*********************
 *      DEFINES
 *********************/
#define SENS_STACKSIZE 1024
#define DISP_STACKSIZE 8192
#define PRIORITY 7
#define SENSING_SLEEP_MS 100
#define DISPLAY_SLEEP_MS 101
K_MUTEX_DEFINE(gyro_data_mutex);
/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
  LIGHT,
  DARK,
} screen_style_t;

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

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void read_gyro_data(const struct device * gyro_dev);
void display_gyro_data(void);
void hud_set_type(screen_style_t style);
void hud_set_line_width(lv_coord_t width);
int sensing(void);
int display(void);
/*=====================
 *  Functions
 *====================*/
void read_gyro_data(const struct device * gyro_dev)
{
	k_mutex_lock(&gyro_data_mutex, K_FOREVER);
	sensor_sample_fetch(gyro_dev);
	sensor_channel_get(gyro_dev, SENSOR_CHAN_GYRO_XYZ, gyr);
	k_mutex_unlock(&gyro_data_mutex);
}
void display_gyro_data(void)
{
	k_mutex_lock(&gyro_data_mutex, K_FOREVER);
	lv_pitch_ladder_set_angles(pitch_ladder_obj, gyr[1].val1 , gyr[2].val1*10);
	lv_compass_angle(compass_obj, gyr[0].val1);
	k_mutex_unlock(&gyro_data_mutex);
}
void hud_set_type(screen_style_t style){
	if(style == DARK){
		lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
		lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
		lv_pitch_ladder_set_dark_style(pitch_ladder_obj);
		lv_compass_set_dark_style(compass_obj);
	}else{
		lv_pitch_ladder_set_light_style(pitch_ladder_obj);
		lv_compass_set_light_style(compass_obj);
	}
}
void hud_set_line_width(lv_coord_t width)
{
	lv_compass_set_line_width(compass_obj, 2);
	lv_pitch_ladder_set_line_width(pitch_ladder_obj, 2);
}
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

	hud_set_type(DARK);
	hud_set_line_width(2);

	//display_set_brightness(display_dev, 255);
	//display_set_orientation(display_dev, DISPLAY_ORIENTATION_ROTATED_90);
	display_blanking_off(display_dev);

	while (1) {
		display_gyro_data();
		lv_task_handler();
		k_msleep(DISPLAY_SLEEP_MS);
	}
}

K_THREAD_DEFINE(sensing_id, SENS_STACKSIZE, sensing, NULL, NULL, NULL, PRIORITY, 0, 500);
K_THREAD_DEFINE(display_id, DISP_STACKSIZE, display, NULL, NULL, NULL, PRIORITY, 0, 500);
