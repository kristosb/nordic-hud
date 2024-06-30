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

#include <app/drivers/blink.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define SLEEP_TIME_MS   10*60*1000
#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX  1300U

#define TICK_PERIOD   (300)

#define LED0_NODE	DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
//const struct device *gyro_dev = DEVICE_DT_GET(DT_NODELABEL(bmi055_g));
const struct device *gyro_dev = DEVICE_DT_GET(DT_NODELABEL(bno055_l));
//const struct device *const gyro_dev = DEVICE_DT_GET_ONE(bosch_bno055);

struct sensor_value gyr[3];

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


static lv_obj_t   * screen0_label_obj;
static lv_obj_t   * screen0_x_obj;
static lv_obj_t   * screen0_y_obj;
static short        screen0_label_value;
static short        screen0_x_value;
static short        screen0_y_value;
static param_t screen0_elements [] = {
    { .object = &screen0_label_obj,  .value = &screen0_label_value, .step = 5, .max = 150, .min = 0},
	{ .object = &screen0_x_obj,  .value = &screen0_x_value, .step = 5, .max = 150, .min = 0},
	{ .object = &screen0_y_obj,  .value = &screen0_y_value, .step = 5, .max = 150, .min = 0},
};
static screens_t screens [] = {
    { .screen = NULL, .count = 1, .params = screen0_elements }
};

void display_timer_handler(struct k_timer * timer);
void control_task_handler(struct k_work * work);

K_TIMER_DEFINE(control_timer, display_timer_handler, NULL);
K_WORK_DEFINE(control_work, control_task_handler);

void display_timer_handler(struct k_timer * timer)
{
    k_work_submit(&control_work);
}

void control_task_handler(struct k_work * work)
{  
	static int screen_id = 0;
    static int param_id  = 0;

    gpio_pin_toggle_dt(&led);

	sensor_sample_fetch(gyro_dev);
	sensor_channel_get(gyro_dev, SENSOR_CHAN_GYRO_XYZ, gyr);

	// printk("GX: %d; GY: %d; GZ: %d;\n",
	// 		gyr[0].val1,
	// 		gyr[1].val1,
	// 		gyr[2].val1);


    // screen0_label_value += 5;
    // if(screen0_label_value > 120) screen0_label_value = 0;
	// screen0_x_value +=1;
	// screen0_y_value -=1;
	//printk("x = %d\n", screen0_x_value);
	screen0_label_value = gyr[0].val1;
	screen0_x_value = gyr[1].val1;
	screen0_y_value = gyr[2].val1;
    numeric_param_update(screen_id, 0, screen0_label_value, "h: %d");
	numeric_param_update(screen_id, 1, screen0_x_value, "p: %d");
	numeric_param_update(screen_id, 2, screen0_y_value, "r: %d");

	lv_task_handler();
}

void display_screens_init(void)
{
    screens[0].screen = lv_obj_create(NULL);
    static lv_style_t style_main;
    static lv_style_t style_indicator;

    lv_style_init(&style_main);
    lv_style_set_text_color(&style_main, lv_color_black());
    lv_style_set_bg_color(&style_main, lv_color_white());

    lv_style_init(&style_indicator);
    lv_style_set_text_color(&style_indicator, lv_color_white());
    lv_style_set_bg_color(&style_indicator, lv_color_black());
    lv_style_set_border_width(&style_indicator, 2);
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);

	lv_scr_load(screens[0].screen);
    
	lv_obj_t *hello_world_label = lv_label_create(lv_scr_act());
	lv_label_set_text(hello_world_label, "WIKTOR");
	lv_obj_align_to(hello_world_label, screens[0].screen, LV_ALIGN_TOP_RIGHT, 0, 0);

	screen0_label_obj = lv_label_create(lv_scr_act());
	lv_label_set_text(screen0_label_obj, "11");
	lv_obj_align_to(screen0_label_obj, NULL, LV_ALIGN_LEFT_MID, 0, -20);
	screen0_label_value = 55;

	screen0_x_obj = lv_label_create(lv_scr_act());
	lv_label_set_text(screen0_x_obj, "22");
	lv_obj_align_to(screen0_x_obj, NULL, LV_ALIGN_LEFT_MID, 0, 0);
	screen0_x_value = 22;

	screen0_y_obj = lv_label_create(lv_scr_act());
	lv_label_set_text(screen0_y_obj, "33");
	lv_obj_align_to(screen0_y_obj, NULL, LV_ALIGN_LEFT_MID, 0, 20);
	screen0_y_value = 33;

}

void numeric_param_update(int screen_id, int param_id, short value, const char *format)
{
    param_t * param = &screens[screen_id].params[param_id];
    if (param == NULL || *param->object == NULL)
        return;

    if (lv_obj_check_type(*param->object, &lv_label_class)) {
		lv_label_set_text_fmt(*param->object, format, *param->value); //"X%d: %d"
        return;
    }
}

int main(void)
{
	int ret;
	unsigned int period_ms = BLINK_PERIOD_MS_MAX;
	const struct device *blink;


	printk("Zephyr Example Application %s\n", APP_VERSION_STRING);

    if (!device_is_ready(led.port)) {
        return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	
	blink = DEVICE_DT_GET(DT_NODELABEL(blink_led));
	if (!device_is_ready(blink)) {
		LOG_ERR("Blink LED not ready");
		return 0;
	}

	ret = blink_off(blink);
	if (ret < 0) {
		LOG_ERR("Could not turn off LED (%d)", ret);
		return 0;
	}

    if (display_dev == NULL) {
        LOG_ERR("Display device not found.");
        return -1;
    }
	//LOG_ERR("Error test");
	//LOG_INF("inf test");
	if (!device_is_ready(gyro_dev)) {
		printk("Device %s is not ready\n", gyro_dev->name);
		return 0;
	}

	printk("Device initialization success\n");
    display_screens_init();
    lv_scr_load(screens[0].screen);

	display_blanking_off(display_dev);
	
	blink_set_period_ms(blink, period_ms);
	k_timer_start(&control_timer, K_MSEC(TICK_PERIOD), K_MSEC(TICK_PERIOD));

	

	sensor_sample_fetch(gyro_dev);
	sensor_channel_get(gyro_dev, SENSOR_CHAN_GYRO_XYZ, gyr);

	printk("GX: %d.%d; GY: %d.%d; GZ: %d.%d;\n",
			gyr[0].val1, gyr[0].val2,
			gyr[1].val1, gyr[1].val2,
			gyr[2].val1, gyr[2].val2);

	while (1) {

        k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}

