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


/*********************
 *      INCLUDES
 *********************/
#include <math.h>
#include <app/lib/lv_compass.h>

#include <core/lv_group.h>
#include <misc/lv_assert.h>
#include <misc/lv_math.h>
#include <draw/lv_draw.h>
//#include <draw/lv_draw_line.h>
#include <draw/lv_draw_arc.h>
#include <misc/lv_txt.h>
#include <core/lv_event.h>
//#include "lv_hud_common.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(lv_compass, CONFIG_SENSOR_LOG_LEVEL);

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_comapss_class)

#define LV_COMAPSS_LABEL_TXT_LEN          (20U)
#define LV_COMAPSS_DEFAULT_ANGLE_RANGE    ((uint32_t) 270U)
#define LV_COMAPSS_DEFAULT_ROTATION       ((int32_t) 135U)
#define LV_COMAPSS_TICK_IDX_DEFAULT_ID    ((uint32_t) 255U)
#define LV_COMAPSS_DEFAULT_LABEL_GAP      ((uint32_t) 15U)

#define LV_COMAPSS_NONE 256
/**********************
 *      TYPEDEFS
 **********************/
typedef struct _lv_event_dsc_t {
    lv_event_cb_t cb;
    void * user_data;
    lv_event_code_t filter : 8;
} lv_event_dsc_t;
/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_comapss_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_comapss_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_comapss_event(const lv_obj_class_t * class_p, lv_event_t * event);



/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_comapss_class  = {
    .constructor_cb = lv_comapss_constructor,
    .destructor_cb = lv_comapss_destructor,
    .event_cb = lv_comapss_event,
    .instance_size = sizeof(lv_comapss_t),
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .base_class = &lv_obj_class,
    //.name = "comapss",
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_compass_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*======================
 * Add/remove functions
 *=====================*/

/*
 * New object specific "add" or "remove" functions come here
 */

/*=====================
 * Setter functions
 *====================*/
void lv_comapss_angle(lv_obj_t * obj, int32_t angle)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->heading_angle = angle;
    comapss->widget_draw = true;

    lv_obj_invalidate(obj);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_comapss_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    _lv_ll_init(&comapss->section_ll, sizeof(lv_comapss_section_t));

    lv_obj_set_size((lv_obj_t *)comapss, COMPAS_WIDTH, COMPAS_HEIGHT);
    lv_obj_align((lv_obj_t *)comapss, LV_ALIGN_TOP_MID, 0, 0);

    comapss->post_draw = false;
    comapss->draw_ticks_on_top = false;
    comapss->txt_src = NULL;
    comapss->heading_angle = 0;
    comapss->widget_draw = false;

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_comapss_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{

    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_comapss_section_t * section;
    while(comapss->section_ll.head) {
        section = _lv_ll_get_head(&comapss->section_ll);
        _lv_ll_remove(&comapss->section_ll, section);
        //lv_free(section);
        //lv_free((lv_obj_t *)section);
    }
    _lv_ll_clear(&comapss->section_ll);

    LV_TRACE_OBJ_CREATE("finished");
}
static void lv_comapss_draw_label( lv_draw_ctx_t * layer, lv_draw_label_dsc_t *dsc, int16_t x, int16_t y, int16_t angle, int tick_num)
{
    char buf[8] = {0};
    lv_snprintf(buf, sizeof(buf), "%d", (int16_t)tick_num);
        
    lv_area_t label_coords;

    label_coords.x1 = COMPAS_WIDTH / 2 + x;
    label_coords.y1 = y;
    label_coords.x2 = 30 + COMPAS_WIDTH / 2 + x;
    label_coords.y2 = 14 + y;

    lv_draw_label((struct _lv_draw_ctx_t *)layer, dsc, &label_coords, buf, NULL);

}
static void lv_comapss_draw_tick_major( lv_draw_ctx_t *layer, lv_draw_line_dsc_t *dsc, int16_t x, int16_t y, int16_t angle)
{
    lv_point_t tick_point_a;
    lv_point_t tick_point_b;
    tick_point_a.x = COMPAS_WIDTH / 2 + x;
    tick_point_a.y = COMPAS_FONT_HEIGHT + y;
    tick_point_b.x = COMPAS_WIDTH / 2 + x;
    tick_point_b.y = COMPAS_FONT_HEIGHT + COMPAS_MAJOR_TICK_LENGHT + y;
    lv_draw_line((struct _lv_draw_ctx_t *)layer, dsc, &tick_point_a, &tick_point_b);
}
static void lv_comapss_draw_tick_minor( lv_draw_ctx_t *layer, lv_draw_line_dsc_t *dsc, int16_t x, int16_t y, int16_t angle)
{
    lv_point_t tick_point_a;
    lv_point_t tick_point_b;
    tick_point_a.x = COMPAS_WIDTH / 2 + COMPAS_TICK_SPACING /2 + x;
    tick_point_a.y = COMPAS_FONT_HEIGHT;
    tick_point_b.x = COMPAS_WIDTH / 2 + COMPAS_TICK_SPACING /2 + x;
    tick_point_b.y = COMPAS_FONT_HEIGHT + COMPAS_MINOR_TICK_LENGHT;
    lv_draw_line((struct _lv_draw_ctx_t *)layer, dsc, &tick_point_a, &tick_point_b);
}
static int32_t lv_compass_limit(int32_t value)
{
    int32_t result = value;
    if(value < 0) result = result + 360;
    if(value > 359) result = result - 360;
    return result;
}
static void lv_comapss_redraw( lv_obj_t * obj, lv_event_t * event)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_draw_ctx_t *layer = lv_event_get_draw_ctx(event);

    int32_t x_offset = -(comapss->heading_angle%10)*LV_COMPASS_SPACE/10;

    int16_t scale = LV_COMPASS_SCALE;// tick lenght
    int16_t tickRange = LV_COMPASS_TICK_RANGE;  // number of ticks
    int16_t scaleStart = (floor(comapss->heading_angle/scale)*scale-floor(scale*tickRange/2));

    //LOG_INF("start = %d, offset = %d, heading = %d", scaleStart, xoffset, heading);
    lv_compass_tick_info_t scaleValues[LV_COMPASS_TICK_RANGE + 1] = {
        {.x_offset = -LV_COMPASS_SPACE*2 + x_offset, .label_value = lv_compass_limit(scaleStart)},
        {.x_offset = -LV_COMPASS_SPACE + x_offset, .label_value = lv_compass_limit(scaleStart + LV_COMPASS_SCALE)}, 
        {.x_offset = x_offset, .label_value = lv_compass_limit(scaleStart + 2*LV_COMPASS_SCALE)}, 
        {.x_offset = LV_COMPASS_SPACE + x_offset, .label_value = lv_compass_limit(scaleStart+ 3*LV_COMPASS_SCALE)}, 
        {.x_offset = LV_COMPASS_SPACE*2 + x_offset, .label_value = lv_compass_limit(scaleStart + 4*LV_COMPASS_SCALE)},     
    };

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    //lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);
    lv_draw_line_dsc_t main_line_dsc;
    lv_draw_line_dsc_init(&main_line_dsc);
    //lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &main_line_dsc);
    for(int i = 0; i < LV_COMPASS_TICK_RANGE + 1; i++){
        lv_comapss_draw_label((struct _lv_draw_ctx_t *)layer, &label_dsc, scaleValues[i].x_offset, 0, 0, scaleValues[i].label_value);
        lv_comapss_draw_tick_major((struct _lv_draw_ctx_t *)layer, &main_line_dsc, scaleValues[i].x_offset, 0, 0);
        lv_comapss_draw_tick_minor((struct _lv_draw_ctx_t *)layer, &main_line_dsc, scaleValues[i].x_offset, 0, 0);
    }
    lv_comapss_draw_tick_major((struct _lv_draw_ctx_t *)layer, &main_line_dsc, 0, 1 + COMPAS_MAJOR_TICK_LENGHT, 0);
    lv_comapss_draw_label((struct _lv_draw_ctx_t *)layer, &label_dsc, 2, COMPAS_MAJOR_TICK_LENGHT + COMPAS_FONT_HEIGHT, 0, comapss->heading_angle);

}

static void lv_comapss_event(const lv_obj_class_t * class_p, lv_event_t * event)
{
    LV_UNUSED(class_p);

    uint8_t  res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t event_code = lv_event_get_code(event);
    lv_obj_t * obj = lv_event_get_current_target(event);
    lv_comapss_t * comapss = (lv_comapss_t *) obj;
    LV_UNUSED(comapss);

    if(event_code == LV_EVENT_DRAW_MAIN) {
        lv_comapss_redraw(obj, event);
    }
    if(event_code == LV_EVENT_DRAW_POST) {

    }
    else if(event_code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        // NOTE: Extend comapss draw size so the first tick label can be shown
    }
    else {
        // Nothing to do. Invalid event 
    }
}
