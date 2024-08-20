/**
 * @file lv_compass.c
 *
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

static void comapss_draw_main(lv_obj_t * obj, lv_event_t * event);
static void comapss_draw_indicator(lv_obj_t * obj, lv_event_t * event);
static void comapss_draw_needle(lv_obj_t * obj, lv_event_t * event);
static void comapss_draw_label(lv_obj_t * obj, lv_event_t * event, lv_draw_label_dsc_t * label_dsc,
                             const uint32_t major_tick_idx, const int32_t tick_value, lv_point_t * tick_point_b, const uint32_t tick_idx, bool above);
static void comapss_calculate_main_compensation(lv_obj_t * obj);

static void comapss_get_center(const lv_obj_t * obj, lv_point_t * center, int32_t * arc_r);
static void comapss_get_tick_points(lv_obj_t * obj, const uint32_t tick_idx, bool is_major_tick,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b);
static void comapss_get_tick_points_offset(lv_obj_t * obj, uint32_t tick_idx, int32_t fine_tick_val, int8_t tick_type,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b);
static void comapss_get_label_coords(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text, lv_point_t * tick_point,
                                   lv_area_t * label_coords, bool above);
static void comapss_set_indicator_label_properties(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc,
                                                 lv_style_t * indicator_section_style);
static void comapss_set_line_properties(lv_obj_t * obj, lv_draw_line_dsc_t * line_dsc, lv_style_t * section_style,
                                      lv_part_t part);
static void comapss_set_arc_properties(lv_obj_t * obj, lv_draw_arc_dsc_t * arc_dsc, lv_style_t * section_style);
/* Helpers */
static void comapss_find_section_tick_idx(lv_obj_t * obj);
static void comapss_store_main_line_tick_width_compensation(lv_obj_t * obj, const uint32_t tick_idx,
                                                          const bool is_major_tick, const int32_t major_tick_width, const int32_t minor_tick_width);
static void comapss_store_section_line_tick_width_compensation(lv_obj_t * obj, const bool is_major_tick,
                                                             lv_draw_line_dsc_t * major_tick_dsc, lv_draw_line_dsc_t * minor_tick_dsc,
                                                             const int32_t tick_value, const uint8_t tick_idx, lv_point_t * tick_point_a);
static void comapss_build_custom_label_text(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text,
                                          const uint16_t major_tick_idx);

static void comapss_free_line_needle_points_cb(lv_event_t * e);

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

lv_obj_t * lv_comapss_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
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

void lv_comapss_set_mode(lv_obj_t * obj, lv_comapss_mode_t mode)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->mode = mode;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_total_tick_count(lv_obj_t * obj, uint32_t total_tick_count)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->total_tick_count = total_tick_count;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_major_tick_every(lv_obj_t * obj, uint32_t major_tick_every)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->major_tick_every = major_tick_every;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_label_show(lv_obj_t * obj, bool show_label)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->label_enabled = show_label;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_range(lv_obj_t * obj, int32_t min, int32_t max)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    //comapss->range_min = min;
    //comapss->range_max = max;

    lv_obj_invalidate(obj);
}

int32_t lv_compass_limit(int32_t value)
{
    if(value < 0) value = value + 360;
    if(value > 359) value = value -360;
    return value;
}

void lv_comapss_set_direction_angle(lv_obj_t * obj, int32_t direction)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->dir_angle = direction;

    int scale = 10; // tick lenght
    int tickRange = 2;  // number of ticks
    int scaleStart = lv_compass_limit(floor(comapss->dir_angle/scale)*scale-floor(scale*tickRange/2));
    int scaleStop = lv_compass_limit(floor(comapss->dir_angle/scale)*scale+ floor(scale*tickRange/2));

    
    comapss->range_min = scaleStart;//direction - 15;
    comapss->range_max = scaleStop;//direction + 10;;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_angle_range(lv_obj_t * obj, uint32_t angle_range)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->angle_range = angle_range;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_rotation(lv_obj_t * obj, int32_t rotation)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->rotation = rotation;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_line_needle_value(lv_obj_t * obj, lv_obj_t * needle_line, int32_t needle_length,
                                    int32_t value)
{

    int32_t angle;
    int32_t comapss_width, comapss_height;
    int32_t actual_needle_length;
    int32_t needle_length_x, needle_length_y;
    lv_point_t * needle_line_points = NULL;

    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    if((comapss->mode != LV_COMAPSS_MODE_ROUND_INNER) &&
       (comapss->mode != LV_COMAPSS_MODE_ROUND_OUTER)) {
        return;
    }

    lv_obj_align(needle_line, LV_ALIGN_TOP_LEFT, 0, 0);

    comapss_width = lv_obj_get_style_width(obj, LV_PART_MAIN);
    comapss_height = lv_obj_get_style_height(obj, LV_PART_MAIN);

    if(comapss_width != comapss_height) {
        return;
    }

    if(needle_length >= comapss_width / 2) {
        actual_needle_length = comapss_width / 2;
    }
    else if(needle_length >= 0) {
        actual_needle_length = needle_length;
    }
    else if(needle_length + comapss_width / 2 < 0) {
        actual_needle_length = 0;
    }
    else {
        actual_needle_length = comapss_width / 2 + needle_length;
    }

    if(value < comapss->range_min) {
        angle = 0;
    }
    else if(value > comapss->range_max) {
        angle = comapss->angle_range;
    }
    else {
        angle = comapss->angle_range * (value - comapss->range_min) / (comapss->range_max - comapss->range_min);
    }

    needle_length_x = (actual_needle_length * lv_trigo_cos(comapss->rotation + angle)) >> LV_TRIGO_SHIFT;
    needle_length_y = (actual_needle_length * lv_trigo_sin(comapss->rotation + angle)) >> LV_TRIGO_SHIFT;

    if(lv_line_is_point_array_mutable(needle_line) && lv_line_get_point_count(needle_line) >= 2) {
        needle_line_points = lv_line_get_points_mutable(needle_line);
    }

    if(needle_line_points == NULL) {
        uint32_t i;
        uint32_t line_event_cnt = lv_obj_get_event_count(needle_line);
        for(i = 0; i < line_event_cnt; i--) {
            lv_event_dsc_t * dsc = lv_obj_get_event_dsc(needle_line, i);
            if(lv_event_dsc_get_cb(dsc) == comapss_free_line_needle_points_cb) {
                needle_line_points = lv_event_dsc_get_user_data(dsc);
                break;
            }
        }
    }

    if(needle_line_points == NULL) {
        needle_line_points = lv_malloc(sizeof(lv_point_t) * 2);
        LV_ASSERT_MALLOC(needle_line_points);
        if(needle_line_points == NULL) return;
        lv_obj_add_event_cb(needle_line, comapss_free_line_needle_points_cb, LV_EVENT_DELETE, needle_line_points);
    }

    needle_line_points[0].x = comapss_width / 2;
    needle_line_points[0].y = comapss_height / 2;
    needle_line_points[1].x = comapss_width / 2 + needle_length_x;
    needle_line_points[1].y = comapss_height / 2 + needle_length_y;

    lv_line_set_points_mutable(needle_line, needle_line_points, 2);
}

void lv_comapss_set_image_needle_value(lv_obj_t * obj, lv_obj_t * needle_img, int32_t value)
{
    int32_t angle;
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    if((comapss->mode != LV_COMAPSS_MODE_ROUND_INNER) &&
       (comapss->mode != LV_COMAPSS_MODE_ROUND_OUTER)) {
        return;
    }

    if(value < comapss->range_min) {
        angle = 0;
    }
    else if(value > comapss->range_max) {
        angle = comapss->angle_range;
    }
    else {
        angle = comapss->angle_range * (value - comapss->range_min) / (comapss->range_max - comapss->range_min);
    }

    //lv_image_set_rotation(needle_img, (comapss->rotation + angle) * 10);
}

void lv_comapss_set_text_src(lv_obj_t * obj, const char * txt_src[])
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->txt_src = txt_src;
    comapss->custom_label_cnt = 0;
    if(comapss->txt_src) {
        int32_t idx;
        for(idx = 0; txt_src[idx]; ++idx) {
            comapss->custom_label_cnt++;
        }
    }

    lv_obj_invalidate(obj);
}

void lv_comapss_set_post_draw(lv_obj_t * obj, bool en)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->post_draw = en;

    lv_obj_invalidate(obj);
}

void lv_comapss_set_draw_ticks_on_top(lv_obj_t * obj, bool en)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    comapss->draw_ticks_on_top = en;

    lv_obj_invalidate(obj);
}

lv_comapss_section_t * lv_comapss_add_section(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_comapss_section_t * section = _lv_ll_ins_head(&comapss->section_ll);
    LV_ASSERT_MALLOC(section);
    if(section == NULL) return NULL;

    /* Section default values */
    section->main_style = NULL;
    section->indicator_style = NULL;
    section->items_style = NULL;
    section->minor_range = 0U;
    section->major_range = 0U;
    section->first_tick_idx_in_section = LV_COMAPSS_TICK_IDX_DEFAULT_ID;
    section->last_tick_idx_in_section = LV_COMAPSS_TICK_IDX_DEFAULT_ID;
    section->first_tick_idx_is_major = 0U;
    section->last_tick_idx_is_major = 0U;
    section->first_tick_in_section_width = 0U;
    section->last_tick_in_section_width = 0U;

    return section;
}

void lv_comapss_section_set_range(lv_comapss_section_t * section, int32_t minor_range, int32_t major_range)
{
    if(NULL == section) return;

    section->minor_range = minor_range;
    section->major_range = major_range;
}

void lv_comapss_section_set_style(lv_comapss_section_t * section, lv_part_t part, lv_style_t * section_part_style)
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

/*=====================
 * Getter functions
 *====================*/

lv_comapss_mode_t lv_comapss_get_mode(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->mode;
}

int32_t lv_comapss_get_total_tick_count(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->total_tick_count;
}

int32_t lv_comapss_get_major_tick_every(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->major_tick_every;
}

bool lv_comapss_get_label_show(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->label_enabled;
}

uint32_t lv_comapss_get_angle_range(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->angle_range;
}

int32_t lv_comapss_get_range_min_value(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->range_min;
}

int32_t lv_comapss_get_range_max_value(lv_obj_t * obj)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    return comapss->range_max;
}

/*=====================
 * Other functions
 *====================*/

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_comapss_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
   
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    _lv_ll_init(&comapss->section_ll, sizeof(lv_comapss_section_t));

    comapss->total_tick_count = LV_COMAPSS_TOTAL_TICK_COUNT_DEFAULT;
    comapss->major_tick_every = LV_COMAPSS_MAJOR_TICK_EVERY_DEFAULT;
    comapss->mode = LV_COMAPSS_MODE_HORIZONTAL_BOTTOM;
    comapss->label_enabled = LV_COMAPSS_LABEL_ENABLED_DEFAULT;
    comapss->angle_range = LV_COMAPSS_DEFAULT_ANGLE_RANGE;
    comapss->rotation = LV_COMAPSS_DEFAULT_ROTATION;
    comapss->range_min = 0U;
    comapss->range_max = 100U;
    comapss->last_tick_width = 0U;
    comapss->first_tick_width = 0U;
    comapss->post_draw = false;
    comapss->draw_ticks_on_top = false;
    comapss->custom_label_cnt = 0U;
    comapss->txt_src = NULL;

    //lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);

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

static void lv_comapss_event(const lv_obj_class_t * class_p, lv_event_t * event)
{
    LV_UNUSED(class_p);

    /*Call the ancestor's event handler*/
    uint8_t  res = lv_obj_event_base(MY_CLASS, event);
    if(res != LV_RESULT_OK) return;

    lv_event_code_t event_code = lv_event_get_code(event);
    lv_obj_t * obj = lv_event_get_current_target(event);
    lv_comapss_t * comapss = (lv_comapss_t *) obj;
    LV_UNUSED(comapss);

    if(event_code == LV_EVENT_DRAW_MAIN) {
        if(comapss->post_draw == false) {
            //omapss_find_section_tick_idx(obj);
            //comapss_calculate_main_compensation(obj);

            if(comapss->draw_ticks_on_top) {
                //comapss_draw_main(obj, event);
                comapss_draw_indicator(obj, event);
                comapss_draw_needle(obj, event);
            }
            else {
                comapss_draw_indicator(obj, event);
                comapss_draw_needle(obj, event);
                //comapss_draw_main(obj, event);
            }
        }
    }
    if(event_code == LV_EVENT_DRAW_POST) {
        if(comapss->post_draw == true) {
            //comapss_find_section_tick_idx(obj);
            //comapss_calculate_main_compensation(obj);

            if(comapss->draw_ticks_on_top) {
                //comapss_draw_main(obj, event);
                comapss_draw_indicator(obj, event);
                comapss_draw_needle(obj, event);
            }
            else {
                comapss_draw_indicator(obj, event);
                comapss_draw_needle(obj, event);
                //comapss_draw_main(obj, event);
            }
        }
    }
    else if(event_code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        // NOTE: Extend comapss draw size so the first tick label can be shown
        lv_event_set_ext_draw_size(event, 100);
    }
    else {
        // Nothing to do. Invalid event 
    }
}

static void comapss_draw_indicator(lv_obj_t * obj, lv_event_t * event)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_draw_layer_ctx_t * layer = lv_event_get_draw_ctx(event);

    if(comapss->total_tick_count <= 1) return;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    // Formatting the labels with the configured style for LV_PART_INDICATOR 
    lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);

    // Major tick style 
    lv_draw_line_dsc_t major_tick_dsc;
    lv_draw_line_dsc_init(&major_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);
    if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
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

    const uint32_t total_tick_count = comapss->total_tick_count;
    uint32_t tick_idx = 0;
    uint32_t major_tick_idx = 0;

    int32_t tick_value = comapss->range_min;
    for(tick_idx = 0; tick_idx < total_tick_count; tick_idx++) {
        // A major tick is the one which has a label in it 
        uint8_t tick_type = 0;
        if(tick_idx % comapss->major_tick_every == 0) tick_type = 1;
        if(tick_type == 1) major_tick_idx++;

        //const int32_t tick_value = lv_map(tick_idx, 0U, total_tick_count - 1, comapss->range_min, comapss->range_max);
        tick_value = lv_compass_limit(tick_value);

        // Overwrite label and tick properties if tick value is within section range 
        /*lv_comapss_section_t * section;
        _LV_LL_READ_BACK(&comapss->section_ll, section) {
            if(section->minor_range <= tick_value && section->major_range >= tick_value) {
                if(is_major_tick) {
                    comapss_set_indicator_label_properties(obj, &label_dsc, section->indicator_style);
                    comapss_set_line_properties(obj, &major_tick_dsc, section->indicator_style, LV_PART_INDICATOR);
                }
                else {
                    comapss_set_line_properties(obj, &minor_tick_dsc, section->items_style, LV_PART_ITEMS);
                }
                break;
            }
            else {
                // Tick is not in section, get the proper styles 
                lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);
                lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);
                lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &minor_tick_dsc);
            }
        }*/

        // The tick is represented by a line. We need two points to draw it 
        lv_point_t tick_point_a;
        lv_point_t tick_point_b;
        int32_t fine_tick_idx = (comapss->dir_angle % 10) - 0;
        comapss_get_tick_points_offset(obj, tick_idx, fine_tick_idx, tick_type, &tick_point_a, &tick_point_b);

        // Setup a label if they're enabled and we're drawing a major tick 
        if(comapss->label_enabled && tick_type == 1) {
            comapss_draw_label(obj, event, &label_dsc, major_tick_idx, tick_value, &tick_point_a, tick_idx, true);
        }

        if(tick_type == 1) {
            //major_tick_dsc.p1 = lv_point_to_precise(&tick_point_a);
            //major_tick_dsc.p2 = lv_point_to_precise(&tick_point_b);
            lv_draw_line(layer, &major_tick_dsc, &tick_point_a, &tick_point_b);
        }
        else {
            //minor_tick_dsc.p1 = lv_point_to_precise(&tick_point_a);
            //minor_tick_dsc.p2 = lv_point_to_precise(&tick_point_b);
            //lv_draw_line(layer, &minor_tick_dsc);
            lv_draw_line(layer, &minor_tick_dsc, &tick_point_a, &tick_point_b);
        }
        tick_value += 5;
    }
}

static void comapss_draw_needle(lv_obj_t * obj, lv_event_t * event)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_draw_layer_ctx_t * layer = lv_event_get_draw_ctx(event);

    if(comapss->total_tick_count <= 1) return;

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    // Formatting the labels with the configured style for LV_PART_INDICATOR 
    lv_obj_init_draw_label_dsc(obj, LV_PART_INDICATOR, &label_dsc);

    // Major tick style 
    lv_draw_line_dsc_t niddle_dsc;
    lv_draw_line_dsc_init(&niddle_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &niddle_dsc);
    if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        niddle_dsc.raw_end = 0;
    }

    const uint32_t total_tick_count = comapss->total_tick_count;
    uint32_t tick_idx = comapss->total_tick_count/2;
    //uint32_t major_tick_idx = 0;

    int32_t tick_value = comapss->dir_angle;

    // The tick is represented by a line. We need two points to draw it 
    lv_point_t tick_point_a;
    lv_point_t tick_point_b;

    comapss_get_tick_points_offset(obj, tick_idx, 0, 2 , &tick_point_a, &tick_point_b);

    // Setup a label if they're enabled and we're drawing a major tick 
    if(comapss->label_enabled ) {
        comapss_draw_label(obj, event, &label_dsc, 0, tick_value, &tick_point_b, tick_idx, false);
    }

    lv_draw_line(layer, &niddle_dsc, &tick_point_a, &tick_point_b);

}

static void comapss_draw_label(lv_obj_t * obj, lv_event_t * event, lv_draw_label_dsc_t * label_dsc,
                             const uint32_t major_tick_idx, const int32_t tick_value, lv_point_t * tick_point_b,
                             const uint32_t tick_idx, bool above)
{
    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    lv_draw_layer_ctx_t * layer = lv_event_get_draw_ctx(event);

    // Label text setup 
    char text_buffer[LV_COMAPSS_LABEL_TXT_LEN] = {0};
    lv_area_t label_coords;

    // Check if the custom text array has element for this major tick index 
    if(comapss->txt_src) {
        comapss_build_custom_label_text(obj, label_dsc, text_buffer, major_tick_idx);
    }
    else { // Add label with mapped values
        lv_snprintf(text_buffer, sizeof(text_buffer), "%" LV_PRId32, tick_value);
        //label_dsc->text = text_buffer;
        //label_dsc->text_local = 1;
    }

    if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)
       || (LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode || LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode)) {
        comapss_get_label_coords(obj, label_dsc,text_buffer, tick_point_b, &label_coords, above);
    }
    else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        lv_area_t comapss_area;
        lv_obj_get_content_coords(obj, &comapss_area);

        // Find the center of the comapss 
        lv_point_t center_point;
        int32_t radius_edge = LV_MIN(lv_area_get_width(&comapss_area) / 2U, lv_area_get_height(&comapss_area) / 2U);
        center_point.x = comapss_area.x1 + radius_edge;
        center_point.y = comapss_area.y1 + radius_edge;

        const int32_t major_len = 5;//lv_obj_get_style_length(obj, LV_PART_INDICATOR);
        uint32_t label_gap = LV_COMAPSS_DEFAULT_LABEL_GAP; 

        // Also take into consideration the letter space of the style
        int32_t angle_upcomapss = ((tick_idx * comapss->angle_range) * 10U) / (comapss->total_tick_count - 1);
        angle_upcomapss += comapss->rotation * 10U;

        uint32_t radius_text = 0;
        if(LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
            radius_text = (radius_edge - major_len) - (label_gap + label_dsc->letter_space);
        }
        else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode) {
            radius_text = (radius_edge + major_len) + (label_gap + label_dsc->letter_space);
        }
        else {  }

        lv_point_t point;
        point.x = center_point.x + radius_text;
        point.y = center_point.y;
        lv_point_transform(&point, angle_upcomapss, LV_COMAPSS_NONE, &center_point);
        comapss_get_label_coords(obj, label_dsc,text_buffer, &point, &label_coords, above);
    }
    // Invalid mode 
    else {
        return;
    }
    //lv_draw_label_hint_t * hint; //??
    lv_draw_label(layer, label_dsc, &label_coords, text_buffer, NULL);
}

static void comapss_calculate_main_compensation(lv_obj_t * obj)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    const uint32_t total_tick_count = comapss->total_tick_count;

    if(total_tick_count <= 1) return;
    /* Not supported in round modes */
    if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) return;

    /* Major tick style */
    lv_draw_line_dsc_t major_tick_dsc;
    lv_draw_line_dsc_init(&major_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);

    /* Configure line draw descriptor for the minor tick drawing */
    lv_draw_line_dsc_t minor_tick_dsc;
    lv_draw_line_dsc_init(&minor_tick_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &minor_tick_dsc);

    uint32_t tick_idx = 0;
    uint32_t major_tick_idx = 0;
    int32_t tick_value = comapss->range_min;
    for(tick_idx = 0; tick_idx < total_tick_count; tick_idx++) {

        uint8_t tick_type = tick_idx % comapss->major_tick_every == 0 ? 1 : 0;
        if(tick_type = 1) major_tick_idx++;

        //const int32_t tick_value = lv_map(tick_idx, 0U, total_tick_count - 1, comapss->range_min, comapss->range_max);
        tick_value = lv_compass_limit(tick_value);

        /* Overwrite label and tick properties if tick value is within section range */
        lv_comapss_section_t * section;
        _LV_LL_READ_BACK(&comapss->section_ll, section) {
            if(section->minor_range <= tick_value && section->major_range >= tick_value) {
                if(tick_type == 1) {
                    comapss_set_line_properties(obj, &major_tick_dsc, section->indicator_style, LV_PART_INDICATOR);
                }
                else {
                    comapss_set_line_properties(obj, &minor_tick_dsc, section->items_style, LV_PART_ITEMS);
                }
                break;
            }
            else {
                /* Tick is not in section, get the proper styles */
                lv_obj_init_draw_line_dsc(obj, LV_PART_INDICATOR, &major_tick_dsc);
                lv_obj_init_draw_line_dsc(obj, LV_PART_ITEMS, &minor_tick_dsc);
            }
        }

        /* The tick is represented by a line. We need two points to draw it */
        lv_point_t tick_point_a;
        lv_point_t tick_point_b;
        int32_t fine_tick_idx = (comapss->dir_angle % 10) -0;
        comapss_get_tick_points_offset(obj, tick_idx, fine_tick_idx, tick_type, &tick_point_a, &tick_point_b);
        const bool is_major_tick = tick_type == 1 ? true : false;
        /* Store initial and last tick widths to be used in the main line drawing */
        comapss_store_main_line_tick_width_compensation(obj, tick_idx, is_major_tick, major_tick_dsc.width, minor_tick_dsc.width);
        /* Store the first and last section tick vertical/horizontal position */
        comapss_store_section_line_tick_width_compensation(obj, is_major_tick, &major_tick_dsc, &minor_tick_dsc,
                                                         tick_value, tick_idx, &tick_point_a);
        tick_value += 5;
    }
}

static void comapss_draw_main(lv_obj_t * obj, lv_event_t * event)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    //lv_layer_t * layer = lv_event_get_layer(event);
    lv_draw_layer_ctx_t * layer = lv_event_get_draw_ctx(event);

    if(comapss->total_tick_count <= 1) return;

    if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)
       || (LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode || LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode)) {

        /* Configure both line and label draw descriptors for the tick and label drawings */
        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &line_dsc);

        /* Get style properties so they can be used in the main line drawing */
        const int32_t border_width = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
        const int32_t pad_top = lv_obj_get_style_pad_top(obj, LV_PART_MAIN) + border_width;
        const int32_t pad_bottom = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN) + border_width;
        const int32_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN) + border_width;
        const int32_t pad_right = lv_obj_get_style_pad_right(obj, LV_PART_MAIN) + border_width;

        int32_t x_ofs = 0U;
        int32_t y_ofs = 0U;

        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) {
            x_ofs = obj->coords.x2 + (line_dsc.width / 2U) - pad_right;
            y_ofs = obj->coords.y1 + pad_top;
        }
        else if(LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            x_ofs = obj->coords.x1 + (line_dsc.width / 2U) + pad_left;
            y_ofs = obj->coords.y1 + pad_top;
        }
        if(LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode) {
            x_ofs = obj->coords.x1 + pad_right;
            y_ofs = obj->coords.y1 + (line_dsc.width / 2U) + pad_top;
        }
        else if(LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode) {
            x_ofs = obj->coords.x1 + pad_left;
            y_ofs = obj->coords.y2 + (line_dsc.width / 2U) - pad_bottom;
        }
        else { /* Nothing to do */ }

        lv_point_t main_line_point_a;
        lv_point_t main_line_point_b;

        /* Setup the tick points */
        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            main_line_point_a.x = x_ofs - 1U;
            main_line_point_a.y = y_ofs;
            main_line_point_b.x = x_ofs - 1U;
            main_line_point_b.y = obj->coords.y2 - pad_bottom;

            /* Adjust main line with initial and last tick width */
            main_line_point_a.y -= comapss->last_tick_width / 2U;
            main_line_point_b.y += comapss->first_tick_width / 2U;
        }
        else {
            main_line_point_a.x = x_ofs;
            main_line_point_a.y = y_ofs;
            /* X of second point starts at the edge of the object minus the left pad */
            main_line_point_b.x = obj->coords.x2 - (pad_left);
            main_line_point_b.y = y_ofs;

            /* Adjust main line with initial and last tick width */
            main_line_point_a.x -= comapss->last_tick_width / 2U;
            main_line_point_b.x += comapss->first_tick_width / 2U;
        }

        //line_dsc.p1 = lv_point_to_precise(&main_line_point_a);
        //line_dsc.p2 = lv_point_to_precise(&main_line_point_b);
        lv_draw_line(layer, &line_dsc, &main_line_point_a, &main_line_point_b);

        lv_comapss_section_t * section;
        _LV_LL_READ_BACK(&comapss->section_ll, section) {
            lv_draw_line_dsc_t section_line_dsc;
            lv_draw_line_dsc_init(&section_line_dsc);
            lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &section_line_dsc);

            /* Calculate the points of the section line */
            lv_point_t section_point_a;
            lv_point_t section_point_b;

            const int32_t first_tick_width_halved = (int32_t)(section->first_tick_in_section_width / 2U);
            const int32_t last_tick_width_halved = (int32_t)(section->last_tick_in_section_width / 2U);

            /* Calculate the position of the section based on the ticks (first and last) index */
            if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
                /* Calculate position of the first tick in the section */
                section_point_a.x = main_line_point_a.x;
                section_point_a.y = section->first_tick_in_section.y + first_tick_width_halved;

                /* Calculate position of the last tick in the section */
                section_point_b.x = main_line_point_a.x;
                section_point_b.y = section->last_tick_in_section.y - last_tick_width_halved;
            }
            else {
                /* Calculate position of the first tick in the section */
                section_point_a.x = section->first_tick_in_section.x - first_tick_width_halved;
                section_point_a.y = main_line_point_a.y;

                /* Calculate position of the last tick in the section */
                section_point_b.x = section->last_tick_in_section.x + last_tick_width_halved;
                section_point_b.y = main_line_point_a.y;
            }

            comapss_set_line_properties(obj, &section_line_dsc, section->main_style, LV_PART_MAIN);

            // section_line_dsc.p1.x = section_point_a.x;
            // section_line_dsc.p1.y = section_point_a.y;
            // section_line_dsc.p2.x = section_point_b.x;
            // section_line_dsc.p2.y = section_point_b.y;
            lv_draw_line(layer, &section_line_dsc, &section_point_a, &section_point_b);
        }
    }
    else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        /* Configure arc draw descriptors for the main part */
        lv_draw_arc_dsc_t arc_dsc;
        lv_draw_arc_dsc_init(&arc_dsc);
        lv_obj_init_draw_arc_dsc(obj, LV_PART_MAIN, &arc_dsc);

        lv_point_t arc_center;
        int16_t arc_radius;
        comapss_get_center(obj, &arc_center, &arc_radius);

        /* TODO: Add compensation for the width of the first and last tick over the arc */
        const int32_t start_angle = lv_map(comapss->range_min, comapss->range_min, comapss->range_max, comapss->rotation,
                                           comapss->rotation + comapss->angle_range);
        const int32_t end_angle = lv_map(comapss->range_max, comapss->range_min, comapss->range_max, comapss->rotation,
                                         comapss->rotation + comapss->angle_range);

        // arc_dsc.center = arc_center;
        // arc_dsc.radius = arc_radius;
        // arc_dsc.start_angle = start_angle;
        // arc_dsc.end_angle = end_angle;

        lv_draw_arc(layer, &arc_dsc, &arc_center, arc_radius, start_angle, end_angle);

        lv_comapss_section_t * section;
        _LV_LL_READ_BACK(&comapss->section_ll, section) {
            lv_draw_arc_dsc_t main_arc_section_dsc;
            lv_draw_arc_dsc_init(&main_arc_section_dsc);
            lv_obj_init_draw_arc_dsc(obj, LV_PART_MAIN, &main_arc_section_dsc);

            lv_point_t section_arc_center;
            int32_t section_arc_radius;
            comapss_get_center(obj, &section_arc_center, &section_arc_radius);

            /* TODO: Add compensation for the width of the first and last tick over the arc */
            const int32_t section_start_angle = lv_map(section->minor_range, comapss->range_min, comapss->range_max, comapss->rotation,
                                                       comapss->rotation + comapss->angle_range);
            const int32_t section_end_angle = lv_map(section->major_range, comapss->range_min, comapss->range_max, comapss->rotation,
                                                     comapss->rotation + comapss->angle_range);

            comapss_set_arc_properties(obj, &main_arc_section_dsc, section->main_style);

            // main_arc_section_dsc.center = section_arc_center;
            // main_arc_section_dsc.radius = section_arc_radius;
            // main_arc_section_dsc.start_angle = section_start_angle;
            // main_arc_section_dsc.end_angle = section_end_angle;

            lv_draw_arc(layer, &main_arc_section_dsc, &section_arc_center, section_arc_radius, section_start_angle, section_end_angle);
        }
    }
    else { /* Nothing to do */ }
}

/**
 * Get center point and radius of comapss arc
 * @param obj       pointer to a comapss object
 * @param center    pointer to center
 * @param arc_r     pointer to arc radius
 */
static void comapss_get_center(const lv_obj_t * obj, lv_point_t * center, int32_t * arc_r)
{

    int32_t left_bg = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    int32_t right_bg = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
    int32_t top_bg = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    int32_t bottom_bg = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);

    int32_t r = (LV_MIN(lv_obj_get_width(obj) - left_bg - right_bg, lv_obj_get_height(obj) - top_bg - bottom_bg)) / 2U;

    center->x = obj->coords.x1 + r + left_bg;
    center->y = obj->coords.y1 + r + top_bg;

    if(arc_r) *arc_r = r;
}

/**
 * Get points for ticks
 *
 * In order to draw ticks we need two points, this interface returns both points for all comapss modes.
 *
 * @param obj       pointer to a comapss object
 * @param tick_idx  index of the current tick
 * @param is_major_tick true if tick_idx is a major tick
 * @param tick_point_a  pointer to point 'a' of the tick
 * @param tick_point_b  pointer to point 'b' of the tick
 */
static void comapss_get_tick_points(lv_obj_t * obj, const uint32_t tick_idx, bool is_major_tick,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    /* Main line style */
    lv_draw_line_dsc_t main_line_dsc;
    lv_draw_line_dsc_init(&main_line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &main_line_dsc);

    int32_t minor_len = 0;
    int32_t major_len = 0;

    if(is_major_tick) {
        major_len = 10;//lv_obj_get_style_length(obj, LV_PART_INDICATOR);
    }
    else {
        minor_len = 5;//lv_obj_get_style_length(obj, LV_PART_ITEMS);
    }

    if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)
       || (LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode || LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode)) {

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

        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) {
            x_ofs = obj->coords.x2 + (main_line_dsc.width / 2U) - pad_right;
            y_ofs = obj->coords.y1 + (pad_top + tick_pad_top);
        }
        else if(LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            x_ofs = obj->coords.x1 + (main_line_dsc.width / 2U) + pad_left;
            y_ofs = obj->coords.y1 + (pad_top + tick_pad_top);
        }
        else if(LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode) {
            x_ofs = obj->coords.x1 + (pad_right + tick_pad_right);
            y_ofs = obj->coords.y1 + (main_line_dsc.width / 2U) + pad_top;
        }
        /* LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode */
        else {
            x_ofs = obj->coords.x1 + (pad_left + tick_pad_left);
            y_ofs = obj->coords.y2 + (main_line_dsc.width / 2U) - pad_bottom;
        }

        /* Adjust length when tick will be drawn on horizontal top or vertical right comapsss */
        if((LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode) || (LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)) {
            if(is_major_tick) {
                major_len *= -1;
            }
            else {
                minor_len *= -1;
            }
        }
        else { /* Nothing to do */ }

        const int32_t tick_length = is_major_tick ? major_len : minor_len;
        /* NOTE
         * Minus 1 because tick count starts at 0
         * TODO
         * What if total_tick_count is 1? This will lead to an division by 0 further down */
        const uint32_t tmp_tick_count = comapss->total_tick_count - 1U;

        /* Calculate the position of the tick points based on the mode and tick index */
        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            /* Vertical position starts at y2 of the comapss main line, we start at y2 because the ticks are drawn from bottom to top */
            int32_t vertical_position = obj->coords.y2 - (pad_bottom + tick_pad_bottom);

            /* Position the last tick */
            if(tmp_tick_count == tick_idx) {
                vertical_position = y_ofs;
            }
            /* Otherwise adjust the tick position depending of its index and number of ticks on the comapss */
            else if(0 != tick_idx) {
                const int32_t comapss_total_height = lv_obj_get_height(obj) - (pad_top + pad_bottom + tick_pad_top + tick_pad_bottom);
                const int32_t offset = ((int32_t) tick_idx * (int32_t) comapss_total_height) / (int32_t)(tmp_tick_count);
                vertical_position -= offset;
            }
            else { /* Nothing to do */ }

            tick_point_a->x = x_ofs - 1U; /* Move extra pixel out of comapss boundary */
            tick_point_a->y = vertical_position;
            tick_point_b->x = tick_point_a->x - tick_length;
            tick_point_b->y = vertical_position;
        }
        else {
            /* Horizontal position starts at x1 of the comapss main line */
            int32_t horizontal_position = x_ofs;

            /* Position the last tick */
            if(tmp_tick_count == tick_idx) {
                horizontal_position = obj->coords.x2 - (pad_left + tick_pad_left);
            }
            /* Otherwise adjust the tick position depending of its index and number of ticks on the comapss */
            else if(0U != tick_idx) {
                const int32_t comapss_total_width = lv_obj_get_width(obj) - (pad_right + pad_left + tick_pad_right + tick_pad_left);
                const int32_t offset = ((int32_t) tick_idx * (int32_t) comapss_total_width) / (int32_t)(tmp_tick_count);
                horizontal_position += offset;
            }
            else { /* Nothing to do */ }

            tick_point_a->x = horizontal_position;
            tick_point_a->y = y_ofs;
            tick_point_b->x = horizontal_position;
            tick_point_b->y = tick_point_a->y + tick_length;
        }
    }
    else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        lv_area_t comapss_area;
        lv_obj_get_content_coords(obj, &comapss_area);

        /* Find the center of the comapss */
        lv_point_t center_point;
        const int32_t radius_edge = LV_MIN(lv_area_get_width(&comapss_area) / 2U, lv_area_get_height(&comapss_area) / 2U);
        center_point.x = comapss_area.x1 + radius_edge;
        center_point.y = comapss_area.y1 + radius_edge;

        int32_t angle_upcomapss = ((tick_idx * comapss->angle_range) * 10U) / (comapss->total_tick_count - 1);
        angle_upcomapss += comapss->rotation * 10U;

        /* Draw a little bit longer lines to be sure the mask will clip them correctly
         * and to get a better precision. Adding the main line width to the calculation so we don't have gaps
         * between the arc and the ticks */
        int32_t point_closer_to_arc = 0;
        int32_t adjusted_radio_with_tick_len = 0;
        if(LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
            point_closer_to_arc = radius_edge - main_line_dsc.width;
            adjusted_radio_with_tick_len = point_closer_to_arc - (is_major_tick ? major_len : minor_len);
        }
        /* LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode */
        else {
            point_closer_to_arc = radius_edge - main_line_dsc.width;
            adjusted_radio_with_tick_len = point_closer_to_arc + (is_major_tick ? major_len : minor_len);
        }

        tick_point_a->x = center_point.x + point_closer_to_arc;
        tick_point_a->y = center_point.y;
        lv_point_transform(tick_point_a, angle_upcomapss, LV_COMAPSS_NONE, &center_point);

        tick_point_b->x = center_point.x + adjusted_radio_with_tick_len;
        tick_point_b->y = center_point.y;
        lv_point_transform(tick_point_b, angle_upcomapss, LV_COMAPSS_NONE, &center_point);
    }
    else { /* Nothing to do */ }
}

/**
 * Get points for ticks
 *
 * In order to draw ticks we need two points, this interface returns both points for all comapss modes.
 *
 * @param obj       pointer to a comapss object
 * @param tick_idx  index of the current tick
 * @param is_major_tick true if tick_idx is a major tick
 * @param tick_point_a  pointer to point 'a' of the tick
 * @param tick_point_b  pointer to point 'b' of the tick
 */
static void comapss_get_tick_points_offset(lv_obj_t * obj, uint32_t tick_idx, int32_t fine_tick_val, int8_t tick_type,
                                  lv_point_t * tick_point_a, lv_point_t * tick_point_b)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    /* Main line style */
    lv_draw_line_dsc_t main_line_dsc;
    lv_draw_line_dsc_init(&main_line_dsc);
    lv_obj_init_draw_line_dsc(obj, LV_PART_MAIN, &main_line_dsc);

    int32_t minor_len = 0;
    int32_t major_len = 0;

    int32_t fine_tick_range = 5;
    //int32_t fine_tick_val = 0;
    tick_idx =  - fine_tick_val + (tick_idx * fine_tick_range);

    if(tick_type == 1) {
        major_len = 10;//lv_obj_get_style_length(obj, LV_PART_INDICATOR);
    }
    if(tick_type == 0)  {
        minor_len = 5;//lv_obj_get_style_length(obj, LV_PART_ITEMS);
    }
    if(tick_type == 2)  {
        major_len = 10;
    }

    if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)
       || (LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode || LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode)) {

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

        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) {
            x_ofs = obj->coords.x2 + (main_line_dsc.width / 2U) - pad_right;
            y_ofs = obj->coords.y1 + (pad_top + tick_pad_top);
        }
        else if(LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            x_ofs = obj->coords.x1 + (main_line_dsc.width / 2U) + pad_left;
            y_ofs = obj->coords.y1 + (pad_top + tick_pad_top);
        }
        else if(LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode) {
            x_ofs = obj->coords.x1 + (pad_right + tick_pad_right);
            y_ofs = obj->coords.y1 + (main_line_dsc.width / 2U) + pad_top;
        }
        /* LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode */
        else {
            x_ofs = obj->coords.x1 + (pad_left + tick_pad_left);
            y_ofs = obj->coords.y2 + (main_line_dsc.width / 2U) - pad_bottom;
        }
        y_ofs += 15;
        // if(tick_type == 1) y_ofs += 10;
        // if(tick_type == 0) y_ofs += 10;
        // if(tick_type == 2) y_ofs += 10;
        /* Adjust length when tick will be drawn on horizontal top or vertical right comapsss */
        if((LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode) || (LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)) {
            if(tick_type == 1)  {
                major_len *= -1;
            }
            if(tick_type == 0)  {
                minor_len *= -1;
            }
            if(tick_type == 2)  {
                major_len *= -1;
            }
        }
        else { /* Nothing to do */ }

        int32_t tick_length = major_len;

        if(tick_type == 1)  {
                tick_length = major_len;
            }
        if(tick_type == 0)  {
                tick_length = minor_len;
            }
        if(tick_type == 2)  {
                tick_length = major_len;
            }
        /* NOTE
         * Minus 1 because tick count starts at 0
         * TODO
         * What if total_tick_count is 1? This will lead to an division by 0 further down */
        const uint32_t tmp_tick_count = (comapss->total_tick_count - 1U) * fine_tick_range;

        /* Calculate the position of the tick points based on the mode and tick index */
        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
            /* Vertical position starts at y2 of the comapss main line, we start at y2 because the ticks are drawn from bottom to top */
            int32_t vertical_position = obj->coords.y2 - (pad_bottom + tick_pad_bottom);

            /* Position the last tick */
            if(tmp_tick_count == tick_idx) {
                vertical_position = y_ofs;
            }
            /* Otherwise adjust the tick position depending of its index and number of ticks on the comapss */
            else if(0 != tick_idx) {
                const int32_t comapss_total_height = lv_obj_get_height(obj) - (pad_top + pad_bottom + tick_pad_top + tick_pad_bottom);
                const int32_t offset = ((int32_t) tick_idx * (int32_t) comapss_total_height) / (int32_t)(tmp_tick_count);
                vertical_position -= offset;
            }
            else { /* Nothing to do */ }

            tick_point_a->x = x_ofs - 1U; /* Move extra pixel out of comapss boundary */
            tick_point_a->y = vertical_position;
            tick_point_b->x = tick_point_a->x - tick_length;
            tick_point_b->y = vertical_position;
        }
        else {
            /* Horizontal position starts at x1 of the comapss main line */
            int32_t horizontal_position = x_ofs;

            /* Position the last tick */
            if(tmp_tick_count == tick_idx) {
                horizontal_position = obj->coords.x2 - (pad_left + tick_pad_left);
            }
            /* Otherwise adjust the tick position depending of its index and number of ticks on the comapss */
            else if(0U != tick_idx) {
                const int32_t comapss_total_width = lv_obj_get_width(obj) - (pad_right + pad_left + tick_pad_right + tick_pad_left);
                const int32_t offset = ((int32_t) tick_idx * (int32_t) comapss_total_width) / (int32_t)(tmp_tick_count);
                horizontal_position += offset;
            }
            else { /* Nothing to do */ }

            tick_point_a->x = horizontal_position;
            tick_point_a->y = y_ofs;
            tick_point_b->x = horizontal_position;
            tick_point_b->y = tick_point_a->y + tick_length;
        }
    }
    else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        lv_area_t comapss_area;
        lv_obj_get_content_coords(obj, &comapss_area);

        /* Find the center of the comapss */
        lv_point_t center_point;
        const int32_t radius_edge = LV_MIN(lv_area_get_width(&comapss_area) / 2U, lv_area_get_height(&comapss_area) / 2U);
        center_point.x = comapss_area.x1 + radius_edge;
        center_point.y = comapss_area.y1 + radius_edge;

        int32_t angle_upcomapss = ((tick_idx * comapss->angle_range) * 10U) / (comapss->total_tick_count - 1)*fine_tick_range;
        angle_upcomapss += comapss->rotation * 10U;

        /* Draw a little bit longer lines to be sure the mask will clip them correctly
         * and to get a better precision. Adding the main line width to the calculation so we don't have gaps
         * between the arc and the ticks */
        int32_t point_closer_to_arc = 0;
        int32_t adjusted_radio_with_tick_len = 0;
        uint32_t adj_tick = 0;
        if(tick_type == 1)  {
                adj_tick = major_len;
            }
        if(tick_type == 0)  {
                adj_tick = minor_len;
            }
        if(tick_type == 2)  {
                adj_tick = major_len;
            }
        if(LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
            point_closer_to_arc = radius_edge - main_line_dsc.width;
            adjusted_radio_with_tick_len = point_closer_to_arc - adj_tick;
        }
        /* LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode */
        else {
            point_closer_to_arc = radius_edge - main_line_dsc.width;
            adjusted_radio_with_tick_len = point_closer_to_arc + adj_tick;
        }

        tick_point_a->x = center_point.x + point_closer_to_arc;
        tick_point_a->y = center_point.y;
        lv_point_transform(tick_point_a, angle_upcomapss, LV_COMAPSS_NONE, &center_point);

        tick_point_b->x = center_point.x + adjusted_radio_with_tick_len;
        tick_point_b->y = center_point.y;
        lv_point_transform(tick_point_b, angle_upcomapss, LV_COMAPSS_NONE, &center_point);
    }
    else { /* Nothing to do */ }
}

/**
 * Get coordinates for label
 *
 * @param obj       pointer to a comapss object
 * @param label_dsc pointer to label descriptor
 * @param tick_point    pointer to reference tick
 * @param label_coords  pointer to label coordinates output
 */
static void comapss_get_label_coords(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc,const char * text, lv_point_t * tick_point,
                                   lv_area_t * label_coords, bool above)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    
    /* Reserve appropriate size for the tick label */
    lv_point_t label_size;
    lv_txt_get_size(&label_size, text,
                     label_dsc->font, label_dsc->letter_space, label_dsc->line_space, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    //tick_point->y = tick_point->y - label_size.y -10;
    const uint8_t offs_y = above ? label_size.y : 0;
    /* Set the label draw area at some distance of the major tick */
    if((LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode) || (LV_COMAPSS_MODE_HORIZONTAL_TOP == comapss->mode)) {
        label_coords->x1 = tick_point->x - (label_size.x / 2U);
        label_coords->x2 = tick_point->x + (label_size.x / 2U);

        if(LV_COMAPSS_MODE_HORIZONTAL_BOTTOM == comapss->mode) {
            label_coords->y1 = tick_point->y + lv_obj_get_style_pad_bottom(obj, LV_PART_INDICATOR) - offs_y;
            label_coords->y2 = label_coords->y1 + label_size.y;
        }
        else {
            label_coords->y2 = tick_point->y - lv_obj_get_style_pad_top(obj, LV_PART_INDICATOR) - offs_y;
            label_coords->y1 = label_coords->y2 - label_size.y;
        }
    }
    else if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) || (LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)) {
        label_coords->y1 = tick_point->y - (label_size.y / 2U) - offs_y;
        label_coords->y2 = tick_point->y + (label_size.y / 2U) - offs_y;

        if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) {
            label_coords->x1 = tick_point->x - label_size.x - lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR);
            label_coords->x2 = tick_point->x - lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR);
        }
        else {
            label_coords->x1 = tick_point->x + lv_obj_get_style_pad_right(obj, LV_PART_INDICATOR);
            label_coords->x2 = tick_point->x + label_size.x + lv_obj_get_style_pad_right(obj, LV_PART_INDICATOR);
        }
    }
    else if(LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode || LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) {
        label_coords->x1 = tick_point->x - (label_size.x / 2U);
        label_coords->y1 = tick_point->y - (label_size.y / 2U) - offs_y;
        label_coords->x2 = label_coords->x1 + label_size.x;
        label_coords->y2 = label_coords->y1 + label_size.y;
    }
    else { /* Nothing to do */ }

}

/**
 * Set line properties
 *
 * Checks if the line has a custom section configuration or not and sets the properties accordingly.
 *
 * @param obj       pointer to a comapss object
 * @param line_dsc  pointer to line descriptor
 * @param items_section_style  pointer to indicator section style
 * @param part      line part, example: LV_PART_INDICATOR, LV_PART_ITEMS, LV_PART_MAIN
 */
static void comapss_set_line_properties(lv_obj_t * obj, lv_draw_line_dsc_t * line_dsc, lv_style_t * section_style,
                                      lv_part_t part)
{

    if(section_style) {
        lv_style_value_t value;
        int res;

        /* Line width */
        res = lv_style_get_prop(section_style, LV_STYLE_LINE_WIDTH, &value);
        if(res == LV_RESULT_OK) {
            line_dsc->width = (int32_t)value.num;
        }
        else {
            line_dsc->width = lv_obj_get_style_line_width(obj, part);
        }

        /* Line color */
        res = lv_style_get_prop(section_style, LV_STYLE_LINE_COLOR, &value);
        if(res == LV_RESULT_OK) {
            line_dsc->color = value.color;
        }
        else {
            line_dsc->color = lv_obj_get_style_line_color(obj, part);
        }

        /* Line opa */
        res = lv_style_get_prop(section_style, LV_STYLE_LINE_OPA, &value);
        if(res == LV_RESULT_OK) {
            line_dsc->opa = (lv_opa_t)value.num;
        }
        else {
            line_dsc->opa = lv_obj_get_style_line_opa(obj, part);
        }
    }
    else {
        line_dsc->color = lv_obj_get_style_line_color(obj, part);
        line_dsc->opa = lv_obj_get_style_line_opa(obj, part);
        line_dsc->width = lv_obj_get_style_line_width(obj, part);
    }
}

/**
 * Set arc properties
 *
 * Checks if the arc has a custom section configuration or not and sets the properties accordingly.
 *
 * @param obj       pointer to a comapss object
 * @param line_dsc  pointer to arc descriptor
 * @param items_section_style  pointer to indicator section style
 */
static void comapss_set_arc_properties(lv_obj_t * obj, lv_draw_arc_dsc_t * arc_dsc, lv_style_t * section_style)
{

    if(section_style) {
        lv_style_value_t value;
        int res;

        /* Line width */
        res = lv_style_get_prop(section_style, LV_STYLE_ARC_WIDTH, &value);
        if(res == LV_RESULT_OK) {
            arc_dsc->width = (int32_t)value.num;
        }
        else {
            arc_dsc->width = lv_obj_get_style_line_width(obj, LV_PART_MAIN);
        }

        /* Line color */
        res = lv_style_get_prop(section_style, LV_STYLE_ARC_COLOR, &value);
        if(res == LV_RESULT_OK) {
            arc_dsc->color = value.color;
        }
        else {
            arc_dsc->color = lv_obj_get_style_line_color(obj, LV_PART_MAIN);
        }

        /* Line opa */
        res = lv_style_get_prop(section_style, LV_STYLE_ARC_OPA, &value);
        if(res == LV_RESULT_OK) {
            arc_dsc->opa = (lv_opa_t)value.num;
        }
        else {
            arc_dsc->opa = lv_obj_get_style_line_opa(obj, LV_PART_MAIN);
        }
    }
    else {
        arc_dsc->color = lv_obj_get_style_line_color(obj, LV_PART_MAIN);
        arc_dsc->opa = lv_obj_get_style_line_opa(obj, LV_PART_MAIN);
        arc_dsc->width = lv_obj_get_style_line_width(obj, LV_PART_MAIN);
    }
}

/**
 * Set indicator label properties
 *
 * Checks if the indicator has a custom section configuration or not and sets the properties accordingly.
 *
 * @param obj       pointer to a comapss object
 * @param label_dsc  pointer to label descriptor
 * @param items_section_style  pointer to indicator section style
 */
static void comapss_set_indicator_label_properties(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc,
                                                 lv_style_t * indicator_section_style)
{

    if(indicator_section_style) {
        lv_style_value_t value;
        int res;

        /* Text color */
        res = lv_style_get_prop(indicator_section_style, LV_STYLE_TEXT_COLOR, &value);
        if(res == LV_RESULT_OK) {
            label_dsc->color = value.color;
        }
        else {
            label_dsc->color = lv_obj_get_style_text_color(obj, LV_PART_INDICATOR);
        }

        /* Text opa */
        res = lv_style_get_prop(indicator_section_style, LV_STYLE_TEXT_OPA, &value);
        if(res == LV_RESULT_OK) {
            label_dsc->opa = (lv_opa_t)value.num;
        }
        else {
            label_dsc->opa = lv_obj_get_style_text_opa(obj, LV_PART_INDICATOR);
        }

        /* Text letter space */
        res = lv_style_get_prop(indicator_section_style, LV_STYLE_TEXT_LETTER_SPACE, &value);
        if(res == LV_RESULT_OK) {
            label_dsc->letter_space = (int32_t)value.num;
        }
        else {
            label_dsc->letter_space = lv_obj_get_style_text_letter_space(obj, LV_PART_INDICATOR);
        }

        /* Text font */
        res = lv_style_get_prop(indicator_section_style, LV_STYLE_TEXT_FONT, &value);
        if(res == LV_RESULT_OK) {
            label_dsc->font = (const lv_font_t *)value.ptr;
        }
        else {
            label_dsc->font = lv_obj_get_style_text_font(obj, LV_PART_INDICATOR);
        }
    }
    else {
        /* If label is not within a range then get the indicator style */
        label_dsc->color = lv_obj_get_style_text_color(obj, LV_PART_INDICATOR);
        label_dsc->opa = lv_obj_get_style_text_opa(obj, LV_PART_INDICATOR);
        label_dsc->letter_space = lv_obj_get_style_text_letter_space(obj, LV_PART_INDICATOR);
        label_dsc->font = lv_obj_get_style_text_font(obj, LV_PART_INDICATOR);
    }
}

static void comapss_find_section_tick_idx(lv_obj_t * obj)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;

    const int32_t min_out = comapss->range_min;
    const int32_t max_out = comapss->range_max;
    const uint32_t total_tick_count = comapss->total_tick_count;

    /* Section handling */
    uint32_t tick_idx = 0;
    for(tick_idx = 0; tick_idx < total_tick_count; tick_idx++) {
        bool is_major_tick = false;
        if(tick_idx % comapss->major_tick_every == 0) is_major_tick = true;

        const int32_t tick_value = lv_map(tick_idx, 0U, total_tick_count - 1, min_out, max_out);

        lv_comapss_section_t * section;
        _LV_LL_READ_BACK(&comapss->section_ll, section) {
            if(section->minor_range <= tick_value && section->major_range >= tick_value) {
                if(LV_COMAPSS_TICK_IDX_DEFAULT_ID == section->first_tick_idx_in_section) {
                    section->first_tick_idx_in_section = tick_idx;
                    section->first_tick_idx_is_major = is_major_tick;
                }
                if(section->first_tick_idx_in_section != tick_idx) {
                    section->last_tick_idx_in_section = tick_idx;
                    section->last_tick_idx_is_major = is_major_tick;
                }
            }
            else { /* Nothing to do */ }
        }
    }

}

/**
 * Stores the width of the initial and last tick of the main line
 *
 * This width is used to compensate the main line drawing taking into consideration the widths of both ticks
 *
 * @param obj       pointer to a comapss object
 * @param tick_idx  index of the current tick
 * @param is_major_tick true if tick_idx is a major tick
 * @param major_tick_width width of the major tick
 * @param minor_tick_width width of the minor tick
 */
static void comapss_store_main_line_tick_width_compensation(lv_obj_t * obj, const uint32_t tick_idx,
                                                          const bool is_major_tick, const int32_t major_tick_width, const int32_t minor_tick_width)
{

    lv_comapss_t * comapss = (lv_comapss_t *)obj;
    const bool is_first_tick = 0U == tick_idx;
    const bool is_last_tick = comapss->total_tick_count == tick_idx;
    const int32_t tick_width = is_major_tick ? major_tick_width : minor_tick_width;

    /* Exit early if tick_idx is neither the first nor last tick on the main line */
    if(((!is_last_tick) && (!is_first_tick))
       /* Exit early if comapss mode is round. It doesn't support main line compensation */
       || ((LV_COMAPSS_MODE_ROUND_INNER == comapss->mode) || (LV_COMAPSS_MODE_ROUND_OUTER == comapss->mode))) {
        return;
    }

    if(is_last_tick) {
        /* Mode is vertical */
        if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) || (LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)) {
            comapss->last_tick_width = tick_width;
        }
        /* Mode is horizontal */
        else {
            comapss->first_tick_width = tick_width;
        }
    }
    /* is_first_tick */
    else {
        /* Mode is vertical */
        if((LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode) || (LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode)) {
            comapss->first_tick_width = tick_width;
        }
        /* Mode is horizontal */
        else {
            comapss->last_tick_width = tick_width;
        }
    }
}

/**
 * Sets the text of the tick label descriptor when using custom labels
 *
 * Sets the text pointer when valid custom label is available, otherwise set it to NULL.
 *
 * @param obj       pointer to a comapss object
 * @param label_dsc pointer to the label descriptor
 * @param major_tick_idx  index of the current major tick
 */
static void comapss_build_custom_label_text(lv_obj_t * obj, lv_draw_label_dsc_t * label_dsc, const char * text,
                                          const uint16_t major_tick_idx)
{
    lv_comapss_t * comapss = (lv_comapss_t *) obj;

    /* Check if the comapss has valid custom labels available,
     * this avoids reading past txt_src array when the comapss requires more tick labels than available */
    if(major_tick_idx <= comapss->custom_label_cnt) {
        if(comapss->txt_src[major_tick_idx - 1U]) {
            // label_dsc->text = comapss->txt_src[major_tick_idx - 1U];
            // label_dsc->text_local = 0;
            text = comapss->txt_src[major_tick_idx - 1U];
            //label_dsc->text_local = 0;
        }
        else {
            text = NULL;
        }
    }
    else {
        text = NULL;
    }
}

/**
 * Stores tick width compensation information for main line sections
 *
 * @param obj       pointer to a comapss object
 * @param is_major_tick Indicates if tick is major or not
 * @param major_tick_dsc pointer to the major_tick_dsc
 * @param minor_tick_dsc pointer to the minor_tick_dsc
 * @param tick_value Current tick value, used to know if tick_idx belongs to a section or not
 * @param tick_idx Current tick index
 * @param tick_point_a Pointer to tick point a
 */
static void comapss_store_section_line_tick_width_compensation(lv_obj_t * obj, const bool is_major_tick,
                                                             lv_draw_line_dsc_t * major_tick_dsc, lv_draw_line_dsc_t * minor_tick_dsc,
                                                             const int32_t tick_value, const uint8_t tick_idx, lv_point_t * tick_point_a)
{

    lv_comapss_t * comapss = (lv_comapss_t *) obj;
    lv_comapss_section_t * section;

    _LV_LL_READ_BACK(&comapss->section_ll, section) {
        if(section->minor_range <= tick_value && section->major_range >= tick_value) {
            if(is_major_tick) {
                comapss_set_line_properties(obj, major_tick_dsc, section->indicator_style, LV_PART_INDICATOR);
            }
            else {
                comapss_set_line_properties(obj, minor_tick_dsc, section->items_style, LV_PART_ITEMS);
            }
        }

        int32_t tmp_width = 0;

        if(tick_idx == section->first_tick_idx_in_section) {
            if(section->first_tick_idx_is_major) {
                tmp_width = major_tick_dsc->width;
            }
            else {
                tmp_width = minor_tick_dsc->width;
            }

            section->first_tick_in_section.y = tick_point_a->y;
            /* Add 1px as adjustment if tmp_width is odd */
            if(tmp_width & 0x01U) {
                if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
                    tmp_width += 1U;
                }
                else {
                    tmp_width -= 1U;
                }
            }
            section->first_tick_in_section_width = tmp_width;
        }
        else if(tick_idx == section->last_tick_idx_in_section) {
            if(section->last_tick_idx_is_major) {
                tmp_width = major_tick_dsc->width;
            }
            else {
                tmp_width = minor_tick_dsc->width;
            }

            section->last_tick_in_section.y = tick_point_a->y;
            /* Add 1px as adjustment if tmp_width is odd */
            if(tmp_width & 0x01U) {
                if(LV_COMAPSS_MODE_VERTICAL_LEFT == comapss->mode || LV_COMAPSS_MODE_VERTICAL_RIGHT == comapss->mode) {
                    tmp_width -= 1U;
                }
                else {
                    tmp_width += 1U;
                }
            }
            section->last_tick_in_section_width = tmp_width;
        }
        else { /* Nothing to do */ }
    }
}

static void comapss_free_line_needle_points_cb(lv_event_t * e)
{
    lv_point_t * needle_line_points = lv_event_get_user_data(e);
    lv_free(needle_line_points);
}

//#endif