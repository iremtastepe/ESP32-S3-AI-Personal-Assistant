
#include "cat/ui/CatAnimation.hpp"
#include "cat_breath.h"
#include "cat_wake_frames.h"

namespace cat::ui {

static const void* wake_frames[] = {
    &cat_wake_00,
    &cat_wake_01,
    &cat_wake_02,
    &cat_wake_03,
    &cat_wake_04,
    &cat_wake_05
};

static void on_cat_clicked(lv_event_t* event)
{
    lv_obj_t* cat = static_cast<lv_obj_t*>(
        lv_event_get_target(event)
    );

    if (lv_obj_has_state(cat, LV_STATE_USER_1)) {
        return;
    }

    lv_obj_add_state(cat, LV_STATE_USER_1);

    lv_animimg_delete(cat);

    lv_animimg_set_src(cat, wake_frames, 6);
    lv_animimg_set_duration(cat, 3000);
    lv_animimg_set_repeat_count(cat, 1);

    lv_animimg_start(cat);
}

lv_obj_t* create_cat_animation(lv_obj_t* parent)
{
    lv_obj_t* cat = cat_breath_create(parent);

    lv_obj_add_flag(cat, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_add_event_cb(
        cat,
        on_cat_clicked,
        LV_EVENT_CLICKED,
        nullptr
    );

    return cat;
}

}
