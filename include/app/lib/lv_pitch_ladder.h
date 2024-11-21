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

#ifndef APP_LIB_LV_PITCH_LADDER_H_
#define APP_LIB_LV_PITCH_LADDER_H_

// #ifdef __cplusplus
// extern "C" {
// #endif

#define LV_USE_PITCH_LADDER 1
/*********************
 *      INCLUDES
 *********************/
#include <lv_conf_internal.h>

#include <core/lv_obj.h>
#include <widgets/lv_line.h>

/*********************
 *      DEFINES
 *********************/

/**Default value of total minor ticks. */
#define LV_PITCH_LADDER_TOTAL_TICK_COUNT_DEFAULT (11U)
LV_EXPORT_CONST_INT(LV_PITCH_LADDER_TOTAL_TICK_COUNT_DEFAULT);

/**Default value of major tick every nth ticks. */
#define LV_PITCH_LADDER_MAJOR_TICK_EVERY_DEFAULT (5U)
LV_EXPORT_CONST_INT(LV_PITCH_LADDER_MAJOR_TICK_EVERY_DEFAULT);

/**Default value of pitch_ladder label enabled. */
#define LV_PITCH_LADDER_LABEL_ENABLED_DEFAULT (1U)
LV_EXPORT_CONST_INT(LV_PITCH_LADDER_LABEL_ENABLED_DEFAULT);

#define LV_PITCH_LADDE_CANVAS_WIDTH (80U)
#define LV_PITCH_LADDE_CANVAS_HEIGHT (53U)
#define LV_PITCH_LADDER_HORIZ_GAP    LV_PITCH_LADDE_CANVAS_WIDTH/4
#define LV_PITCH_LADDER_HORIZ_LEND (LV_PITCH_LADDE_CANVAS_WIDTH -LV_PITCH_LADDER_HORIZ_GAP)/2
#define LV_PITCH_LADDER_HORIZ_RSTART LV_PITCH_LADDER_HORIZ_LEND + LV_PITCH_LADDER_HORIZ_GAP
#define LV_PITCH_LADDER_AIM_W (8U)
#define LV_PITCH_LADDER_SPACE (16)
#define LV_PITCH_LADDER_THICK (4U)
#define LV_PITCH_LADDER_LADDER_WDIFF (5U)
#define LV_PITCH_LADDER_FONT_SIZE (14U)
#define LV_PITCH_LADDER_LABEL_XOFFSET (4U)
#define LV_PITCH_LADDER_ROLL_TICK LV_PITCH_LADDER_SPACE
#define LV_PITCH_LADDER_ROLL_TICK_RANGE (4U)
#define LV_PITCH_LADDER_LABEL_W (16U)
#define LV_PITCH_LADDER_PITCH_SCALE (10U)

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Pitch_ladder mode
 */
enum {
    LV_PITCH_LADDER_MODE_HORIZONTAL_TOP    = 0x00U,
    LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM = 0x01U,
    LV_PITCH_LADDER_MODE_VERTICAL_LEFT     = 0x02U,
    LV_PITCH_LADDER_MODE_VERTICAL_RIGHT    = 0x04U,
    LV_PITCH_LADDER_MODE_ROUND_INNER       = 0x08U,
    LV_PITCH_LADDER_MODE_ROUND_OUTER      = 0x10U,
    _LV_PITCH_LADDER_MODE_LAST
};
typedef uint32_t lv_pitch_ladder_mode_t;

typedef struct {
    int label_value;
    int y_offset;
} lv_pitch_tick_info_t;

typedef struct {
    lv_style_t * indicator_style;
    lv_style_t * items_style;
} lv_pitch_ladder_section_t;

typedef struct {
    lv_obj_t obj;
    lv_ll_t section_ll;     /**< Linked list for the sections (stores lv_pitch_ladder_section_t)*/
    const char ** txt_src;
    lv_pitch_ladder_mode_t mode;
    int32_t pitch_angle;
    int32_t roll_angle;
    uint32_t post_draw          : 1;
    uint32_t widget_draw          : 1;
} lv_pitch_ladder_t;

extern const lv_obj_class_t lv_pitch_ladder_class;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create an pitch_ladder object
 * @param parent    pointer to an object, it will be the parent of the new pitch_ladder
 * @return          pointer to the created pitch_ladder
 */
lv_obj_t * lv_pitch_ladder_create(lv_obj_t * parent);

/*======================
 * Add/remove functions
 *=====================*/

/*=====================
 * Setter functions
 *====================*/
/**
 * Set the pitch value on a pitch_ladder
 * @param obj           pointer to a pitch_ladder object
 * @param pitch       pitch angle of the pitch_ladder
 */
void lv_pitch_ladder_set_angles(lv_obj_t * obj, int32_t pitch, int32_t roll);

/*=====================
 * Getter functions
 *====================*/


#endif /*APP_LIB_LV_PITCH_LADDER_H*/

