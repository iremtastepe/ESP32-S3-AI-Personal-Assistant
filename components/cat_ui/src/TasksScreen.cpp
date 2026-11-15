
#include "cat/ui/TasksScreen.hpp"
#include "cat/ui/HomeScreen.hpp"
#include "lvgl.h"
#include "tr_font.h"

namespace cat::ui {
namespace {

constexpr uint32_t BG = 0xF5F0E8;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t INK = 0x463843;
constexpr uint32_t MUTED = 0x987F97;
constexpr uint32_t PURPLE = 0x896990;
constexpr uint32_t LILAC = 0xEDE4ED;
constexpr uint32_t TRACK = 0xDCCBDD;

// -------------------------
// YARDIMCI FONKSIYONLAR
// -------------------------

lv_obj_t* box(
    lv_obj_t* parent,
    int x,
    int y,
    int w,
    int h,
    uint32_t bg,
    int radius = 0
)
{
    lv_obj_t* obj = lv_obj_create(parent);

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    lv_obj_set_style_bg_color(
        obj, lv_color_hex(bg), 0
    );

    lv_obj_set_style_bg_opa(
        obj, LV_OPA_COVER, 0
    );

    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);

    lv_obj_remove_flag(
        obj, LV_OBJ_FLAG_SCROLLABLE
    );

    return obj;
}

lv_obj_t* txt(
    lv_obj_t* parent,
    const char* value,
    int x,
    int y,
    uint32_t color,
    const lv_font_t* font = &tr_font_12
)
{
    lv_obj_t* obj = lv_label_create(parent);

    lv_label_set_text(obj, value);
    lv_obj_set_pos(obj, x, y);

    lv_obj_set_style_text_color(
        obj, lv_color_hex(color), 0
    );

    lv_obj_set_style_text_font(
        obj, font, 0
    );

    return obj;
}

// -------------------------
// EKRAN GECISLERI
// -------------------------

void go_home(lv_event_t*)
{
    lv_async_call(
        [](void*) {
            create_home_screen();
        },
        nullptr
    );
}

// -------------------------
// GOREV SATIRLARI
// -------------------------

void add_task(
    lv_obj_t* parent,
    int y,
    const char* name,
    bool completed,
    bool carried = false
)
{
    lv_obj_t* row = box(
        parent,
        2, y,
        216, 30,
        WHITE, 10
    );

    lv_obj_t* circle = box(
        row,
        8, 7,
        16, 16,
        completed ? PURPLE : WHITE,
        LV_RADIUS_CIRCLE
    );

    lv_obj_set_style_border_width(
        circle,
        completed ? 0 : 2,
        0
    );

    lv_obj_set_style_border_color(
        circle,
        lv_color_hex(TRACK),
        0
    );

    if (completed) {
        lv_obj_t* check = txt(
            circle,
            LV_SYMBOL_OK,
            0, 0,
            WHITE,
            LV_FONT_DEFAULT
        );

        lv_obj_center(check);
    }

    lv_obj_t* name_label = txt(
        row,
        name,
        32, 8,
        completed ? MUTED : INK
    );

    lv_obj_set_width(
        name_label,
        carried ? 110 : 173
    );

    lv_label_set_long_mode(
        name_label,
        LV_LABEL_LONG_MODE_DOTS
    );

    if (completed) {
        lv_obj_set_style_text_decor(
            name_label,
            LV_TEXT_DECOR_STRIKETHROUGH,
            0
        );
    }

    if (carried) {
        txt(
            row,
            "↻ Devreden",
            143, 8,
            PURPLE
        );
    }
}

// -------------------------
// BOLUM BASLIKLARI
// -------------------------

void add_heading(
    lv_obj_t* parent,
    const char* heading,
    int y
)
{
    txt(
        parent,
        heading,
        3, y,
        INK,
        &tr_font_14
    );
}

// -------------------------
// UST BASLIK
// -------------------------

void create_header(lv_obj_t* screen)
{
    txt(
        screen,
        "10 Kasım, Salı",
        20, 10,
        MUTED
    );

    txt(
        screen,
        "Görevlerim",
        18, 32,
        INK,
        &tr_font_14
    );

    lv_obj_t* add = box(
        screen,
        197, 29,
        32, 32,
        PURPLE,
        LV_RADIUS_CIRCLE
    );

    lv_obj_t* plus = txt(
        add,
        "+",
        0, 0,
        WHITE,
        &tr_font_14
    );

    lv_obj_center(plus);
}

// -------------------------
// GUNLUK ILERLEME
// -------------------------

void create_progress(lv_obj_t* screen)
{
    lv_obj_t* card = box(
        screen,
        10, 67,
        220, 45,
        LILAC, 13
    );

    txt(
        card,
        "Günlük ilerleme",
        11, 7,
        INK
    );

    txt(
        card,
        "1/3",
        184, 7,
        PURPLE
    );

    // Ilerleme cubugu arka plani
    box(
        card,
        11, 29,
        198, 8,
        TRACK, 5
    );

    // Tamamlanan gorev orani
    box(
        card,
        11, 29,
        66, 8,
        PURPLE, 5
    );
}

// -------------------------
// KAYDIRILABILIR GOREV LISTESI
// -------------------------

void create_task_list(lv_obj_t* screen)
{
    lv_obj_t* list = box(
        screen,
        10, 118,
        220, 124,
        BG
    );

    lv_obj_add_flag(
        list,
        LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_set_scroll_dir(
        list,
        LV_DIR_VER
    );

    lv_obj_set_scrollbar_mode(
        list,
        LV_SCROLLBAR_MODE_AUTO
    );

    // BUGUN

    add_heading(
        list,
        "Bugün",
        2
    );

    add_task(
        list,
        23,
        "Kitap oku",
        true
    );

    add_task(
        list,
        57,
        "Ders çalış",
        false
    );

    add_task(
        list,
        91,
        "Projeye devam et",
        false
    );

    // YARIN

    add_heading(
        list,
        "Yarın",
        131
    );

    add_task(
        list,
        152,
        "İngilizce çalış",
        false
    );

    add_task(
        list,
        186,
        "Alışveriş yap",
        false
    );

    // YAKLASAN

    add_heading(
        list,
        "Yaklaşan",
        226
    );

    add_task(
        list,
        247,
        "Proje sunumu hazırla",
        false
    );

    // GECMIS GOREVLER

    lv_obj_t* history = box(
        list,
        2, 289,
        216, 30,
        LILAC, 10
    );

    txt(
        history,
        "Geçmiş görevleri gör  >",
        12, 8,
        PURPLE
    );
}

// -------------------------
// ALT MENU KEDI SIMGESI
// -------------------------

void create_cat_icon(
    lv_obj_t* parent,
    int x,
    int y
)
{
    // Kulaklar
    box(parent, x + 3, y, 3, 7, MUTED);
    box(parent, x + 15, y, 3, 7, MUTED);

    // Kafa
    box(
        parent,
        x + 3,
        y + 6,
        15, 11,
        MUTED, 3
    );

    // Gozler
    box(
        parent,
        x + 7,
        y + 10,
        2, 2,
        WHITE
    );

    box(
        parent,
        x + 13,
        y + 10,
        2, 2,
        WHITE
    );
}

// -------------------------
// ALT MENU
// -------------------------

void create_navigation(lv_obj_t* screen)
{
    lv_obj_t* nav = box(
        screen,
        0, 245,
        240, 39,
        WHITE
    );

    // ANA SAYFA

    txt(
        nav,
        LV_SYMBOL_HOME,
        35, 2,
        MUTED,
        LV_FONT_DEFAULT
    );

    txt(
        nav,
        "Ana sayfa",
        15, 20,
        MUTED
    );

    // GOREVLER

    txt(
        nav,
        LV_SYMBOL_LIST,
        115, 2,
        PURPLE,
        LV_FONT_DEFAULT
    );

    txt(
        nav,
        "Görevler",
        95, 20,
        PURPLE
    );

    // KEDIM

    create_cat_icon(
        nav,
        188, 2
    );

    txt(
        nav,
        "Kedim",
        177, 20,
        MUTED
    );

    // ANA SAYFAYA DONUS ICIN
    // GORUNMEZ DOKUNMATIK ALAN

    lv_obj_t* home_button = box(
        nav,
        0, 0,
        80, 39,
        WHITE
    );

    lv_obj_set_style_bg_opa(
        home_button,
        LV_OPA_TRANSP,
        0
    );

    lv_obj_add_flag(
        home_button,
        LV_OBJ_FLAG_CLICKABLE
    );

    lv_obj_add_event_cb(
        home_button,
        go_home,
        LV_EVENT_CLICKED,
        nullptr
    );
}

} // namespace

// -------------------------
// GOREVLERIM ANA FONKSIYONU
// -------------------------

void create_tasks_screen()
{
    lv_obj_t* screen = lv_screen_active();

    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(BG),
        0
    );

    lv_obj_set_style_bg_opa(
        screen,
        LV_OPA_COVER,
        0
    );

    lv_obj_remove_flag(
        screen,
        LV_OBJ_FLAG_SCROLLABLE
    );

    create_header(screen);
    create_progress(screen);
    create_task_list(screen);
    create_navigation(screen);
}

} // namespace cat::ui
