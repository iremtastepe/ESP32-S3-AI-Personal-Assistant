
#include "cat/ui/CatScreen.hpp"
#include "cat/ui/HomeScreen.hpp"
#include "cat/ui/TasksScreen.hpp"

#include "lvgl.h"
#include "tr_font.h"

#include <cstdint>

namespace cat::ui {
namespace {

constexpr int CANVAS_W = 200;
constexpr int CANVAS_H = 146;

constexpr uint32_t BG     = 0xF5F0E8;
constexpr uint32_t WHITE  = 0xFFFFFF;
constexpr uint32_t BLACK  = 0x242128;
constexpr uint32_t SHADE  = 0x37313B;
constexpr uint32_t GOLD   = 0xE7B955;
constexpr uint32_t PINK   = 0xD78DAF;
constexpr uint32_t PURPLE = 0x896990;
constexpr uint32_t LILAC  = 0xEDE4ED;
constexpr uint32_t MUTED  = 0x987F97;
constexpr uint32_t INK    = 0x463843;
constexpr uint32_t FOOD   = 0xA86B44;

// RGB565: 200 x 146 x 2 = 58.400 bayt
alignas(4) static lv_color16_t pixels[
    CANVAS_W * CANVAS_H
];

struct CatState {
    lv_obj_t* canvas = nullptr;
    lv_timer_t* timer = nullptr;

    uint32_t tick = 0;
};

CatState cat;

// -------------------------
// RENK VE PIKSEL YARDIMCILARI
// -------------------------

lv_color16_t rgb565(uint32_t rgb)
{
    lv_color16_t color{};

    color.red =
        static_cast<uint16_t>(
            (rgb >> 19) & 0x1F
        );

    color.green =
        static_cast<uint16_t>(
            (rgb >> 10) & 0x3F
        );

    color.blue =
        static_cast<uint16_t>(
            (rgb >> 3) & 0x1F
        );

    return color;
}

void pixel(
    int x,
    int y,
    lv_color16_t color
)
{
    if (
        x < 0 || y < 0 ||
        x >= CANVAS_W ||
        y >= CANVAS_H
    ) {
        return;
    }

    pixels[y * CANVAS_W + x] = color;
}

void fill_rect(
    int x,
    int y,
    int w,
    int h,
    uint32_t rgb
)
{
    lv_color16_t color = rgb565(rgb);

    for (int py = y; py < y + h; ++py) {
        for (int px = x; px < x + w; ++px) {
            pixel(px, py, color);
        }
    }
}

void rounded_rect(
    int x,
    int y,
    int w,
    int h,
    int radius,
    uint32_t rgb
)
{
    lv_color16_t color = rgb565(rgb);

    int r = radius;

    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    for (int py = y; py < y + h; ++py) {
        for (int px = x; px < x + w; ++px) {

            int cx = px;
            int cy = py;

            if (px < x + r) {
                cx = x + r;
            } else if (px >= x + w - r) {
                cx = x + w - r - 1;
            }

            if (py < y + r) {
                cy = y + r;
            } else if (py >= y + h - r) {
                cy = y + h - r - 1;
            }

            int dx = px - cx;
            int dy = py - cy;

            if (dx * dx + dy * dy <= r * r) {
                pixel(px, py, color);
            }
        }
    }
}

void ellipse(
    int x,
    int y,
    int w,
    int h,
    uint32_t rgb
)
{
    lv_color16_t color = rgb565(rgb);

    const int64_t w2 =
        static_cast<int64_t>(w) * w;

    const int64_t h2 =
        static_cast<int64_t>(h) * h;

    for (int py = y; py < y + h; ++py) {
        for (int px = x; px < x + w; ++px) {

            const int64_t dx =
                2LL * (px - x) + 1 - w;

            const int64_t dy =
                2LL * (py - y) + 1 - h;

            if (
                dx * dx * h2 +
                dy * dy * w2 <= w2 * h2
            ) {
                pixel(px, py, color);
            }
        }
    }
}

void clear_canvas()
{
    lv_color16_t white = rgb565(WHITE);

    for (int i = 0; i < CANVAS_W * CANVAS_H; ++i) {
        pixels[i] = white;
    }
}

// -------------------------
// GORUNUR KUYRUK
// -------------------------

void draw_tail(int pose)
{
    // Kuyruk govdenin arkasindan cikar.
    rounded_rect(
        51, 112,
        27, 12,
        4, BLACK
    );

    rounded_rect(
        43, 104,
        17, 16,
        5, BLACK
    );

    if (pose == 0) {
        rounded_rect(
            35, 91,
            16, 20,
            5, BLACK
        );

        rounded_rect(
            34, 76,
            14, 23,
            5, BLACK
        );
    }
    else if (pose == 1) {
        rounded_rect(
            32, 95,
            18, 18,
            5, BLACK
        );

        rounded_rect(
            28, 81,
            16, 22,
            5, BLACK
        );
    }
    else {
        rounded_rect(
            29, 101,
            19, 17,
            5, BLACK
        );

        rounded_rect(
            23, 90,
            17, 20,
            5, BLACK
        );
    }
}

// -------------------------
// OTURAN GOVDE
// -------------------------

void draw_body()
{
    // Govdenin yuvarlak ana silueti
    ellipse(
        65, 70,
        72, 58,
        BLACK
    );

    // Boyun ve omuzlar
    rounded_rect(
        75, 66,
        52, 33,
        12, BLACK
    );

    // Sol arka bacak
    rounded_rect(
        63, 101,
        33, 31,
        13, BLACK
    );

    // Sag arka bacak
    rounded_rect(
        108, 101,
        34, 31,
        13, BLACK
    );

    // On bacaklar: cok hafif ton farki
    rounded_rect(
        86, 96,
        18, 38,
        8, SHADE
    );

    rounded_rect(
        105, 96,
        18, 38,
        8, SHADE
    );

    // Sol on pati
    rounded_rect(
        84, 119,
        21, 19,
        6, BLACK
    );

    // Sag on pati
    rounded_rect(
        105, 119,
        21, 19,
        6, BLACK
    );

    // Patiler arasindaki ince ayrim
    fill_rect(
        103, 124,
        2, 14,
        SHADE
    );

    // Boyundaki lila tasma
    rounded_rect(
        78, 66,
        49, 7,
        2, PURPLE
    );
}

// -------------------------
// ORIJINAL KEDI YUZU
// -------------------------

void draw_head(bool eyes_closed)
{
    // Kafa ve govde ayni merkezde.
    constexpr int HEAD_X = 61;
    constexpr int HEAD_Y = 6;
    constexpr int SCALE = 5;

    static const char* pattern[12] = {
        "  ###      ###  ",
        "  ####    ####  ",
        "  ############  ",
        " ############## ",
        " ############## ",
        " ###YY####YY### ",
        " ###YY####YY### ",
        " ############## ",
        " ######PP###### ",
        " ############## ",
        "  ############  ",
        "   ##########   "
    };

    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 16; ++x) {

            char p = pattern[y][x];

            if (p == ' ') {
                continue;
            }

            uint32_t color =
                p == 'Y' ? GOLD :
                p == 'P' ? PINK :
                           BLACK;

            fill_rect(
                HEAD_X + x * SCALE,
                HEAD_Y + y * SCALE,
                SCALE,
                SCALE,
                color
            );
        }
    }

    if (eyes_closed) {
        // Altin gozler kisa sure kapanir.
        fill_rect(
            HEAD_X + 4 * SCALE,
            HEAD_Y + 5 * SCALE,
            2 * SCALE,
            2 * SCALE,
            BLACK
        );

        fill_rect(
            HEAD_X + 10 * SCALE,
            HEAD_Y + 5 * SCALE,
            2 * SCALE,
            2 * SCALE,
            BLACK
        );
    }
}

// -------------------------
// KEDIYI TAMAMEN CIZ
// -------------------------

void render_cat()
{
    clear_canvas();

    // Kedinin altindaki yumusak golge
    ellipse(
        49, 133,
        103, 11,
        LILAC
    );

    constexpr int tail_cycle[4] = {
        0, 1, 2, 1
    };

    int tail_pose = tail_cycle[
        (cat.tick / 4) % 4
    ];

    // Kuyruk ilk cizilir: govdenin arkasinda.
    draw_tail(tail_pose);

    draw_body();

    bool blink =
        cat.tick > 0 &&
        (cat.tick % 25 == 0);

    draw_head(blink);

    if (cat.canvas) {
        lv_obj_invalidate(cat.canvas);
    }
}

// -------------------------
// ANIMASYON
// -------------------------

void update_animation(lv_timer_t*)
{
    ++cat.tick;
    render_cat();
}

void cleanup(lv_event_t*)
{
    if (cat.timer) {
        lv_timer_delete(cat.timer);
        cat.timer = nullptr;
    }

    cat.canvas = nullptr;
}

// -------------------------
// LVGL ARAYUZ YARDIMCILARI
// -------------------------

lv_obj_t* box(
    lv_obj_t* parent,
    int x,
    int y,
    int w,
    int h,
    uint32_t color,
    int radius = 0
)
{
    lv_obj_t* obj = lv_obj_create(parent);

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    lv_obj_set_style_bg_color(
        obj, lv_color_hex(color), 0
    );

    lv_obj_set_style_bg_opa(
        obj, LV_OPA_COVER, 0
    );

    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_radius(obj, radius, 0);

    lv_obj_remove_flag(
        obj, LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_remove_flag(
        obj, LV_OBJ_FLAG_CLICKABLE
    );

    return obj;
}

lv_obj_t* text(
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

void go_tasks(lv_event_t*)
{
    lv_async_call(
        [](void*) {
            create_tasks_screen();
        },
        nullptr
    );
}

// -------------------------
// KEDI SAHNESI
// -------------------------

void create_scene(lv_obj_t* screen)
{
    lv_obj_t* stage = box(
        screen,
        10, 46,
        220, 150,
        WHITE, 14
    );

    lv_obj_t* canvas = lv_canvas_create(stage);

    lv_canvas_set_buffer(
        canvas,
        pixels,
        CANVAS_W,
        CANVAS_H,
        LV_COLOR_FORMAT_RGB565
    );

    lv_obj_set_pos(
        canvas,
        10, 2
    );

    lv_obj_remove_flag(
        canvas, LV_OBJ_FLAG_CLICKABLE
    );

    cat.canvas = canvas;

    lv_obj_add_event_cb(
        canvas,
        cleanup,
        LV_EVENT_DELETE,
        nullptr
    );

    render_cat();

    cat.timer = lv_timer_create(
        update_animation,
        160,
        nullptr
    );
}

// -------------------------
// MAMA VE OYUN ALANI
// -------------------------

void create_action_buttons(lv_obj_t* screen)
{
    // Simdilik pasif: animasyonlar sonraki asama.
    lv_obj_t* mama = box(
        screen,
        10, 203,
        105, 35,
        LILAC, 11
    );

    box(
        mama,
        13, 14,
        23, 9,
        PURPLE, 4
    );

    box(
        mama,
        16, 9,
        17, 8,
        FOOD, 4
    );

    text(
        mama,
        "Mama",
        46, 9,
        MUTED,
        &tr_font_14
    );

    lv_obj_t* oyun = box(
        screen,
        125, 203,
        105, 35,
        LILAC, 11
    );

    box(
        oyun,
        15, 10,
        17, 17,
        PURPLE,
        LV_RADIUS_CIRCLE
    );

    box(
        oyun,
        20, 13,
        5, 5,
        PINK, 3
    );

    text(
        oyun,
        "Oyun",
        44, 9,
        MUTED,
        &tr_font_14
    );
}

// -------------------------
// ALT MENÜ
// -------------------------

void nav_button(
    lv_obj_t* bar,
    int x,
    const char* symbol,
    const char* title,
    bool active,
    lv_event_cb_t callback
)
{
    lv_obj_t* button = box(
        bar,
        x, 0,
        80, 39,
        WHITE
    );

    lv_obj_add_flag(
        button, LV_OBJ_FLAG_CLICKABLE
    );

    const uint32_t color =
        active ? PURPLE : MUTED;

    lv_obj_t* icon = text(
        button,
        symbol,
        0, 2,
        color,
        LV_FONT_DEFAULT
    );

    lv_obj_align(
        icon,
        LV_ALIGN_TOP_MID,
        0, 2
    );

    lv_obj_t* caption = text(
        button,
        title,
        0, 20,
        color
    );

    lv_obj_set_width(caption, 80);

    lv_obj_set_style_text_align(
        caption,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    if (callback) {
        lv_obj_add_event_cb(
            button,
            callback,
            LV_EVENT_CLICKED,
            nullptr
        );
    }
}

void create_navigation(lv_obj_t* screen)
{
    lv_obj_t* bar = box(
        screen,
        0, 245,
        240, 39,
        WHITE
    );

    nav_button(
        bar,
        0,
        LV_SYMBOL_HOME,
        "Ana sayfa",
        false,
        go_home
    );

    nav_button(
        bar,
        80,
        LV_SYMBOL_LIST,
        "Görevler",
        false,
        go_tasks
    );

    nav_button(
        bar,
        160,
        LV_SYMBOL_IMAGE,
        "Kedim",
        true,
        nullptr
    );
}

} // namespace

// -------------------------
// KEDIM EKRANI
// -------------------------

void create_cat_screen()
{
    lv_obj_t* screen = lv_screen_active();

    // Onceki animasyon zamanlayicisi temizlenir.
    lv_obj_clean(screen);

    cat = CatState{};

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

    text(
        screen,
        "Kedim",
        18, 12,
        INK,
        &tr_font_14
    );

    text(
        screen,
        "Mırmır",
        173, 15,
        MUTED
    );

    create_scene(screen);
    create_action_buttons(screen);
    create_navigation(screen);
}

} // namespace cat::ui
