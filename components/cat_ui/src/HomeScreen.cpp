
#include "cat/ui/HomeScreen.hpp"
#include "cat/ui/TasksScreen.hpp"
#include "lvgl.h"
#include "tr_font.h"
#include <cstring>

namespace cat::ui {
namespace {

constexpr uint32_t BG = 0xF5F0E8;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t INK = 0x463843;
constexpr uint32_t MUTED = 0x987F97;
constexpr uint32_t PURPLE = 0x896990;
constexpr uint32_t LILAC = 0xEDE4ED;
constexpr uint32_t BADGE = 0xF1E7F3;

struct Task {
    const char* name;
    const char* time;
    bool done;
};

Task tasks[3] = {
    {"Ders çalış", "09:00", false},
    {"Kitap oku", "14:30", true},
    {"Odayı düzenle", nullptr, false}
};

int order[3] = {0, 1, 2};

lv_obj_t* rows[3] = {};
lv_obj_t* circles[3] = {};
lv_obj_t* checkmarks[3] = {};
lv_obj_t* task_labels[3] = {};

lv_obj_t* counter = nullptr;
lv_obj_t* mood_label = nullptr;
lv_obj_t* left_eye = nullptr;
lv_obj_t* right_eye = nullptr;

lv_color_t clr(uint32_t value)
{
    return lv_color_hex(value);
}

const lv_font_t* clock_font()
{
#if LV_FONT_MONTSERRAT_20
    return &lv_font_montserrat_20;
#elif LV_FONT_MONTSERRAT_18
    return &lv_font_montserrat_18;
#else
    return LV_FONT_DEFAULT;
#endif
}

lv_obj_t* rect(
    lv_obj_t* parent,
    int x,
    int y,
    int w,
    int h,
    uint32_t fill,
    int radius = 0
)
{
    lv_obj_t* obj = lv_obj_create(parent);

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, radius, 0);

    lv_obj_set_style_bg_color(obj, clr(fill), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

    lv_obj_remove_flag(
        obj, LV_OBJ_FLAG_SCROLLABLE
    );

    return obj;
}

lv_obj_t* label(
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
        obj, clr(color), 0
    );

    lv_obj_set_style_text_font(
        obj, font, 0
    );

    return obj;
}

// -------------------------
// BILGI PENCERELERI
// -------------------------

void dismiss(lv_event_t* event)
{
    lv_obj_t* overlay = static_cast<lv_obj_t*>(
        lv_event_get_user_data(event)
    );

    lv_obj_delete(overlay);
}

void popup(
    const char* heading,
    const char* body
)
{
    lv_obj_t* overlay = rect(
        lv_screen_active(),
        0, 0, 240, 284, INK
    );

    lv_obj_set_style_bg_opa(
        overlay, LV_OPA_50, 0
    );

    lv_obj_add_flag(
        overlay, LV_OBJ_FLAG_CLICKABLE
    );

    lv_obj_t* card = rect(
        overlay,
        18, 84, 204, 117,
        WHITE, 14
    );

    lv_obj_t* title = label(
        card,
        heading,
        12, 12,
        INK,
        &tr_font_14
    );

    lv_obj_set_size(title, 180, 40);

    lv_label_set_long_mode(
        title, LV_LABEL_LONG_MODE_DOTS
    );

    lv_obj_t* content = label(
        card,
        body,
        12, 55,
        MUTED
    );

    lv_obj_set_width(content, 180);

    lv_obj_t* close = rect(
        card,
        119, 85, 73, 25,
        PURPLE, 12
    );

    lv_obj_add_flag(
        close, LV_OBJ_FLAG_CLICKABLE
    );

    lv_obj_t* txt = label(
        close, "Kapat", 0, 0, WHITE
    );

    lv_obj_center(txt);

    lv_obj_add_event_cb(
        close,
        dismiss,
        LV_EVENT_CLICKED,
        overlay
    );
}

// -------------------------
// EKRAN GECISLERI
// -------------------------

void open_tasks(lv_event_t*)
{
    lv_async_call(
        [](void*) {
            create_tasks_screen();
        },
        nullptr
    );
}

void open_cat(lv_event_t*)
{
    popup(
        "Kedim",
        "Kedi ekranı sonraki aşamada eklenecek."
    );
}

void open_ai(lv_event_t*)
{
    popup(
        "AI Sohbet",
        "Sesli sohbet daha sonra bağlanacak."
    );
}

// -------------------------
// GOREV SISTEMI
// -------------------------

bool after(int a, int b)
{
    if (tasks[a].done != tasks[b].done) {
        return tasks[a].done;
    }

    if (!tasks[a].time) {
        return tasks[b].time != nullptr;
    }

    if (!tasks[b].time) {
        return false;
    }

    return std::strcmp(
        tasks[a].time,
        tasks[b].time
    ) > 0;
}

void refresh_tasks()
{
    for (int i = 0; i < 3; ++i) {
        order[i] = i;
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = i + 1; j < 3; ++j) {

            if (after(order[i], order[j])) {
                const int tmp = order[i];
                order[i] = order[j];
                order[j] = tmp;
            }
        }
    }

    int done_count = 0;

    for (const Task& task : tasks) {
        if (task.done) {
            ++done_count;
        }
    }

    lv_label_set_text_fmt(
        counter, "%d/3", done_count
    );

    for (int i = 0; i < 3; ++i) {

        const Task& task = tasks[order[i]];

        char display[128];

        if (task.time) {
            lv_snprintf(
                display,
                sizeof(display),
                "%s  %s",
                task.time,
                task.name
            );
        } else {
            lv_snprintf(
                display,
                sizeof(display),
                "%s",
                task.name
            );
        }

        lv_label_set_text(
            task_labels[i], display
        );

        lv_obj_set_style_text_color(
            task_labels[i],
            clr(task.done ? MUTED : INK),
            0
        );

        lv_obj_set_style_text_decor(
            task_labels[i],
            task.done
                ? LV_TEXT_DECOR_STRIKETHROUGH
                : LV_TEXT_DECOR_NONE,
            0
        );

        lv_obj_set_style_bg_color(
            circles[i],
            clr(task.done ? PURPLE : WHITE),
            0
        );

        lv_obj_set_style_border_color(
            circles[i],
            clr(task.done ? PURPLE : MUTED),
            0
        );

        if (task.done) {
            lv_obj_remove_flag(
                checkmarks[i],
                LV_OBJ_FLAG_HIDDEN
            );
        } else {
            lv_obj_add_flag(
                checkmarks[i],
                LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    lv_label_set_text(
        mood_label,
        done_count == 3
            ? "Çok mutlu!"
            : "Dinleniyor..."
    );
}

void toggle_task(lv_event_t* event)
{
    lv_obj_t* target = static_cast<lv_obj_t*>(
        lv_event_get_target(event)
    );

    for (int i = 0; i < 3; ++i) {

        if (rows[i] == target) {

            tasks[order[i]].done =
                !tasks[order[i]].done;

            refresh_tasks();
            break;
        }
    }
}

void task_detail(lv_event_t* event)
{
    lv_obj_t* target = static_cast<lv_obj_t*>(
        lv_event_get_target(event)
    );

    for (int i = 0; i < 3; ++i) {

        if (rows[i] == target) {

            const Task& task = tasks[order[i]];

            popup(
                task.name,
                task.time
                    ? task.time
                    : "Saat belirtilmedi"
            );

            break;
        }
    }
}

// -------------------------
// KEDI GOZ KIRPMA
// -------------------------

void blink(lv_timer_t* timer)
{
    static bool closed = false;

    closed = !closed;

    const uint32_t eye_color = closed
        ? 0x242128
        : 0xE7B955;

    lv_obj_set_style_bg_color(
        left_eye,
        clr(eye_color),
        0
    );

    lv_obj_set_style_bg_color(
        right_eye,
        clr(eye_color),
        0
    );

    lv_timer_set_period(
        timer,
        closed ? 160 : 3000
    );
}

// -------------------------
// ANA KARTTAKI KEDI KAFASI
// -------------------------

void create_cat(lv_obj_t* parent)
{
    lv_obj_t* canvas = rect(
        parent,
        14, 2,
        56, 42,
        LILAC
    );

    static const char* pattern[] = {
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

    constexpr int pixel_size = 3;
    constexpr int offset_x = 4;
    constexpr int offset_y = 3;

    for (int y = 0; y < 12; ++y) {

        for (int x = 0; x < 16;) {

            const char p = pattern[y][x];

            if (p == ' ') {
                ++x;
                continue;
            }

            int end = x + 1;

            while (
                end < 16 &&
                pattern[y][end] == p
            ) {
                ++end;
            }

            const uint32_t fill =
                p == 'Y' ? 0xE7B955 :
                p == 'P' ? 0xD78DAF :
                           0x242128;

            rect(
                canvas,
                offset_x + x * pixel_size,
                offset_y + y * pixel_size,
                (end - x) * pixel_size,
                pixel_size,
                fill
            );

            x = end;
        }
    }

    left_eye = rect(
        canvas,
        offset_x + 4 * pixel_size,
        offset_y + 5 * pixel_size,
        2 * pixel_size,
        2 * pixel_size,
        0xE7B955
    );

    right_eye = rect(
        canvas,
        offset_x + 10 * pixel_size,
        offset_y + 5 * pixel_size,
        2 * pixel_size,
        2 * pixel_size,
        0xE7B955
    );
lv_timer_t* timer = lv_timer_create(
    blink,
    3000,
    nullptr
);

lv_obj_add_event_cb(
    canvas,
    [](lv_event_t* event) {
        lv_timer_t* timer = static_cast<lv_timer_t*>(
            lv_event_get_user_data(event)
        );

        if (timer != nullptr) {
            lv_timer_delete(timer);
        }

        left_eye = nullptr;
        right_eye = nullptr;
    },
    LV_EVENT_DELETE,
    timer
);
}

// -------------------------
// GOREV KARTI
// -------------------------

void create_tasks_card(lv_obj_t* screen)
{
    lv_obj_t* card = rect(
        screen,
        10, 69,
        220, 122,
        WHITE, 16
    );

    label(
        card,
        "Bugünkü görevler",
        13, 9,
        INK,
        &tr_font_14
    );

    lv_obj_t* badge = rect(
        card,
        177, 9,
        32, 20,
        BADGE, 10
    );

    counter = label(
        badge, "1/3", 0, 0, PURPLE
    );

    lv_obj_center(counter);

    for (int i = 0; i < 3; ++i) {

        rows[i] = rect(
            card,
            12,
            34 + i * 23,
            197,
            22,
            WHITE
        );

        lv_obj_add_flag(
            rows[i],
            LV_OBJ_FLAG_CLICKABLE
        );

        lv_obj_add_event_cb(
            rows[i],
            toggle_task,
            LV_EVENT_SHORT_CLICKED,
            nullptr
        );

        lv_obj_add_event_cb(
            rows[i],
            task_detail,
            LV_EVENT_LONG_PRESSED,
            nullptr
        );

        circles[i] = rect(
            rows[i],
            1, 3,
            16, 16,
            WHITE,
            LV_RADIUS_CIRCLE
        );

        lv_obj_set_style_border_width(
            circles[i], 2, 0
        );

        lv_obj_set_style_border_color(
            circles[i], clr(MUTED), 0
        );

        checkmarks[i] = label(
            circles[i],
            LV_SYMBOL_OK,
            0, 0,
            WHITE,
            LV_FONT_DEFAULT
        );

        lv_obj_center(checkmarks[i]);

        task_labels[i] = label(
            rows[i],
            "",
            25, 2,
            INK
        );

        lv_obj_set_size(
            task_labels[i],
            169, 19
        );

        lv_label_set_long_mode(
            task_labels[i],
            LV_LABEL_LONG_MODE_DOTS
        );
    }

    lv_obj_t* more = rect(
        card,
        13, 102,
        175, 17,
        WHITE
    );

    lv_obj_add_flag(
        more,
        LV_OBJ_FLAG_CLICKABLE
    );

    label(
        more,
        "Tüm görevleri gör >",
        0, 0,
        PURPLE
    );

    lv_obj_add_event_cb(
        more,
        open_tasks,
        LV_EVENT_CLICKED,
        nullptr
    );
}

// -------------------------
// KEDI VE MIKROFON KARTI
// -------------------------

void create_cat_card(lv_obj_t* screen)
{
    lv_obj_t* card = rect(
        screen,
        10, 196,
        220, 46,
        LILAC, 14
    );

    create_cat(card);

    label(
        card,
        "Mırmır",
        86, 8,
        INK,
        &tr_font_14
    );

    mood_label = label(
        card,
        "Dinleniyor...",
        86, 27,
        MUTED
    );

    lv_obj_t* mic = rect(
        card,
        177, 5,
        37, 37,
        PURPLE,
        LV_RADIUS_CIRCLE
    );

    lv_obj_add_flag(
        mic,
        LV_OBJ_FLAG_CLICKABLE
    );

    rect(mic, 15, 8, 7, 16, WHITE, 4);
    rect(mic, 18, 24, 2, 6, WHITE);
    rect(mic, 12, 30, 14, 2, WHITE, 1);
    rect(mic, 11, 20, 2, 6, WHITE, 1);
    rect(mic, 25, 20, 2, 6, WHITE, 1);

    lv_obj_add_event_cb(
        mic,
        open_ai,
        LV_EVENT_CLICKED,
        nullptr
    );
}

// -------------------------
// ALT MENU KEDI IKONU
// -------------------------

void create_nav_cat_icon(
    lv_obj_t* parent,
    uint32_t icon_color
)
{
    auto pixel = [&](int x, int y, int w, int h) {

        lv_obj_t* part = rect(
            parent,
            x, y, w, h,
            icon_color
        );

        lv_obj_remove_flag(
            part,
            LV_OBJ_FLAG_CLICKABLE
        );
    };

    // Sol kulak
    pixel(31, 3, 4, 8);
    pixel(35, 7, 4, 3);

    // Sag kulak
    pixel(45, 3, 4, 8);
    pixel(41, 7, 4, 3);

    // Kafa
    pixel(35, 9, 11, 2);
    pixel(31, 10, 3, 7);
    pixel(47, 10, 3, 7);
    pixel(34, 17, 13, 2);

    // Gozler
    pixel(36, 13, 2, 2);
    pixel(43, 13, 2, 2);

    // Burun
    pixel(40, 16, 2, 2);
}

// -------------------------
// ALT MENU BUTONLARI
// -------------------------

void nav_button(
    lv_obj_t* bar,
    int left,
    const char* icon_text,
    const char* caption,
    lv_event_cb_t callback,
    bool active
)
{
    lv_obj_t* btn = rect(
        bar,
        left, 0,
        80, 36,
        WHITE
    );

    lv_obj_add_flag(
        btn,
        LV_OBJ_FLAG_CLICKABLE
    );

    const uint32_t icon_color =
        active ? PURPLE : MUTED;

    if (std::strcmp(caption, "Kedim") == 0) {

        create_nav_cat_icon(
            btn,
            icon_color
        );

    } else {

        lv_obj_t* icon = label(
            btn,
            icon_text,
            0, 0,
            icon_color,
            LV_FONT_DEFAULT
        );

        lv_obj_align(
            icon,
            LV_ALIGN_TOP_MID,
            0, 2
        );
    }

    lv_obj_t* name = label(
        btn,
        caption,
        0, 20,
        icon_color
    );

    lv_obj_set_width(name, 76);

    lv_obj_set_style_text_align(
        name,
        LV_TEXT_ALIGN_CENTER,
        0
    );

    lv_obj_align(
        name,
        LV_ALIGN_TOP_MID,
        0, 20
    );

    if (callback) {
        lv_obj_add_event_cb(
            btn,
            callback,
            LV_EVENT_CLICKED,
            nullptr
        );
    }
}

void create_navigation(lv_obj_t* screen)
{
    lv_obj_t* bar = rect(
        screen,
        0, 244,
        240, 40,
        WHITE
    );

    nav_button(
        bar,
        0,
        LV_SYMBOL_HOME,
        "Ana sayfa",
        nullptr,
        true
    );

    nav_button(
        bar,
        80,
        LV_SYMBOL_LIST,
        "Görevler",
        open_tasks,
        false
    );

    nav_button(
        bar,
        160,
        "",
        "Kedim",
        open_cat,
        false
    );
}

} // namespace

// -------------------------
// ANA EKRAN
// -------------------------

void create_home_screen()
{
    lv_obj_t* screen = lv_screen_active();

    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(
        screen, clr(BG), 0
    );

    lv_obj_set_style_bg_opa(
        screen, LV_OPA_COVER, 0
    );

    lv_obj_remove_flag(
        screen,
        LV_OBJ_FLAG_SCROLLABLE
    );

    // Tarih
    label(
        screen,
        "10 Kasım, Salı",
        23, 15,
        MUTED
    );

    // Hava durumu
    rect(
        screen,
        184, 16,
        12, 12,
        0xFFC247,
        LV_RADIUS_CIRCLE
    );

    label(
        screen,
        "18°",
        203, 14,
        MUTED
    );

    // Saat
    label(
        screen,
        "14:12",
        23, 37,
        INK,
        clock_font()
    );

    // Kisa mesaj
    lv_obj_t* mood = label(
        screen,
        "Güzel bir gün!",
        146, 47,
        MUTED
    );

    lv_obj_set_width(mood, 86);

    lv_label_set_long_mode(
        mood,
        LV_LABEL_LONG_MODE_CLIP
    );

    // Ana ekran bolumleri
    create_tasks_card(screen);
    create_cat_card(screen);
    create_navigation(screen);

    refresh_tasks();
}

} // namespace cat::ui
