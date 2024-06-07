/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
//#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include <app/drivers/blink.h>

#include <app_version.h>

LOG_MODULE_REGISTER(main, CONFIG_APP_LOG_LEVEL);

#define BLINK_PERIOD_MS_STEP 100U
#define BLINK_PERIOD_MS_MAX  1300U

#define TICK_PERIOD   (1000)

#define LED0_NODE	DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

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
    gpio_pin_toggle_dt(&led);
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

	gpio_pin_toggle_dt(&led);

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

	//printk("Use the sensor to change LED blinking period\n");
	blink_set_period_ms(blink, period_ms);
	k_timer_start(&control_timer, K_MSEC(TICK_PERIOD), K_MSEC(TICK_PERIOD));

	return 0;
}

