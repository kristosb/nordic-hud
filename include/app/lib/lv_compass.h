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
#ifndef APP_LIB_LV_COMPASS_H_
#define APP_LIB_LV_COMPASS_H_

// #ifdef __cplusplus
// extern "C" {
// #endif

#define LV_USE_COMAPSS 1
/*********************
 *      INCLUDES
 *********************/
#include <lv_conf_internal.h>
#include <core/lv_obj.h>
#include <widgets/lv_line.h>

/*********************
 *      DEFINES
 *********************/
#define COMPAS_WIDTH 128
#define COMPAS_HEIGHT 34
#define COMPAS_FONT_HEIGHT 14
#define COMPAS_MAJOR_TICK_LENGHT 6
#define COMPAS_MINOR_TICK_LENGHT COMPAS_MAJOR_TICK_LENGHT / 2
#define LV_COMPASS_SPACE (32)
#define COMPAS_TICK_SPACING LV_COMPASS_SPACE
#define LV_COMPASS_SCALE (10)
#define LV_COMPASS_TICK_RANGE (4)

/**********************
 *      TYPEDEFS
 **********************/
enum _lv_result_t {
    LV_RESULT_INVALID = 0, /*Typically indicates that the object is deleted (become invalid) in the action
                      function or an operation was failed*/
    LV_RESULT_OK,      /*The object is valid (no deleted) after the action*/
};

typedef struct {
    int16_t label_value;
    int16_t x_offset;
} lv_compass_tick_info_t;

typedef struct {
    lv_style_t * main_style;
    int32_t minor_range;
} lv_comapss_section_t;

typedef struct {
    lv_obj_t obj;
    lv_ll_t section_ll;     /**< Linked list for the sections (stores lv_comapss_section_t)*/
    const char ** txt_src;
    int32_t heading_angle;
    uint32_t post_draw          : 1;
    uint32_t draw_ticks_on_top  : 1;
    uint32_t widget_draw  : 1;
    /* Private properties */

} lv_comapss_t;
/*=====================
 * Setter functions
 *====================*/

/**
 * Set comapss angle. 
 * @param obj       pointer the comapss object
 * @param angle      value of the angle
 */
void lv_comapss_angle(lv_obj_t * obj, int32_t angle);
/**********************
 *      MACROS
 **********************/

#endif /*APP_LIB_LV_COMPASS_H_*/

