#include <zephyr/kernel.h> 
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#include "oled.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(oled, 3);

#define LED0_NODE	DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#define SW0_NODE	DT_ALIAS(sw0) 
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);



static struct gpio_callback button_cb_data;


static const struct device * display_dev;


void display_timer_handler(struct k_timer * timer);
void display_task_handler(struct k_work * work);

K_TIMER_DEFINE(display_timer, display_timer_handler, NULL);

K_WORK_DEFINE(display_work, display_task_handler);

#define TICK_PERIOD   (1000)






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


static lv_obj_t   * screen0_slider_obj;
static short        screen0_slider_value;
static param_t screen0_elements [] = {
    { .object = &screen0_slider_obj,  .value = &screen0_slider_value, .step = 5, .max = 100, .min = 0},
};
static screens_t screens [] = {
    { .screen = NULL, .count = 1, .params = screen0_elements }
};

void display_task_handler(struct k_work * work)
{  
    static int screen_id = 0;
    static int param_id  = 0;
    static short slider_val = 0;  
    gpio_pin_toggle_dt(&led);

    slider_val += 5;
    if(slider_val > 120) slider_val = 0;
    numeric_param_update(screen_id, param_id, slider_val);
    lv_task_handler();
}


void display_timer_handler(struct k_timer * timer)
{
    k_work_submit(&display_work);
}

void numeric_param_update(int screen_id, int param_id, short value)
{
    param_t * param = &screens[screen_id].params[param_id];

    if (param == NULL || *param->object == NULL)
        return;

    

    if (value <= param->min)  *param->value = param->min;
    else if ( value >  param->max)  *param->value = param->max; 
         else *param->value = value;

    if (lv_obj_check_type(*param->object, &lv_label_class)) {
        return;
    }

    if (lv_obj_check_type(*param->object, &lv_slider_class)) {
        lv_slider_set_value(*param->object, *param->value, LV_ANIM_OFF);
        LOG_INF("%s: value %d", __func__, lv_slider_get_value(*param->object));
        return;
    }
}

void display_param_update(int screen_id, int param_id, bool inc)
{
    param_t * param = &screens[screen_id].params[param_id];

    if (param == NULL || *param->object == NULL)
        return;

    if (inc) *param->value += param->step;
    else     *param->value -= param->step;

    if (*param->value <= param->min)  *param->value = param->min;
    if (*param->value >  param->max)  *param->value =0;// param->max;

    if (lv_obj_check_type(*param->object, &lv_label_class)) {
        static char value[4];
        snprintf(value, sizeof(value), "%u", *param->value);
        lv_label_set_text(*param->object, value);
        return;
    }

    if (lv_obj_check_type(*param->object, &lv_slider_class)) {
        lv_slider_set_value(*param->object, *param->value, LV_ANIM_OFF);
        LOG_INF("%s: value %d", __func__, lv_slider_get_value(*param->object));
        return;
    }
}
void display_screens_init(void)
{
    screens[0].screen = lv_obj_create(NULL);
    static lv_style_t style_main;
    static lv_style_t style_indicator;
     static lv_style_t style_knob;

    lv_style_init(&style_main);
    lv_style_set_text_color(&style_main, lv_color_black());
    lv_style_set_bg_color(&style_main, lv_color_white());

    lv_style_init(&style_indicator);
    lv_style_set_text_color(&style_indicator, lv_color_white());
    lv_style_set_bg_color(&style_indicator, lv_color_black());
    lv_style_set_border_width(&style_indicator, 2);
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);

    lv_style_init(&style_knob);
    lv_style_set_text_color(&style_knob, lv_color_black());
    lv_style_set_bg_color(&style_knob, lv_color_white());
    lv_style_set_border_width(&style_knob, 2);
    lv_style_set_radius(&style_knob, LV_RADIUS_CIRCLE);


    lv_scr_load(screens[0].screen);
    lv_obj_t * screen0_label = lv_label_create(lv_scr_act());
    lv_label_set_text(screen0_label, "Pg1");
    lv_obj_align_to(screen0_label, screens[0].screen, LV_ALIGN_TOP_RIGHT, 0, 0);

    screen0_slider_obj = lv_slider_create(lv_scr_act());
    lv_slider_set_mode(screen0_slider_obj, LV_SLIDER_MODE_NORMAL);
    lv_obj_add_style(screen0_slider_obj, &style_main, LV_PART_MAIN);
    lv_obj_add_style(screen0_slider_obj, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(screen0_slider_obj, &style_knob, LV_PART_KNOB);
    lv_obj_set_height(screen0_slider_obj, 8); 
    lv_obj_set_width(screen0_slider_obj, 110);    
    lv_slider_set_range(screen0_slider_obj, 0, 100);
    lv_slider_set_value(screen0_slider_obj, 60, LV_ANIM_OFF);
    screen0_slider_value = 55;

    lv_obj_align_to(screen0_slider_obj, NULL, LV_ALIGN_CENTER, 0, 0);
    

}


int display_init(void)
{


    int ret = 0;
    if (!device_is_ready(led.port)) {
        return 0;
	}

	if (!device_is_ready(button.port)) {
		return 0;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE );


    display_dev = device_get_binding(DT_NODE_FULL_NAME(DT_CHOSEN(zephyr_display)));
    if (display_dev == NULL) {
        LOG_ERR("Display device not found.");
        return -1;
    }
    LOG_INF("Display device init");
    display_screens_init();

    lv_scr_load(screens[0].screen);
    display_blanking_off(display_dev);
    k_timer_start(&display_timer, K_MSEC(TICK_PERIOD), K_MSEC(TICK_PERIOD));

    return 0;
}