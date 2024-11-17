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
#include <draw/lv_img_buf.h>
#include <draw/lv_draw_label.h>
//#include <draw/lv_draw_arc.h>
//#include <core/lv_obj_style_gen.h>
//#include <extra/others/lv_others.h>
#include <extra/others/snapshot/lv_snapshot.h>

#include <misc/lv_txt.h>
#include <core/lv_event.h>
#include <zephyr/logging/log.h>

//#include <draw/lv_draw_img.h>
#include <widgets/lv_img.h>
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

/////////////////////////////
//#define LV_USE_IMG
#define LV_ATTRIBUTE_IMG_TEST
//lv_obj_t * panel1;
lv_obj_t * canvasNum;
lv_color_t bufferNum[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 18)];



//const static lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 32)];
//lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 32)];
//LV_IMG_DECLARE(test);
//extern lv_img_dsc_t test;
///////////////////////

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

//static void pitch_ladder_draw_main(lv_obj_t * obj, lv_event_t * event);
static void pitch_ladder_draw_indicator(lv_obj_t * obj, lv_event_t * event);
// static void pitch_ladder_draw_needle(lv_obj_t * obj, lv_event_t * event);
static void pitch_ladder_draw_label(lv_obj_t * obj, lv_event_t * event, lv_draw_label_dsc_t * label_dsc,
                              const uint32_t major_tick_idx, const int32_t tick_value, lv_point_t * tick_point_b, const uint32_t tick_idx, bool above);
static void pitch_ladder_get_tick_points_offset(lv_obj_t * obj, uint32_t tick_idx, int32_t fine_tick_val, int8_t tick_type,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b);
static void pitch_ladder_get_label_coords(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text, lv_point_t * tick_point,
                                   lv_area_t * label_coords, bool above);
static void pitch_ladder_build_custom_label_text(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text,
                                          const uint16_t major_tick_idx);

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

//lv_obj_t * panel1;
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

void lv_pitch_ladder_set_mode(lv_obj_t * obj, lv_pitch_ladder_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->mode = mode;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_total_tick_count(lv_obj_t * obj, uint32_t total_tick_count)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->total_tick_count = total_tick_count;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_major_tick_every(lv_obj_t * obj, uint32_t major_tick_every)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->major_tick_every = major_tick_every;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_label_show(lv_obj_t * obj, bool show_label)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->label_enabled = show_label;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_range(lv_obj_t * obj, int32_t min, int32_t max)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_obj_invalidate(obj);
}

int32_t lv_pitch_ladder_limit(int32_t value)
{
    if(value < 0) value = value + 360;
    if(value > 359) value = value -360;
    return value;
}

void lv_pitch_ladder_set_direction_angle(lv_obj_t * obj, int32_t direction)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->dir_angle = direction;

    int scale = 10; // tick lenght
    int tickRange = 2;  // number of ticks
    int scaleStart = lv_pitch_ladder_limit(floor(pitch_ladder->dir_angle/scale)*scale-floor(scale*tickRange/2));
    int scaleStop = lv_pitch_ladder_limit(floor(pitch_ladder->dir_angle/scale)*scale+ floor(scale*tickRange/2));

    
    pitch_ladder->range_min = scaleStart;//direction - 15;
    pitch_ladder->range_max = scaleStop;//direction + 10;;

    lv_obj_invalidate(obj);
}
void lv_pitch_ladder_set_pitch(lv_obj_t * obj, int32_t pitch)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->pitch_angle = pitch;

    lv_obj_invalidate(obj);
}
void lv_pitch_ladder_set_roll(lv_obj_t * obj, int32_t roll)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->roll_angle = roll;

    lv_obj_invalidate(obj);
}


void lv_pitch_ladder_set_angle_range(lv_obj_t * obj, uint32_t angle_range)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->angle_range = angle_range;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_rotation(lv_obj_t * obj, int32_t rotation)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->rotation = rotation;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_line_needle_value(lv_obj_t * obj, lv_obj_t * needle_line, int32_t needle_length,
                                    int32_t value)
{

}

void lv_pitch_ladder_set_image_needle_value(lv_obj_t * obj, lv_obj_t * needle_img, int32_t value)
{
    int32_t angle;
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    if((pitch_ladder->mode != LV_PITCH_LADDER_MODE_ROUND_INNER) &&
       (pitch_ladder->mode != LV_PITCH_LADDER_MODE_ROUND_OUTER)) {
        return;
    }

    if(value < pitch_ladder->range_min) {
        angle = 0;
    }
    else if(value > pitch_ladder->range_max) {
        angle = pitch_ladder->angle_range;
    }
    else {
        angle = pitch_ladder->angle_range * (value - pitch_ladder->range_min) / (pitch_ladder->range_max - pitch_ladder->range_min);
    }
}

void lv_pitch_ladder_set_text_src(lv_obj_t * obj, const char * txt_src[])
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->txt_src = txt_src;
    pitch_ladder->custom_label_cnt = 0;
    if(pitch_ladder->txt_src) {
        int32_t idx;
        for(idx = 0; txt_src[idx]; ++idx) {
            pitch_ladder->custom_label_cnt++;
        }
    }

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_post_draw(lv_obj_t * obj, bool en)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->post_draw = en;

    lv_obj_invalidate(obj);
}

void lv_pitch_ladder_set_draw_ticks_on_top(lv_obj_t * obj, bool en)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    pitch_ladder->draw_ticks_on_top = en;

    lv_obj_invalidate(obj);
}

lv_pitch_ladder_section_t * lv_pitch_ladder_add_section(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_pitch_ladder_section_t * section = _lv_ll_ins_head(&pitch_ladder->section_ll);
    LV_ASSERT_MALLOC(section);
    if(section == NULL) return NULL;

    /* Section default values */
    section->main_style = NULL;
    section->indicator_style = NULL;
    section->items_style = NULL;
    section->minor_range = 0U;
    section->major_range = 0U;
    section->first_tick_idx_in_section = LV_PITCH_LADDER_TICK_IDX_DEFAULT_ID;
    section->last_tick_idx_in_section = LV_PITCH_LADDER_TICK_IDX_DEFAULT_ID;
    section->first_tick_idx_is_major = 0U;
    section->last_tick_idx_is_major = 0U;
    section->first_tick_in_section_width = 0U;
    section->last_tick_in_section_width = 0U;

    return section;
}

void lv_pitch_ladder_section_set_range(lv_pitch_ladder_section_t * section, int32_t minor_range, int32_t major_range)
{
    if(NULL == section) return;

    section->minor_range = minor_range;
    section->major_range = major_range;
}

void lv_pitch_ladder_section_set_style(lv_pitch_ladder_section_t * section, lv_part_t part, lv_style_t * section_part_style)
{
    if(NULL == section) return;

    switch(part) {
        case LV_PART_MAIN:
            section->main_style = section_part_style;
            break;
        case LV_PART_INDICATOR:
            section->indicator_style = section_part_style;
            break;
        case LV_PART_ITEMS:
            section->items_style = section_part_style;
            break;
        default:
            /* Invalid part */
            break;
    }
}


static void lv_pitch_ladder_draw_numeric2( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    //lv_obj_t *canvas_num = lv_canvas_create(obj);
    lv_obj_t *canvas = lv_canvas_create(obj);
    lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 18)];
    lv_canvas_set_buffer(canvas, buffer,32, 18, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_img_set_angle(canvas, angle);
    lv_obj_align(canvas, LV_ALIGN_CENTER, x, y);

    lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    lv_canvas_fill_bg(canvas, c, LV_OPA_COVER); 

    lv_draw_label_dsc_t lbl_dsc;
    lv_draw_label_dsc_init(&lbl_dsc);

    lv_canvas_draw_text(canvas, 2, 2, 30, &lbl_dsc, "-90");
}

void lv_pitch_ladder_set_angle(lv_obj_t * obj, int16_t rotation)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    //pitch_ladder->roll_angle = rotation;
    pitch_ladder->roll_angle += 30;
    pitch_ladder->widget_draw = true;
    //lv_refr_now(NULL);
    //LOG_INF("coords %d, %d, %d, %d", panel1->coords.x1, panel1->coords.x2, panel1->coords.y1, panel1->coords.y2);
    //lv_obj_invalidate(obj);
    //lv_pitch_ladder_draw_numeric2(obj, -40, -40, pitch_ladder->roll_angle);
    lv_obj_invalidate(obj);
}


/*=====================
 * Getter functions
 *====================*/

lv_pitch_ladder_mode_t lv_pitch_ladder_get_mode(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->mode;
}

int32_t lv_pitch_ladder_get_total_tick_count(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->total_tick_count;
}

int32_t lv_pitch_ladder_get_major_tick_every(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->major_tick_every;
}

bool lv_pitch_ladder_get_label_show(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->label_enabled;
}

uint32_t lv_pitch_ladder_get_angle_range(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->angle_range;
}

int32_t lv_pitch_ladder_get_range_min_value(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->range_min;
}

int32_t lv_pitch_ladder_get_range_max_value(lv_obj_t * obj)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    return pitch_ladder->range_max;
}

/*=====================
 * Other functions
 *====================*/
void lv_img_set_angle_img(lv_obj_t * obj, int16_t angle)
{
    //LOG_INF("angle %d", angle);
    while(angle >= 3600) angle -= 3600;
    while(angle < 0) angle += 3600;

    lv_img_t * img = (lv_img_t *)obj;
    if(angle == img->angle) return;
    LOG_INF("angles %d, %d", angle, img->angle);
    lv_obj_update_layout(obj);  /*Be sure the object's size is calculated*/
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);
    lv_area_t a;
    _lv_img_buf_get_transformed_area(&a, w, h, img->angle, img->zoom, &img->pivot);
    
    LOG_INF("coordsimg %d, %d, %d, %d", a.x1, a.x2, a.y1, a.y2);
    a.x1 += obj->coords.x1;
    a.y1 += obj->coords.y1;
    a.x2 += obj->coords.x1;
    a.y2 += obj->coords.y1;
    
    // obj->coords.x1 = 56+7;
    // obj->coords.y1 = 56;
    // obj->coords.x2 = 71-7;
    // obj->coords.y2 = 71;

    lv_obj_invalidate_area(obj, &a);

    img->angle = angle;

    /* Disable invalidations because lv_obj_refresh_ext_draw_size would invalidate
     * the whole ext draw area */
    lv_disp_t * disp = lv_obj_get_disp(obj);
    lv_disp_enable_invalidation(disp, false);
    lv_obj_refresh_ext_draw_size(obj);
    lv_disp_enable_invalidation(disp, true);

    _lv_img_buf_get_transformed_area(&a, w, h, img->angle, img->zoom, &img->pivot);
    LOG_INF("coordsimg1 %d, %d, %d, %d", a.x1, a.x2, a.y1, a.y2);
    a.x1 += obj->coords.x1;
    a.y1 += obj->coords.y1;
    a.x2 += obj->coords.x1;
    a.y2 += obj->coords.y1;
    lv_obj_invalidate_area(obj, &a);
    //LOG_INF("pivot %d, %d", &img->pivot, img->angle);
    //LOG_INF("coordsimg %d, %d, %d, %d", a->coords.x1, a->coords.x2, a->coords.y1, a->coords.y2);
    LOG_INF("coordsimg2 %d, %d, %d, %d", a.x1, a.x2, a.y1, a.y2);
    LOG_INF("coordsibj %d, %d, %d, %d", obj->coords.x1, obj->coords.x2, obj->coords.y1, obj->coords.y2);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
static void reset_img(lv_img_dsc_t * dsc, uint8_t v)
{
    lv_memset(dsc->data,v, dsc->data_size);
}

#define CANVAS_W  30
#define CANVAS_H  30

#define CANVAS_WIDTH  50
#define CANVAS_HEIGHT  50
/*void lv_example_canvas_2(lv_obj_t * obj)
{


    static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(CANVAS_WIDTH, CANVAS_HEIGHT)];

    canvas = lv_canvas_create(obj);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_INDEXED_1BIT);
    lv_canvas_set_palette(canvas, 0, LV_COLOR_CHROMA_KEY);
    lv_canvas_set_palette(canvas, 1, lv_palette_main(LV_PALETTE_RED));

    lv_color_t c0;
    lv_color_t c1;

    c0.full = 0;
    c1.full = 1;

    lv_canvas_fill_bg(canvas, c1, LV_OPA_COVER);

    uint32_t x;
    uint32_t y;
    for(y = 10; y < 30; y++) {
        for(x = 5; x < 20; x++) {
            lv_canvas_set_px_color(canvas, x, y, c0);
        }
    }


}
void lv_example_line_1(lv_obj_t * obj)
{

    static lv_point_t line_points[] = { {5, 5}, {20, 20}, {30, 10}, {40, 30}, {50, 10} };

    static lv_style_t style_line;
    lv_style_init(&style_line);
    lv_style_set_line_width(&style_line, 8);
    lv_style_set_line_color(&style_line, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_rounded(&style_line, true);

    lv_obj_t * line1;
    line1 = lv_line_create(obj);
    lv_line_set_points(line1, line_points, 5);    
    lv_obj_add_style(line1, &style_line, 0);
    lv_obj_center(line1);
}
static void lv_pitch_ladder_draw_indicator( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{

    //LV_DRAW_BUF_DEFINE_STATIC(draw_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
    //LV_DRAW_BUF_INIT_STATIC(draw_buf);
    const int32_t roll_value = 90;

    //lv_draw_ctx_t *canvas = lv_event_get_draw_ctx(event);
    canvas = lv_canvas_create(obj);
    //lv_obj_t * canvas = lv_canvas_create(obj);
    //lv_canvas_set_buffer(canvas,&test,32, 32, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_canvas_set_buffer(canvas, buffer,32, 32, LV_IMG_CF_TRUE_COLOR_ALPHA);

    lv_color_t c = lv_color_make(0x00, 0xFF, 0x00);
    lv_canvas_set_px_color(canvas, 10, 10, c);

    lv_canvas_fill_bg(canvas, c, LV_OPA_COVER);

    const lv_point_t pts[] = { {5, 5}, {30, 30}};
    // pts[0].x = 20;
    // pts[1].x = 60;  
    // pts[0].y = 20;
    // pts[1].y = 50;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    // line_dsc.color = lv_color_hex3(0x999);
    //line_dsc.width = 3;
    //line_dsc.opa = LV_OPA_30;
    
    lv_canvas_draw_line(canvas, pts, 2, &line_dsc);

    
    lv_draw_label_dsc_t lbl_dsc;
    lv_draw_label_dsc_init(&lbl_dsc);

    lv_canvas_draw_text(canvas, 2, 2, 30, &lbl_dsc, "90");


    //lv_obj_center(canvas);
    lv_img_set_angle(canvas, angle);
    lv_obj_align(canvas, LV_ALIGN_CENTER, x, y);
    


    //lv_img_dsc_t *img = lv_canvas_get_img(canvas);
    //lv_canvas_transform(canvas, &img, 450, LV_IMG_ZOOM_NONE, 10, 10, 32 / 2, 32 / 2, true);
    //lv_draw_label(&layer, &dsc, &coords);

    //lv_canvas_finish_layer(canvas, &layer);
    //LOG_INF("lbl coords %d, %d, %d, %d", label_coords.x1, label_coords.x2, label_coords.y1, label_coords.y2);
}*/

static void lv_pitch_ladder_draw_numeric1( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    //lv_obj_t *canvas_num = lv_canvas_create(obj);
    lv_obj_t *canvas = lv_canvas_create(obj);
    lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 18)];
    lv_canvas_set_buffer(canvas, buffer,32, 18, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_img_set_angle(canvas, angle);
    lv_obj_align(canvas, LV_ALIGN_CENTER, x, y);

    lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    lv_canvas_fill_bg(canvas, c, LV_OPA_COVER); 

    lv_draw_label_dsc_t lbl_dsc;
    lv_draw_label_dsc_init(&lbl_dsc);

    lv_canvas_draw_text(canvas, 2, 2, 30, &lbl_dsc, "-90");
}

static void lv_pitch_ladder_draw_numeric( int16_t x, int16_t y, int16_t angle)
{
    // lv_obj_t *canvas_num = lv_canvas_create(obj);
    // lv_obj_t *canvas = lv_canvas_create(obj);
    // lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(32, 18)];
    // lv_canvas_set_buffer(canvas, buffer,32, 18, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_img_set_angle(canvasNum, angle);
    lv_obj_align(canvasNum, LV_ALIGN_CENTER, x, y);

    lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    lv_canvas_fill_bg(canvasNum, c, LV_OPA_COVER); 

    lv_draw_label_dsc_t lbl_dsc;
    lv_draw_label_dsc_init(&lbl_dsc);

    lv_canvas_draw_text(canvasNum, 2, 2, 30, &lbl_dsc, "-90");
}

static void lv_pitch_ladder_draw_horizon( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    lv_obj_t *canvas = lv_canvas_create(obj);
    lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(96, 2)];
    lv_canvas_set_buffer(canvas, buffer,96, 2, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_img_set_angle(canvas, angle);
    lv_obj_align(canvas, LV_ALIGN_CENTER, x, y);

    lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    lv_canvas_fill_bg(canvas, c, LV_OPA_COVER);

    //const lv_point_t pts[] = { {0, 2}, {95, 2}};
    const lv_point_t pts[] = { {0, 1}, {40, 1}};
    const lv_point_t pts1[] = { {56, 1}, {95, 1}};

    lv_draw_line_dsc_t line_dsc;
    line_dsc.width = 2;
    lv_draw_line_dsc_init(&line_dsc);
    lv_canvas_draw_line(canvas, pts, 2, &line_dsc);
    lv_canvas_draw_line(canvas, pts1, 2, &line_dsc);

    // lv_point_t tick_point_a;
    // lv_point_t tick_point_b;
    // tick_point_a.x = 0;
    // tick_point_a.y = 0;
    // tick_point_b.x = 95;
    // tick_point_b.y = 0;


    // lv_draw_line_dsc_t niddle_dsc;
    // lv_draw_line_dsc_init(&niddle_dsc);
    // //lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &niddle_dsc);

    // lv_draw_line(canvas, &niddle_dsc, &tick_point_a, &tick_point_b);

}
static void lv_pitch_ladder_draw_ladder_up( lv_obj_t * obj, int16_t x, int16_t y, int16_t angle)
{
    lv_obj_t *canvas = lv_canvas_create(obj);
    lv_color_t buffer[LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(64, 4)];
    lv_canvas_set_buffer(canvas, buffer,64, 4, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_img_set_angle(canvas, angle);
    lv_obj_align(canvas, LV_ALIGN_CENTER, x, y);

    lv_color_t c = lv_color_make(0xFF, 0xFF, 0xFF);
    lv_canvas_fill_bg(canvas, c, LV_OPA_COVER);

    const lv_point_t pts[] = { {0, 2}, {23, 2}};
    const lv_point_t pts1[] = { {40, 2}, {63, 2}};

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    
    lv_canvas_draw_line(canvas, pts, 2, &line_dsc);
    lv_canvas_draw_line(canvas, pts1, 2, &line_dsc);


}

void lv_line_transform(lv_point_t * a, lv_point_t * b, int32_t angle, const lv_point_t * pivot)
{
    lv_point_transform(a, angle, 256, pivot);
    lv_point_transform(b, angle, 256, pivot);
}

static void lv_pitch_ladder_refresh_ind( lv_obj_t * obj, lv_event_t * event)
{
    lv_draw_ctx_t *canvas = lv_event_get_draw_ctx(event);

    lv_draw_line_dsc_t major_tick_dsc;
    lv_draw_line_dsc_init(&major_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);

    lv_point_t tick_point_a;
    lv_point_t tick_point_b;
    tick_point_a.x = 20;
    tick_point_a.y = 20;
    tick_point_b.x = 60;
    tick_point_b.y = 20;

    lv_point_t center_point;
    center_point.x = 40;
    center_point.y = 20;

    lv_line_transform(&tick_point_a, &tick_point_b, 300, &center_point);

    lv_draw_line((struct _lv_draw_ctx_t *)canvas, &major_tick_dsc, &tick_point_a, &tick_point_b);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *) obj;
    lv_pitch_ladder_draw_numeric1(obj, 0,0, pitch_ladder->roll_angle);
    //lv_pitch_ladder_draw_numeric1(obj, 0,0,0);
    //lv_pitch_ladder_draw_numeric(0,0,pitch_ladder->roll_angle);
    // lv_draw_label_dsc_t lbl_dsc;
    // lbl_dsc.color = lv_color_black();
    // lv_draw_label_dsc_init(&lbl_dsc);
    // lv_area_t label_coords;
    // label_coords.x1 = 30;
    // label_coords.y1 = 30;
    // label_coords.x2 = 50;
    // label_coords.y2 = 50;

    // char text_buffer[20] = {0};
    // lv_snprintf(text_buffer, sizeof(text_buffer), "%" LV_PRId32, 10);

    // lv_draw_label((struct _lv_draw_ctx_t *)canvas, &lbl_dsc, &label_coords, "XX", NULL);

    //lv_pitch_ladder_draw_numeric2(obj, 0, 0, 450);

    //lv_pitch_ladder_draw_numeric(0, 0, 0);
    //lv_pitch_ladder_draw_numeric1(obj, -40, -40, 450);
    
    //lv_img_set_angle(canvas, 450);
    
    // lv_obj_t * lbl_img = lv_img_create(obj);
    // lv_img_set_src(lbl_img, LV_SYMBOL_DUMMY "X");
    /*
    //lv_obj_t * canvas = lv_canvas_create(obj);
    //lv_canvas_set_buffer(canvas,&test,100, 50, LV_IMG_CF_TRUE_COLOR);

    const lv_point_t pts[] = { {5, 5}, {70, 70}};

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);  
    lv_canvas_draw_line((struct _lv_draw_ctx_t *)canvas, pts, 2, &line_dsc);*/

    //lv_img_set_angle((lv_obj_t *)lv_img_get_src((lv_obj_t *)canvas), 300);
    //lv_img_set_angle(lbl_img, 300);
    

}
void lv_example_img_1(lv_obj_t * obj)
{

    lv_obj_t * img2 = lv_img_create(obj);
    lv_img_set_src(img2, LV_SYMBOL_DUMMY "Accept");
    //lv_obj_align_to(img2, img1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
}
static void lv_pitch_ladder_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
   
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    _lv_ll_init(&pitch_ladder->section_ll, sizeof(lv_pitch_ladder_section_t));

    pitch_ladder->total_tick_count = LV_PITCH_LADDER_TOTAL_TICK_COUNT_DEFAULT;
    pitch_ladder->major_tick_every = LV_PITCH_LADDER_MAJOR_TICK_EVERY_DEFAULT;
    pitch_ladder->mode = LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM;
    pitch_ladder->label_enabled = LV_PITCH_LADDER_LABEL_ENABLED_DEFAULT;
    pitch_ladder->angle_range = LV_PITCH_LADDER_DEFAULT_ANGLE_RANGE;
    pitch_ladder->rotation = LV_PITCH_LADDER_DEFAULT_ROTATION;
    pitch_ladder->range_min = 0U;
    pitch_ladder->range_max = 100U;
    pitch_ladder->last_tick_width = 0U;
    pitch_ladder->first_tick_width = 0U;
    pitch_ladder->post_draw = false;
    pitch_ladder->draw_ticks_on_top = false;
    pitch_ladder->custom_label_cnt = 0U;
    pitch_ladder->txt_src = NULL;
    pitch_ladder->widget_draw = false;


    lv_obj_t *canvasNum = lv_canvas_create(obj);
    lv_canvas_set_buffer(canvasNum, bufferNum,32, 18, LV_IMG_CF_TRUE_COLOR_ALPHA);


    // lv_pitch_ladder_draw_numeric(obj, -40, -40, 450);
    // lv_pitch_ladder_draw_ladder_up(obj, 0, -32, 0);
    // lv_pitch_ladder_draw_horizon(obj, 0, 0, 0);
    // lv_pitch_ladder_draw_ladder_up(obj, 0, 32, 0);
    //lv_pitch_ladder_draw_numeric(obj, -40, 40, 0);

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

static void lv_pitch_ladder_event(const lv_obj_class_t * class_p, lv_event_t * event)
{
    LV_UNUSED(class_p);

    //Call the ancestor's event handler
    uint8_t  res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t event_code = lv_event_get_code(event);
    lv_obj_t * obj = lv_event_get_current_target(event);
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *) obj;
    LV_UNUSED(pitch_ladder);
    //lv_obj_t * target = lv_event_get_target(event);
    

    if(event_code == LV_EVENT_DRAW_MAIN) {
        if(pitch_ladder->widget_draw){
            
            LOG_INF("redraw");
            pitch_ladder->widget_draw = false;
        }else{
            //LOG_INF("LV_EVENT_DRAW_MAIN");
        }
        lv_pitch_ladder_refresh_ind(obj, event);
       LOG_INF("LV_EVENT_DRAW_MAIN");
        //if(lv_obj_has_flag(event->current_target, LV_OBJ_FLAG_EVENT_BUBBLE)) LOG_INF("BUBBLED"); 
        //LOG_INF("LV_EVENT_DRAW_MAIN" );
        //LOG_INF("LV_EVENT_DRAW_MAIN  %d", lv_event_get_draw_ctx(event));
        // if(lv_obj_check_type(obj, MY_CLASS)) {
        //     LOG_INF("PITCH");
        // }else{
        //     LOG_INF("OTHER");
        // }
        //if(lv_obj_check_type(obj, &lv_pitch_ladder_t)) LOG_INF("pitch ladder");
        // if(pitch_ladder->post_draw == false) {
        //     //if(pitch_ladder->draw_ticks_on_top) {
        //         //lv_pitch_ladder_refresh_ind(obj, event);
        //         LOG_INF("draw");

        //     }
        //     else {

        //     }
        
        //lv_pitch_ladder_draw_indicator(obj, event);
        //lv_img_buf_free(&test);
        //lv_image_cache_drop(test);
        // lv_img_set_src(panel1, NULL);
        // lv_img_set_src(panel1, &test);
        //LOG_INF("main event");
        //pitch_ladder_draw_indicator(obj, event);
        
        /*if(pitch_ladder->post_draw == false) {
            if(pitch_ladder->draw_ticks_on_top) {
                pitch_ladder_draw_indicator(obj, event);
                //pitch_ladder_draw_needle(obj, event);
            }
            else {
                //pitch_ladder_draw_indicator(obj, event);
                //pitch_ladder_draw_needle(obj, event);
            }
        }*/
    }
    if(event_code == LV_EVENT_DRAW_POST) {
        //lv_pitch_ladder_refresh_ind(obj, event);
        //LOG_INF("LV_EVENT_DRAW_POST");
        if(pitch_ladder->widget_draw){
            //LOG_INF("event post");
            // lv_pitch_ladder_refresh_ind(obj, event);
            //lv_pitch_ladder_draw_numeric2(obj, -40, -40, pitch_ladder->roll_angle);
            //pitch_ladder->widget_draw = false;
        }
        /*if(pitch_ladder->post_draw == true) {
            
            if(pitch_ladder->draw_ticks_on_top) {
                //pitch_ladder_draw_indicator(obj, event);
                //pitch_ladder_draw_needle(obj, event);
            }
            else {
                //pitch_ladder_draw_indicator(obj, event);
                //pitch_ladder_draw_needle(obj, event);
            }
        }*/
    }
    else if(event_code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        //LOG_INF("EVENT CODE: %d",event_code);
        // NOTE: Extend pitch_ladder draw size so the first tick label can be shown
        lv_event_set_ext_draw_size(event, 100);
    }
    else {
        //LOG_INF("LV_EVENT_INVALID");
        // Nothing to do. Invalid event 
    }
}

static void pitch_ladder_draw_indicator(lv_obj_t * obj, lv_event_t * event)
{
    static int16_t rotimg = 20;
    static lv_coord_t xpiv =0;
    static lv_coord_t ypiv =0;
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;


    //lv_pitch_ladder_img_combine(obj);
    //lv_draw_layer_ctx_t * layer = lv_event_get_draw_ctx(event);
    // lv_draw_ctx_t *layer = lv_event_get_draw_ctx(event);


    // if(pitch_ladder->total_tick_count <= 1) return;

    // lv_draw_label_dsc_t label_dsc;
    // lv_draw_label_dsc_init(&label_dsc);
    // // Formatting the labels with the configured style for LV_PART_INDICATOR 
    // lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);

    // // The tick is represented by a line. We need two points to draw it 
    // lv_point_t tick_point_a;
    // lv_point_t tick_point_b;
    // int32_t fine_tick_idx = (pitch_ladder->dir_angle % 10) - 0;
    // pitch_ladder_get_tick_points_offset(obj, 0, 0, 0, &tick_point_a, &tick_point_b);

    // Setup a label if they're enabled and we're drawing a major tick 
    /*if(pitch_ladder->label_enabled) {
        pitch_ladder_draw_label(obj, event, &label_dsc, 0, pitch_ladder->pitch_angle, &tick_point_a, 0, true);
    }

    lv_draw_line_dsc_t horizon_dsc;
    lv_draw_line_dsc_init(&horizon_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &horizon_dsc);*/

    //lv_obj_set_style_translate_x(obj, 10, 0);
    //lv_obj_set_style_translate_y(obj, 10, 0);
    //lv_obj_set_style_transform_angle(obj, 5, 0);
    //lv_obj_set_style_transform_pivot_y(obj, 45, 0);
    //lv_obj_set_style_transform_rotation(obj, 150, 0); 
    //lv_obj_set_style_transform_angle(layer, 0, 0);

    //lv_draw_line(layer, &horizon_dsc, &tick_point_a, &tick_point_b);

    // lv_point_t pivot_img;
    // lv_img_get_pivot(panel1, &pivot_img);
    // LOG_INF("pivot %d, %d", pivot_img.x, pivot_img.y);



    //rotimg += 10;
    //lv_img_set_offset_y(panel1, yoffs);
    // lv_img_set_pivot(panel1, xpiv, ypiv);
    // lv_img_set_angle(panel1,rotimg);
    // lv_img_set_antialias(panel1, true);
    // //rotimg = -1*rotimg;
    // rotimg = rotimg + 10;
    // if (rotimg>=3600) rotimg = -3600;
    // xpiv += 1;
    // ypiv += 1;
    // if (xpiv>=128) xpiv = 128;
    // if (ypiv>=128) xpiv = 128;
    //lv_obj_t * panel1 = lv_obj_create(obj);

    // lv_obj_t * panel1 = lv_img_create(obj);
    // LV_IMG_DECLARE(test);
    // lv_img_set_src(panel1, &test);
    // lv_obj_align(panel1, LV_ALIGN_CENTER, 0, 0);

    /*Test the rotation. It requires another buffer where the original image is stored.
     *So use previous canvas as image and rotate it to the new canvas*/
    // #define CANVAS_WIDTH 120
    // #define CANVAS_HEIGHT 120
    //LV_DRAW_BUF_DEFINE_STATIC(draw_buf_32bpp, CANVAS_WIDTH, CANVAS_HEIGHT, LV_COLOR_FORMAT_ARGB8888);
    //LV_DRAW_BUF_INIT_STATIC(draw_buf_32bpp);

    /*Create a canvas and initialize its palette*/
    //lv_obj_t * canvas  = lv_canvas_create(lv_screen_active());
    //lv_canvas_set_draw_buf(canvas, &draw_buf_32bpp);
    // lv_canvas_fill_bg(canvas, lv_color_hex3(0xccc), LV_OPA_COVER);
    // lv_obj_center(canvas);

    //lv_canvas_fill_bg(canvas, lv_palette_lighten(LV_PALETTE_GREY, 1), LV_OPA_COVER);

    // lv_canvas_init_layer(canvas, &layer);
    // lv_img_dsc_t img;
    // //lv_draw_buf_to_image(&draw_buf_16bpp, &img);
    // lv_draw_img_dsc_t img_dsc;
    // lv_draw_image_dsc_init(&img_dsc);
    // img_dsc.rotation = 120;
    // img_dsc.src = &img;
    // img_dsc.pivot.x = CANVAS_WIDTH / 2;
    // img_dsc.pivot.y = CANVAS_HEIGHT / 2;

    // lv_area_t coords_img = {0, 0, CANVAS_WIDTH - 1, CANVAS_HEIGHT - 1};
    // lv_draw_image(&layer, &img_dsc, &coords_img);

    // lv_canvas_finish_layer(canvas, &layer);


/*
    // Major tick style 
    lv_draw_line_dsc_t major_tick_dsc;
    lv_draw_line_dsc_init(&major_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);
    if(LV_PITCH_LADDER_MODE_ROUND_OUTER == pitch_ladder->mode || LV_PITCH_LADDER_MODE_ROUND_INNER == pitch_ladder->mode) {
        major_tick_dsc.raw_end = 0;
    }

    // Configure line draw descriptor for the minor tick drawing 
    lv_draw_line_dsc_t minor_tick_dsc;
    lv_draw_line_dsc_init(&minor_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &minor_tick_dsc);

    // Main line style 
    lv_draw_line_dsc_t main_line_dsc;
    lv_draw_line_dsc_init(&main_line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &main_line_dsc);

    const uint32_t total_tick_count = pitch_ladder->total_tick_count;
    uint32_t tick_idx = 0;
    uint32_t major_tick_idx = 0;

    int32_t tick_value = pitch_ladder->range_min;
    for(tick_idx = 0; tick_idx < total_tick_count; tick_idx++) {
        // A major tick is the one which has a label in it 
        uint8_t tick_type = 0;
        if(tick_idx % pitch_ladder->major_tick_every == 0) tick_type = 1;
        if(tick_type == 1) major_tick_idx++;

        tick_value = lv_pitch_ladder_limit(tick_value);

        // The tick is represented by a line. We need two points to draw it 
        lv_point_t tick_point_a;
        lv_point_t tick_point_b;
        int32_t fine_tick_idx = (pitch_ladder->dir_angle % 10) - 0;
        pitch_ladder_get_tick_points_offset(obj, tick_idx, fine_tick_idx, tick_type, &tick_point_a, &tick_point_b);

        // Setup a label if they're enabled and we're drawing a major tick 
        if(pitch_ladder->label_enabled && tick_type == 1) {
            pitch_ladder_draw_label(obj, event, &label_dsc, major_tick_idx, tick_value, &tick_point_a, tick_idx, true);
        }

        if(tick_type == 1) {
            lv_draw_line(layer, &major_tick_dsc, &tick_point_a, &tick_point_b);
        }
        else {
            lv_draw_line(layer, &minor_tick_dsc, &tick_point_a, &tick_point_b);
        }
        tick_value += 5;
    }*/
}

static void pitch_ladder_draw_needle(lv_obj_t * obj, lv_event_t * event)
{
/*    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_draw_layer_ctx_t * layer = (lv_draw_layer_ctx_t *) lv_event_get_draw_ctx(event);

    if(pitch_ladder->total_tick_count <= 1) return;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    // Formatting the labels with the configured style for LV_PART_INDICATOR 
    lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);

    // Major tick style 
    lv_draw_line_dsc_t niddle_dsc;
    lv_draw_line_dsc_init(&niddle_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &niddle_dsc);
    if(LV_PITCH_LADDER_MODE_ROUND_OUTER == pitch_ladder->mode || LV_PITCH_LADDER_MODE_ROUND_INNER == pitch_ladder->mode) {
        niddle_dsc.raw_end = 0;
    }
    uint32_t tick_idx = pitch_ladder->total_tick_count/2;

    int32_t tick_value = pitch_ladder->dir_angle;

    // The tick is represented by a line. We need two points to draw it 
    lv_point_t tick_point_a;
    lv_point_t tick_point_b;

    pitch_ladder_get_tick_points_offset(obj, tick_idx, 0, 2 , &tick_point_a, &tick_point_b);

    // Setup a label if they're enabled and we're drawing a major tick 
    if(pitch_ladder->label_enabled ) {
        pitch_ladder_draw_label(obj, event, &label_dsc, 0, tick_value, &tick_point_b, tick_idx, false);
    }

    lv_draw_line((struct _lv_draw_ctx_t *)layer, &niddle_dsc, &tick_point_a, &tick_point_b);
*/
}

static void pitch_ladder_draw_label(lv_obj_t * obj, lv_event_t * event, lv_draw_label_dsc_t * label_dsc,
                             const uint32_t major_tick_idx, const int32_t tick_value, lv_point_t * tick_point_b,
                             const uint32_t tick_idx, bool above)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    lv_draw_layer_ctx_t * layer = (lv_draw_layer_ctx_t *) lv_event_get_draw_ctx(event);

    // Label text setup 
    char text_buffer[LV_PITCH_LADDER_LABEL_TXT_LEN] = {0};
    lv_area_t label_coords;

    // Check if the custom text array has element for this major tick index 
    if(pitch_ladder->txt_src) {
        pitch_ladder_build_custom_label_text(obj, label_dsc, text_buffer, major_tick_idx);
    }
    else { // Add label with mapped values
        lv_snprintf(text_buffer, sizeof(text_buffer), "%" LV_PRId32, tick_value);
    }

    if((LV_PITCH_LADDER_MODE_VERTICAL_LEFT == pitch_ladder->mode || LV_PITCH_LADDER_MODE_VERTICAL_RIGHT == pitch_ladder->mode)
       || (LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM == pitch_ladder->mode || LV_PITCH_LADDER_MODE_HORIZONTAL_TOP == pitch_ladder->mode)) {
        pitch_ladder_get_label_coords(obj, label_dsc,text_buffer, tick_point_b, &label_coords, above);
    }
    else if(LV_PITCH_LADDER_MODE_ROUND_OUTER == pitch_ladder->mode || LV_PITCH_LADDER_MODE_ROUND_INNER == pitch_ladder->mode) {
        lv_area_t pitch_ladder_area;
        lv_obj_get_content_coords(obj, &pitch_ladder_area);

        // Find the center of the pitch_ladder 
        lv_point_t center_point;
        int32_t radius_edge = LV_MIN(lv_area_get_width(&pitch_ladder_area) / 2U, lv_area_get_height(&pitch_ladder_area) / 2U);
        center_point.x = pitch_ladder_area.x1 + radius_edge;
        center_point.y = pitch_ladder_area.y1 + radius_edge;

        const int32_t major_len = 5;
        uint32_t label_gap = LV_PITCH_LADDER_DEFAULT_LABEL_GAP; 

        // Also take into consideration the letter space of the style
        int32_t angle_uppitch_ladder = ((tick_idx * pitch_ladder->angle_range) * 10U) / (pitch_ladder->total_tick_count - 1);
        angle_uppitch_ladder += pitch_ladder->rotation * 10U;

        uint32_t radius_text = 0;
        if(LV_PITCH_LADDER_MODE_ROUND_INNER == pitch_ladder->mode) {
            radius_text = (radius_edge - major_len) - (label_gap + label_dsc->letter_space);
        }
        else if(LV_PITCH_LADDER_MODE_ROUND_OUTER == pitch_ladder->mode) {
            radius_text = (radius_edge + major_len) + (label_gap + label_dsc->letter_space);
        }
        else {  }

        lv_point_t point;
        point.x = center_point.x + radius_text;
        point.y = center_point.y;
        lv_point_transform(&point, angle_uppitch_ladder, LV_PITCH_LADDER_NONE, &center_point);
        pitch_ladder_get_label_coords(obj, label_dsc,text_buffer, &point, &label_coords, above);
    }
    // Invalid mode 
    else {
        return;
    }
    //lv_draw_label_hint_t * hint; //??
    lv_draw_label((struct _lv_draw_ctx_t *)layer, label_dsc, &label_coords, text_buffer, NULL);
}

/**
 * Get points for ticks
 *
 * In order to draw ticks we need two points, this interface returns both points for all pitch_ladder modes.
 *
 * @param obj       pointer to a pitch_ladder object
 * @param tick_idx  index of the current tick
 * @param is_major_tick true if tick_idx is a major tick
 * @param tick_point_a  pointer to point 'a' of the tick
 * @param tick_point_b  pointer to point 'b' of the tick
 */

static void pitch_ladder_get_tick_points_offset(lv_obj_t * obj, uint32_t tick_idx, int32_t fine_tick_val, int8_t tick_type,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b)
{

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;

    /* Main line style */
    lv_draw_line_dsc_t main_line_dsc;
    lv_draw_line_dsc_init(&main_line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &main_line_dsc);

    int32_t minor_len = 0;
    int32_t major_len = 0;
    int32_t horizon_len = 20;

    int32_t fine_tick_range = 5;
    //int32_t fine_tick_val = 0;
    tick_idx =  - fine_tick_val + (tick_idx * fine_tick_range);

    // if(tick_type == 1) {
    //     major_len = 10;//lv_obj_get_style_length(obj, LV_PART_INDICATOR);
    // }
    // if(tick_type == 0)  {
    //     minor_len = 5;//lv_obj_get_style_length(obj, LV_PART_ITEMS);
    // }
    // if(tick_type == 2)  {
    //     major_len = 10;
    // }

    /* Get style properties so they can be used in the tick and label drawing */
    const int32_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
    const int32_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
    const int32_t pad_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN) + border_width;
    const int32_t pad_right = lv_obj_get_style_pad_right(obj, LV_PART_MAIN) + border_width;
    const int32_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
    const int32_t tick_pad_right = lv_obj_get_style_pad_right(obj, LV_PART_ITEMS);
    const int32_t tick_pad_left = lv_obj_get_style_pad_left(obj, LV_PART_ITEMS);
    const int32_t tick_pad_top = lv_obj_get_style_pad_top(obj, LV_PART_ITEMS);
    const int32_t tick_pad_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_ITEMS);

    int32_t x_ofs = 0U;
    int32_t y_ofs = 0U;
    int32_t y_width = obj->coords.y2 - obj->coords.y1;
    int32_t y_mid = obj->coords.y1 + y_width/2 + (pad_top + tick_pad_top);
    int32_t x_width = obj->coords.x2 - obj->coords.x1;
    int32_t x_mid = obj->coords.x1 + x_width/2;
    x_ofs = obj->coords.x1;
    y_ofs = y_mid;

    // x_ofs = obj->coords.x2 + (main_line_dsc.width / 2U) - pad_right;
    // y_ofs = obj->coords.y1 + (pad_top + tick_pad_top);

    // major_len *= -1;
    // minor_len *= -1;
    // major_len *= -1;

    //int32_t tick_length = major_len;

    // if(tick_type == 1)  {
    //         tick_length = major_len;
    //     }
    // if(tick_type == 0)  {
    //         tick_length = minor_len;
    //     }
    // if(tick_type == 2)  {
    //         tick_length = major_len;
    //     }
    /* NOTE
        * Minus 1 because tick count starts at 0
        * TODO
        * What if total_tick_count is 1? This will lead to an division by 0 further down */
    const uint32_t tmp_tick_count = (pitch_ladder->total_tick_count - 1U) * fine_tick_range;

    /* Calculate the position of the tick points based on the mode and tick index */
    /* Vertical position starts at y2 of the pitch_ladder main line, we start at y2 because the ticks are drawn from bottom to top */
    int32_t vertical_position = obj->coords.y2 - (pad_bottom + tick_pad_bottom);

    /* Position the last tick */
    // if(tmp_tick_count == tick_idx) {
    //     vertical_position = y_ofs;
    // }
    /* Otherwise adjust the tick position depending of its index and number of ticks on the pitch_ladder */
    // else if(0 != tick_idx) {
    //     const int32_t pitch_ladder_total_height = lv_obj_get_height(obj) - (pad_top + pad_bottom + tick_pad_top + tick_pad_bottom);
    //     const int32_t offset = ((int32_t) tick_idx * (int32_t) pitch_ladder_total_height) / (int32_t)(tmp_tick_count);
    //     vertical_position -= offset;
    // }
    // else { /* Nothing to do */ }

    tick_point_a->x = x_ofs + 0U; /* Move extra pixel out of pitch_ladder boundary */
    tick_point_a->y = y_ofs;
    tick_point_b->x = tick_point_a->x + x_width;
    tick_point_b->y = tick_point_a->y;
    // LOG_INF("pitch offset %d, %d, %d, %d", x_ofs, y_ofs, main_line_dsc.width, vertical_position);
    // LOG_INF("pitch coord %d, %d, %d, %d", obj->coords.x1, obj->coords.x2, obj->coords.y1, obj->coords.y2);
    // LOG_INF("pitch width %d", x_width);
}

/**
 * Get coordinates for label
 *
 * @param obj       pointer to a pitch_ladder object
 * @param label_dsc pointer to label descriptor
 * @param tick_point    pointer to reference tick
 * @param label_coords  pointer to label coordinates output
 */

static void pitch_ladder_get_label_coords(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc,const char * text, lv_point_t * tick_point,
                                   lv_area_t * label_coords, bool above)
{

    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *)obj;
    
    /* Reserve appropriate size for the tick label */
    lv_point_t label_size;
    lv_txt_get_size(&label_size, text,
                     label_dsc->font, label_dsc->letter_space, label_dsc->line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    //tick_point->y = tick_point->y - label_size.y -10;
    const uint8_t offs_y = above ? label_size.y : 0;
    /* Set the label draw area at some distance of the major tick */
    if((LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM == pitch_ladder->mode) || (LV_PITCH_LADDER_MODE_HORIZONTAL_TOP == pitch_ladder->mode)) {
        label_coords->x1 = tick_point->x - (label_size.x / 2U);
        label_coords->x2 = tick_point->x + (label_size.x / 2U);

        if(LV_PITCH_LADDER_MODE_HORIZONTAL_BOTTOM == pitch_ladder->mode) {
            label_coords->y1 = tick_point->y + lv_obj_get_style_pad_bottom(obj, LV_PART_INDICATOR) - offs_y;
            label_coords->y2 = label_coords->y1 + label_size.y;
        }
        else {
            label_coords->y2 = tick_point->y - lv_obj_get_style_pad_top(obj, LV_PART_INDICATOR) - offs_y;
            label_coords->y1 = label_coords->y2 - label_size.y;
        }
    }
    else if((LV_PITCH_LADDER_MODE_VERTICAL_LEFT == pitch_ladder->mode) || (LV_PITCH_LADDER_MODE_VERTICAL_RIGHT == pitch_ladder->mode)) {
        label_coords->y1 = tick_point->y - (label_size.y / 2U) - offs_y;
        label_coords->y2 = tick_point->y + (label_size.y / 2U) - offs_y;

        if(LV_PITCH_LADDER_MODE_VERTICAL_LEFT == pitch_ladder->mode) {
            label_coords->x1 = tick_point->x - label_size.x - lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR);
            label_coords->x2 = tick_point->x - lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR);
        }
        else {
            label_coords->x1 = tick_point->x + lv_obj_get_style_pad_right(obj, LV_PART_INDICATOR);
            label_coords->x2 = tick_point->x + label_size.x + lv_obj_get_style_pad_right(obj, LV_PART_INDICATOR);
        }
    }
    else if(LV_PITCH_LADDER_MODE_ROUND_OUTER == pitch_ladder->mode || LV_PITCH_LADDER_MODE_ROUND_INNER == pitch_ladder->mode) {
        label_coords->x1 = tick_point->x - (label_size.x / 2U);
        label_coords->y1 = tick_point->y - (label_size.y / 2U) - offs_y;
        label_coords->x2 = label_coords->x1 + label_size.x;
        label_coords->y2 = label_coords->y1 + label_size.y;
    }
    else { /* Nothing to do */ }

}

/**
 * Sets the text of the tick label descriptor when using custom labels
 *
 * Sets the text pointer when valid custom label is available, otherwise set it to NULL.
 *
 * @param obj       pointer to a pitch_ladder object
 * @param label_dsc pointer to the label descriptor
 * @param major_tick_idx  index of the current major tick
 */

static void pitch_ladder_build_custom_label_text(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text,
                                          const uint16_t major_tick_idx)
{
    lv_pitch_ladder_t * pitch_ladder = (lv_pitch_ladder_t *) obj;

    /* Check if the pitch_ladder has valid custom labels available,
     * this avoids reading past txt_src array when the pitch_ladder requires more tick labels than available */
    if(major_tick_idx <= pitch_ladder->custom_label_cnt) {
        if(pitch_ladder->txt_src[major_tick_idx - 1U]) {
            text = pitch_ladder->txt_src[major_tick_idx - 1U];
        }
        else {
            text = NULL;
        }
    }
    else {
        text = NULL;
    }
}
