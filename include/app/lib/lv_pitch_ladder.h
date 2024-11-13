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

// #ifndef A3688119_289C_497B_9268_272425E79E01
// #define A3688119_289C_497B_9268_272425E79E01

// #ifndef B70830EF_BB65_4CEE_A11C_5BFFAC319948
// #define B70830EF_BB65_4CEE_A11C_5BFFAC319948

#ifndef APP_LIB_LV_PITCH_LADDER_H_
#define APP_LIB_LV_PITCH_LADDER_H_

// #ifdef __cplusplus
// extern "C" {
// #endif

#define LV_USE_PITCH_LADDER 1
/*********************
 *      INCLUDES
 *********************/
// #include "../../lv_conf_internal.h"
#include <lv_conf_internal.h>

//#if LV_USE_PITCH_LADDER != 0

// #include "../../core/lv_obj.h"
// #include "../line/lv_line.h"
// #include "../image/lv_image.h"

#include <core/lv_obj.h>
#include <widgets/lv_line.h>
//#include <widgets/lv_image.h>

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

/**********************
 *      TYPEDEFS
 **********************/
// enum _lv_result_t {
//     LV_RESULT_INVALID = 0, /*Typically indicates that the object is deleted (become invalid) in the action
//                       function or an operation was failed*/
//     LV_RESULT_OK,      /*The object is valid (no deleted) after the action*/
// };
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
    lv_style_t * main_style;
    lv_style_t * indicator_style;
    lv_style_t * items_style;
    int32_t minor_range;
    int32_t major_range;
    uint32_t first_tick_idx_in_section;
    uint32_t last_tick_idx_in_section;
    uint32_t first_tick_idx_is_major;
    uint32_t last_tick_idx_is_major;
    int32_t first_tick_in_section_width;
    int32_t last_tick_in_section_width;
    lv_point_t first_tick_in_section;
    lv_point_t last_tick_in_section;
} lv_pitch_ladder_section_t;

typedef struct {
    lv_obj_t obj;
    lv_ll_t section_ll;     /**< Linked list for the sections (stores lv_pitch_ladder_section_t)*/
    const char ** txt_src;
    lv_pitch_ladder_mode_t mode;
    int32_t range_min;
    int32_t range_max;
    int32_t dir_angle;
    int32_t pitch_angle;
    int32_t roll_angle;
    uint32_t total_tick_count   : 15;
    uint32_t major_tick_every   : 15;
    uint32_t label_enabled      : 1;
    uint32_t post_draw          : 1;
    uint32_t draw_ticks_on_top  : 1;
    /* Round pitch_ladder */
    uint32_t angle_range;
    int32_t rotation;
    /* Private properties */
    int32_t custom_label_cnt;
    int32_t last_tick_width;
    int32_t first_tick_width;
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
 * Set pitch_ladder mode. 
 * @param obj       pointer the pitch_ladder object
 * @param mode      the new pitch_ladder mode
 */
void lv_pitch_ladder_set_mode(lv_obj_t * obj, lv_pitch_ladder_mode_t mode);

/**
 * Set pitch_ladder total tick count (including minor and major ticks)
 * @param obj       pointer the pitch_ladder object
 * @param total_tick_count    New total tick count
 */
void lv_pitch_ladder_set_total_tick_count(lv_obj_t * obj, uint32_t total_tick_count);

/**
 * Sets how often the major tick will be drawn
 * @param obj                 pointer the pitch_ladder object
 * @param major_tick_every    the new count for major tick drawing
 */
void lv_pitch_ladder_set_major_tick_every(lv_obj_t * obj, uint32_t major_tick_every);

/**
 * Sets label visibility
 * @param obj           pointer the pitch_ladder object
 * @param show_label    true/false to enable tick label
 */
void lv_pitch_ladder_set_label_show(lv_obj_t * obj, bool show_label);

/**
 * Set the minimal and maximal values on a pitch_ladder
 * @param obj       pointer to a pitch_ladder object
 * @param min       minimum value of the pitch_ladder
 * @param max       maximum value of the pitch_ladder
 */
void lv_pitch_ladder_set_range(lv_obj_t * obj, int32_t min, int32_t max);

/**
 * Set the direction angle value on a pitch_ladder
 * @param obj           pointer to a pitch_ladder object
 * @param direction       direction angle of the pitch_ladder
 */
void lv_pitch_ladder_set_direction_angle(lv_obj_t * obj, int32_t direction);

/**
 * Set the pitch value on a pitch_ladder
 * @param obj           pointer to a pitch_ladder object
 * @param pitch       pitch angle of the pitch_ladder
 */
void lv_pitch_ladder_set_pitch(lv_obj_t * obj, int32_t pitch);

/**
 * Set the roll value on a pitch_ladder
 * @param obj           pointer to a pitch_ladder object
 * @param roll       roll angle of the pitch_ladder
 */
void lv_pitch_ladder_set_roll(lv_obj_t * obj, int32_t roll);

/**
 * Limit the tick values to 0 .. 360
 * @param value           pointer to a pitch_ladder object
 */
int32_t lv_pitch_ladder_limit(int32_t value);

/**
 * Set properties specific to round pitch_ladder
 * @param obj           pointer to a pitch_ladder object
 * @param angle_range   the angular range of the pitch_ladder
 */
void lv_pitch_ladder_set_angle_range(lv_obj_t * obj, uint32_t angle_range);

/**
 * Set properties specific to round pitch_ladder
 * @param obj       pointer to a pitch_ladder object
 * @param rotation  the angular offset from the 3 o'clock position (clock-wise)
 */
void lv_pitch_ladder_set_rotation(lv_obj_t * obj, int32_t rotation);

/**
 * Point the needle to the corresponding value through the line
 * @param obj              pointer to a pitch_ladder object
 * @param needle_line      needle_line of the pitch_ladder. The line points will be allocated and
 *                         managed by the pitch_ladder unless the line point array was previously set
 *                         using `lv_line_set_points_mutable`.
 * @param needle_length    length of the needle
 *                         needle_length>0 needle_length=needle_length;
 *                         needle_length<0 needle_length=radius-|needle_length|;
 * @param value            needle to point to the corresponding value
 */
void lv_pitch_ladder_set_line_needle_value(lv_obj_t * obj, lv_obj_t * needle_line, int32_t needle_length,
                                    int32_t value);

/**
 * Point the needle to the corresponding value through the image,
   image must point to the right. E.g. -O------>
 * @param obj              pointer to a pitch_ladder object
 * @param needle_img       needle_img of the pitch_ladder
 * @param value            needle to point to the corresponding value
 */
void lv_pitch_ladder_set_image_needle_value(lv_obj_t * obj, lv_obj_t * needle_img, int32_t value);

/**
 * Set custom text source for major ticks labels
 * @param obj       pointer to a pitch_ladder object
 * @param txt_src   pointer to an array of strings which will be display at major ticks
 */
void lv_pitch_ladder_set_text_src(lv_obj_t * obj, const char * txt_src[]);

/**
 * Draw the pitch_ladder after all the children are drawn
 * @param obj       pointer to a pitch_ladder object
 * @param en        true: enable post draw
 */
void lv_pitch_ladder_set_post_draw(lv_obj_t * obj, bool en);

/**
 * Draw the pitch_ladder ticks on top of all parts
 * @param obj       pointer to a pitch_ladder object
 * @param en        true: enable draw ticks on top of all parts
 */
void lv_pitch_ladder_set_draw_ticks_on_top(lv_obj_t * obj, bool en);

/**
 * Add a section to the given pitch_ladder
 * @param obj       pointer to a pitch_ladder object
 * @return          pointer to the new section
 */
lv_pitch_ladder_section_t * lv_pitch_ladder_add_section(lv_obj_t * obj);

/**
 * Set the range for the given pitch_ladder section
 * @param section       pointer to a pitch_ladder section object
 * @param minor_range   section new minor range
 * @param major_range   section new major range
 */
void lv_pitch_ladder_section_set_range(lv_pitch_ladder_section_t * section, int32_t minor_range, int32_t major_range);

/**
 * Set the style of the part for the given pitch_ladder section
 * @param section   pointer to a pitch_ladder section object
 * @param part      the part for the section, e.g. LV_PART_INDICATOR
 * @param section_part_style Pointer to the section part style
 */
void lv_pitch_ladder_section_set_style(lv_pitch_ladder_section_t * section, lv_part_t part, lv_style_t * section_part_style);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get pitch_ladder mode.
 * @param obj   pointer the pitch_ladder object
 * @return      Pitch_ladder mode
 */
lv_pitch_ladder_mode_t lv_pitch_ladder_get_mode(lv_obj_t * obj);

/**
 * Get pitch_ladder total tick count (including minor and major ticks)
 * @param obj   pointer the pitch_ladder object
 * @return      Pitch_ladder total tick count
 */
int32_t lv_pitch_ladder_get_total_tick_count(lv_obj_t * obj);

/**
 * Gets how often the major tick will be drawn
 * @param obj   pointer the pitch_ladder object
 * @return      Pitch_ladder major tick every count
 */
int32_t lv_pitch_ladder_get_major_tick_every(lv_obj_t * obj);

/**
 * Gets label visibility
 * @param obj   pointer the pitch_ladder object
 * @return      true if tick label is enabled, false otherwise
 */
bool lv_pitch_ladder_get_label_show(lv_obj_t * obj);

/**
 * Get angle range of a round pitch_ladder
 * @param obj   pointer to a pitch_ladder object
 * @return      Pitch_ladder angle_range
 */
uint32_t lv_pitch_ladder_get_angle_range(lv_obj_t * obj);

/**
 * Get the min range for the given pitch_ladder section
 * @param obj   pointer to a pitch_ladder section object
 * @return      section minor range
 */
int32_t lv_pitch_ladder_get_range_min_value(lv_obj_t * obj);

/**
 * Get the max range for the given pitch_ladder section
 * @param obj   pointer to a pitch_ladder section object
 * @return      section max range
 */
int32_t lv_pitch_ladder_get_range_max_value(lv_obj_t * obj);


void lv_pitch_ladder_set_angle(lv_obj_t * obj, int16_t rotation);
/**********************
 *      MACROS
 **********************/

//#endif /*LV_USE_PITCH_LADDER*/

// #ifdef __cplusplus
// } /*extern "C"*/
// #endif

#endif /*APP_LIB_LV_PITCH_LADDER_H*/


// #endif /* B70830EF_BB65_4CEE_A11C_5BFFAC319948 */


// #endif /* A3688119_289C_497B_9268_272425E79E01 */
