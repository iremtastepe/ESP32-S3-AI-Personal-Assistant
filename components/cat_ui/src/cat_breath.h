#pragma once
#include "lvgl.h"
#include "cat_breath_frames.h"

// Tamamiyla bu header'a ozel: main.cpp icinde tek noktadan cagiriliyor.
// LVGL ekran kilidi alinmisken kullanilmalidir.
static inline lv_obj_t *cat_breath_create(lv_obj_t *parent)
{
#if LV_USE_ANIMIMG
    static lv_image_dsc_t desc[4] = {};
    static bool initialized = false;
    if (!initialized) {
        cat_breath_setup_frame(&desc[0], cat_breath_pixels_0, sizeof(cat_breath_pixels_0));
        cat_breath_setup_frame(&desc[1], cat_breath_pixels_1, sizeof(cat_breath_pixels_1));
        cat_breath_setup_frame(&desc[2], cat_breath_pixels_2, sizeof(cat_breath_pixels_2));
        cat_breath_setup_frame(&desc[3], cat_breath_pixels_3, sizeof(cat_breath_pixels_3));
        initialized = true;
    }
    static const void *sources[] = {&desc[0], &desc[1], &desc[2], &desc[3]};
    lv_obj_t *cat = lv_animimg_create(parent);
    lv_animimg_set_src(cat, sources, 4);
    lv_animimg_set_duration(cat, 3200);  // 4 kare x 800 ms
    lv_animimg_set_repeat_count(cat, LV_ANIM_REPEAT_INFINITE);
    lv_obj_set_style_transform_scale(cat, 190, LV_PART_MAIN);
    lv_obj_align(cat, LV_ALIGN_CENTER, 20, -52);
    lv_animimg_start(cat);
    return cat;
#else
#error "LV_USE_ANIMIMG kapali. LVGL sdkconfig ayarinda acilmasi gerekiyor."
#endif
}
