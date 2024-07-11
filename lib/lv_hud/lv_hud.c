/*
 * Copyright (c) 2024 kristosb
 *
 * MIT License
 */

#include <app/lib/lv_hud.h>

int custom_get_value(int val)
{
	return (val != 0) ? val : CONFIG_LV_HUD_GET_VALUE_DEFAULT;
}
