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
#include <app/lib/lv_pitch_ladder.h>

#include <core/lv_group.h>
#include <misc/lv_assert.h>
#include <misc/lv_math.h>
#include <draw/lv_draw.h>

#include <draw/lv_draw_line.h>
#include <draw/lv_draw_label.h>

#include <misc/lv_txt.h>
#include <core/lv_event.h>
#include <zephyr/logging/log.h>

#include <widgets/lv_canvas.h>
#include <widgets/lv_line.h>

#include <lvgl.h>

LOG_MODULE_REGISTER(lv_pitch_ladder, CONFIG_SENSOR_LOG_LEVEL);

/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_pitch_ladder_class)

#define LV_PITCH_LADDER_LABEL_TXT_LEN          (20U)
#define LV_PITCH_LADDER_DEFAULT_ANGLE_RANGE    ((uint32_t) 270U)
#define LV_PITCH_LADDER_DEFAULT_ROTATION       ((int32_t) 135U)
#define LV_PITCH_LADDER_TICK_IDX_DEFAULT_ID    ((uint32_t) 255U)
#define LV_PITCH_LADDER_DEFAULT_LABEL_GAP      ((uint32_t) 15U)

#define LV_PITCH_LADDER_NONE 256

#define LV_ATTRIBUTE_IMG_TEST

uint32_t LV_PITCH_EVENT_ROTATE = 0;
enum _lv_result_t {
    LV_RESULT_INVALID = 0, /*Typically indicates that the object is deleted (become invalid) in the action
                      function or an operation was failed*/
    LV_RESULT_OK,      /*The object is valid (no deleted) after the action*/
};
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

static void lv_pitch_ladder_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_pitch_ladder_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_pitch_ladder_event(const lv_obj_class_t * class_p, lv_event_t * event);

/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_pitch_ladder_class  = {
    .constructor_cb = lv_pitch_ladder_constructor,
    .destructor_cb = lv_pitch_ladder_destructor,
    .event_cb = lv_pitch_ladder_event,
    .instance_size = sizeof(lv_pitch_ladder_t),
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .base_class = &lv_obj_class,
    //.name = "pitch_ladder",
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_pitch_ladder_create(lv_obj_t * parent)
{
    //LV_LOG_INFO("begin");
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

void lv_pitch_ladder_set_angles(lv_obj_t * obj, int32_t pitch, int16_t roll)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    while(pitch >= 900) pitch -= 900;
    while(pitch < -900) pitch += 900;

    while(roll >= 1800) roll -= 1800;
    while(roll < -1800) roll += 1800;


    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    pitch_ladder->pitch_angle = pitch;
    pitch_ladder->roll_angle = roll;
    pitch_ladder->widget_draw = true;
    uint32_t btn_id = 0;
    lv_event_send((lv_obj_t *)pitch_ladder, LV_PITCH_EVENT_ROTATE, &btn_id);
    //lv_obj_invalidate(obj);
    //lv_refr_now(NULL);
}

void lv_pitch_ladder_set_dark_style(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    lv_draw_label_dsc_t *label_dsc = &(pitch_ladder->label_dsc);
    label_dsc->color = lv_color_make(0x00, 0x00, 0xFF);//(0x9C, 0x65, 0xCC);
    lv_draw_line_dsc_t *line_dsc = &(pitch_ladder->line_dsc);
    line_dsc->color = lv_color_make(0x00, 0x00, 0xFF);
    pitch_ladder->bg = lv_color_black();
}

void lv_pitch_ladder_set_light_style(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    lv_draw_label_dsc_t *label_dsc = &(pitch_ladder->label_dsc);
    label_dsc->color = lv_color_black();
    lv_draw_line_dsc_t *line_dsc = &(pitch_ladder->line_dsc);
    line_dsc->color = lv_color_black();
    pitch_ladder->bg = lv_color_white();
}
void lv_pitch_ladder_set_line_width(lv_obj_t * obj, lv_coord_t width)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    lv_draw_line_dsc_t *line_dsc = &(pitch_ladder->line_dsc);
    line_dsc->width = width;
}
/*=====================
 * Getter functions
 *====================*/



/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_pitch_ladder_draw_pitch_up( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    const lv_point_t pts[] = { { LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDER_HORIZ_LEND, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts1[] = { { LV_PITCH_LADDER_HORIZ_RSTART + LV_PITCH_LADDER_LABEL_W, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts2[] = { { LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2+LV_PITCH_LADDER_THICK + y}};
    const lv_point_t pts3[] = { { LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, { LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2+LV_PITCH_LADDER_THICK + y} };
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts1, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts2, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts3, 2, &(pitch_ladder->line_dsc));

}
static void lv_pitch_ladder_draw_pitch_down( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    const lv_point_t pts[] = { { LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDER_HORIZ_LEND, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts1[] = { { LV_PITCH_LADDER_HORIZ_RSTART + LV_PITCH_LADDER_LABEL_W, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts2[] = { { LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2-LV_PITCH_LADDER_THICK + y}};
    const lv_point_t pts3[] = { { LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, { LV_PITCH_LADDE_CANVAS_WIDTH-1 - LV_PITCH_LADDER_LADDER_WDIFF, LV_PITCH_LADDE_CANVAS_HEIGHT/2 - LV_PITCH_LADDER_THICK + y} };
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts1, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts2, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts3, 2, &(pitch_ladder->line_dsc));

}
static void lv_pitch_ladder_draw_horizon( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    const lv_point_t pts[] = { {0, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDER_HORIZ_LEND, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts1[] = { {LV_PITCH_LADDER_HORIZ_RSTART + LV_PITCH_LADDER_LABEL_W, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDE_CANVAS_WIDTH-1, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts1, 2, &(pitch_ladder->line_dsc));
}
static void lv_pitch_ladder_draw_aim( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    const lv_point_t pts[] = { {LV_PITCH_LADDE_CANVAS_WIDTH/2 - LV_PITCH_LADDER_AIM_W/2, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}, {LV_PITCH_LADDE_CANVAS_WIDTH/2 +LV_PITCH_LADDER_AIM_W/2, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + y}};
    const lv_point_t pts1[] = { {LV_PITCH_LADDE_CANVAS_WIDTH/2, LV_PITCH_LADDE_CANVAS_HEIGHT/2 - LV_PITCH_LADDER_AIM_W/2 +y}, {LV_PITCH_LADDE_CANVAS_WIDTH/2, LV_PITCH_LADDE_CANVAS_HEIGHT/2 + LV_PITCH_LADDER_AIM_W/2 + y}};
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts, 2, &(pitch_ladder->line_dsc));
    lv_canvas_draw_line(lv_obj_get_child(obj, 0), pts1, 2, &(pitch_ladder->line_dsc));
}
static void lv_pitch_ladder_draw_label( lv_obj_t * obj, int16_t x, int16_t y, int16_t tick_num)
{
    char buf[8] = {0};
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    if(tick_num >= 0) lv_snprintf(buf, sizeof(buf), " %d", (int16_t)tick_num); 
    else lv_snprintf(buf, sizeof(buf), "%d", (int16_t)tick_num);
    lv_canvas_draw_text(lv_obj_get_child(obj, 0), LV_PITCH_LADDE_CANVAS_WIDTH/2 + LV_PITCH_LADDER_LABEL_XOFFSET + x, LV_PITCH_LADDE_CANVAS_HEIGHT/2 -LV_PITCH_LADDER_FONT_SIZE/2 +y, 60, &(pitch_ladder->label_dsc), buf);
}

static void lv_pitch_ladder_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
   
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    _lv_ll_init(&pitch_ladder->section_ll, sizeof(lv_pitch_ladder_section_t));

    lv_obj_set_size((lv_obj_t *)pitch_ladder, LV_PITCH_LADDE_CANVAS_WIDTH, LV_PITCH_LADDE_CANVAS_WIDTH);
    lv_obj_align((lv_obj_t *)pitch_ladder, LV_ALIGN_CENTER, 0, 0);
    
    pitch_ladder->mode = LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM;
    pitch_ladder->post_draw = false;
    pitch_ladder->roll_angle = 0;
    pitch_ladder->pitch_angle = 0;
    pitch_ladder->txt_src = NULL;
    pitch_ladder->widget_draw = false;

    LV_PITCH_EVENT_ROTATE = lv_event_register_id();
    lv_obj_t *canvas = lv_canvas_create(obj);

    static lv_color_t buffer[3*LV_PITCH_LADDE_CANVAS_WIDTH*LV_PITCH_LADDE_CANVAS_HEIGHT];
    lv_canvas_set_buffer(canvas, buffer,LV_PITCH_LADDE_CANVAS_WIDTH, LV_PITCH_LADDE_CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_obj_align(canvas, LV_ALIGN_CENTER, LV_PITCH_LADDER_CANVAS_X_OFFSET, LV_PITCH_LADDER_CANVAS_Y_OFFSET);

    //lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    //lv_color_t c = lv_color_make(0x00, 0xFF, 0x00);
    lv_canvas_fill_bg(lv_obj_get_child(obj, 0), pitch_ladder->bg, LV_OPA_COVER); 

    lv_draw_label_dsc_init(&(pitch_ladder->label_dsc));
    lv_draw_line_dsc_init(&(pitch_ladder->line_dsc));

    //lv_obj_set_style_bg_color((lv_obj_t *)pitch_ladder, lv_color_hex(0xFF0000), LV_PART_MAIN);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_pitch_ladder_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_pitch_ladder_section_t * section;
    while(pitch_ladder->section_ll.head) {
        section = _lv_ll_get_head(&pitch_ladder->section_ll);
        _lv_ll_remove(&pitch_ladder->section_ll, section);
    }
    _lv_ll_clear(&pitch_ladder->section_ll);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_pitch_ladder_redraw( lv_obj_t * obj, lv_event_t * event)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *) obj;

    //lv_color_t c = lv_color_make(0xFF, 0x00, 0x00);
    lv_canvas_fill_bg(lv_obj_get_child(obj, 0), pitch_ladder->bg, LV_OPA_COVER); 
    int32_t yoffset = (pitch_ladder->pitch_angle%10)*LV_PITCH_LADDER_SPACE/10;

    int scale = LV_PITCH_LADDER_PITCH_SCALE;// tick lenght
    int tickRange = LV_PITCH_LADDER_ROLL_TICK_RANGE;  // number of ticks
    int scaleStart = (floor(pitch_ladder->pitch_angle/scale)*scale-floor(scale*tickRange/2));

    //LOG_INF("yoff = %d, scaleStart = %d", yoffset, scaleStart);
    lv_pitch_tick_info_t scaleValues[LV_PITCH_LADDER_ROLL_TICK_RANGE + 1] = {
        {.y_offset = LV_PITCH_LADDER_SPACE*2 + yoffset, .label_value = scaleStart},
        {.y_offset = LV_PITCH_LADDER_SPACE + yoffset, .label_value = scaleStart + LV_PITCH_LADDER_PITCH_SCALE},
        {.y_offset = yoffset, .label_value = scaleStart + 2*LV_PITCH_LADDER_PITCH_SCALE},
        {.y_offset = -LV_PITCH_LADDER_SPACE + yoffset, .label_value = scaleStart + 3*LV_PITCH_LADDER_PITCH_SCALE},
        {.y_offset = -LV_PITCH_LADDER_SPACE*2 + yoffset, .label_value = scaleStart + 4*LV_PITCH_LADDER_PITCH_SCALE}
    };

    for(int i = 0; i < LV_PITCH_LADDER_ROLL_TICK_RANGE + 1; i++){
        lv_pitch_ladder_draw_label(obj, 0, scaleValues[i].y_offset, scaleValues[i].label_value);
        if(scaleValues[i].label_value == 0){
            lv_pitch_ladder_draw_horizon(obj, 0, scaleValues[i].y_offset, 0);
        }
        if(scaleValues[i].label_value > 0){
            lv_pitch_ladder_draw_pitch_up(obj, 0, scaleValues[i].y_offset, 0);
        }
        if(scaleValues[i].label_value < 0){
            lv_pitch_ladder_draw_pitch_down(obj, 0, scaleValues[i].y_offset, 0);
        }
    }
    lv_pitch_ladder_draw_aim(obj, 0, 0, 0);
    
}
static void lv_pitch_ladder_event(const lv_obj_class_t * class_p, lv_event_t * event)
{
    LV_UNUSED(class_p);

    //Call the ancestor's event handler
    uint8_t  res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t event_code = lv_event_get_code(event);
    lv_obj_t * obj = lv_event_get_target(event);
    //lv_obj_t * obj = lv_event_get_current_target(event);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *) obj;
    LV_UNUSED(pitch_ladder);
  
    if(event_code == LV_PITCH_EVENT_ROTATE) {
        lv_img_set_angle(lv_obj_get_child(obj, 0), pitch_ladder->roll_angle);
        lv_pitch_ladder_redraw(obj, event);
    }
    if(event_code == LV_EVENT_DRAW_MAIN) {
        //lv_draw_ctx_t *layer = lv_event_get_draw_ctx(event);
        //lv_canvas_fill_bg(obj, pitch_ladder->bg, LV_OPA_COVER);
        //lv_obj_set_style_bg_color(obj, lv_color_hex(0x00FF00), LV_PART_MAIN);
    }
    if(event_code == LV_EVENT_DRAW_POST) {

    }
    else if(event_code == LV_EVENT_REFR_EXT_DRAW_SIZE) {

    }
    else {

    }

}
