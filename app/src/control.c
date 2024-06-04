#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <string.h>
#include <lvgl.h>

#include "control.h"

#include <zephyr/logging/log.h>

//#include <app/drivers/sensor/bmi05x.h>

#define TICK_PERIOD   (1000)

#define LED0_NODE	DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

const struct device *const dev = DEVICE_DT_GET(DT_NODELABEL(bmi088_g));

struct sensor_value acc[3], gyr[3];
// #define SW0_NODE	DT_ALIAS(sw0) 
// static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);

//static struct gpio_callback button_cb_data;

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
    //static short num_val = 0;  

    gpio_pin_toggle_dt(&led);

    screen0_label_value += 5;
    if(screen0_label_value > 120) screen0_label_value = 0;
	screen0_x_value +=1;
	screen0_y_value -=1;

	//sensor_sample_fetch(dev);

	//sensor_channel_get(dev, SENSOR_CHAN_ACCEL_XYZ, acc);
	// sensor_channel_get(dev, SENSOR_CHAN_GYRO_XYZ, gyr);
	// screen0_x_value = (short) gyr[0].val1;
	// screen0_y_value = (short) gyr[0].val2;

    numeric_param_update(screen_id, 0, screen0_label_value, "x: %d");
	numeric_param_update(screen_id, 1, screen0_x_value, "y: %d");
	numeric_param_update(screen_id, 2, screen0_y_value, "z: %d");

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
	//char result[20];
    if (param == NULL || *param->object == NULL)
        return;

    /*if (value <= param->min)  *param->value = param->min;
    else if ( value >  param->max)  *param->value = param->max; 
         else *param->value = value;*/

    if (lv_obj_check_type(*param->object, &lv_label_class)) {
		lv_label_set_text_fmt(*param->object, format, *param->value); //"X%d: %d"
        //LOG_INF("%s: value %d", __func__, lv_slider_get_value(*param->object));
        return;
    }
}

int control_init(void)
{
	struct sensor_value full_scale, sampling_freq, oversampling;

	/*if (!device_is_ready(dev)) {
		//printf("Device %s is not ready\n", dev->name);
		return 0;
	}*/

	//const struct device *display_dev = device_get_binding(DT_NODE_FULL_NAME(DT_CHOSEN(zephyr_display)));
	
	//const struct device *display_dev = DEVICE_DT_GET(DT_NODELABEL(st7735r_st7735r_ada_160x128));
    int ret = 0;
    if (!device_is_ready(led.port)) {
        return 0;
	}

	/*if (!device_is_ready(button.port)) {
		return 0;
	}*/
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	/*ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE );
*/

    if (display_dev == NULL) {
        //LOG_ERR("Display device not found.");
        return -1;
    }

	/*full_scale.val1 = 2;           
	full_scale.val2 = 0;
	sampling_freq.val1 = 100;       
	sampling_freq.val2 = 0;
	oversampling.val1 = 1;          
	oversampling.val2 = 0;

	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_FULL_SCALE,
			&full_scale);
	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_OVERSAMPLING,
			&oversampling);

	sensor_attr_set(dev, SENSOR_CHAN_ACCEL_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY,
			&sampling_freq);

	full_scale.val1 = 500;        
	full_scale.val2 = 0;
	sampling_freq.val1 = 100;       
	sampling_freq.val2 = 0;
	oversampling.val1 = 1;          
	oversampling.val2 = 0;

	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_FULL_SCALE,
			&full_scale);
	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_OVERSAMPLING,
			&oversampling);
	sensor_attr_set(dev, SENSOR_CHAN_GYRO_XYZ,
			SENSOR_ATTR_SAMPLING_FREQUENCY,
			&sampling_freq);
*/

    display_screens_init();
    lv_scr_load(screens[0].screen);

	display_blanking_off(display_dev);

    //LOG_INF("Display device init");

    k_timer_start(&control_timer, K_MSEC(TICK_PERIOD), K_MSEC(TICK_PERIOD));

    return 0;
}