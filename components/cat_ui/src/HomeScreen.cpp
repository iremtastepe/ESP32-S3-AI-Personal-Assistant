
#include "cat/ui/HomeScreen.hpp"
#include "cat/ui/TasksScreen.hpp"
#include "cat/ui/CatScreen.hpp"
#include "cat/application/TaskStore.hpp"
#include "lvgl.h"
#include "tr_font.h"
#include <cstring>
#include <cstddef>

namespace cat::ui {
namespace {

constexpr uint32_t BG = 0xF5F0E8;
constexpr uint32_t WHITE = 0xFFFFFF;
constexpr uint32_t INK = 0x463843;
constexpr uint32_t MUTED = 0x987F97;
constexpr uint32_t PURPLE = 0x896990;
constexpr uint32_t LILAC = 0xEDE4ED;
constexpr uint32_t BADGE = 0xF1E7F3;

constexpr const char* DEMO_TODAY = "2026-11-10";
constexpr int MAX_VISIBLE = 3;

using cat::application::Task;
using cat::application::TaskStore;

int visible_indices[MAX_VISIBLE] = {-1, -1, -1};

lv_obj_t* rows[MAX_VISIBLE] = {};
lv_obj_t* circles[MAX_VISIBLE] = {};
lv_obj_t* checks[MAX_VISIBLE] = {};
lv_obj_t* names[MAX_VISIBLE] = {};

lv_obj_t* counter = nullptr;
lv_obj_t* mood_label = nullptr;
lv_obj_t* left_eye = nullptr;
lv_obj_t* right_eye = nullptr;

lv_color_t clr(uint32_t color)
{
    return lv_color_hex(color);
}

lv_obj_t* rect(
    lv_obj_t* parent,
    int x, int y,
    int width, int height,
    uint32_t color,
    int radius = 0
)
{
    lv_obj_t* obj = lv_obj_create(parent);

    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, width, height);
    lv_obj_set_style_bg_color(obj, clr(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_radius(obj, radius, 0);

    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_CLICKABLE);

    return obj;
}

lv_obj_t* label(
    lv_obj_t* parent,
    const char* value,
    int x, int y,
    uint32_t color,
    const lv_font_t* font = &tr_font_12
)
{
    lv_obj_t* obj = lv_label_create(parent);

    lv_label_set_text(obj, value);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_style_text_color(obj, clr(color), 0);
    lv_obj_set_style_text_font(obj, font, 0);

    return obj;
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

// -------------------------
// BILGI PENCERESI
// -------------------------

void dismiss(lv_event_t* event)
{
    auto* overlay = static_cast<lv_obj_t*>(
        lv_event_get_user_data(event)
    );

    lv_obj_delete(overlay);
}

void popup(const char* heading, const char* message)
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
        overlay, 18, 84, 204, 117, WHITE, 14
    );

    lv_obj_t* title = label(
        card, heading, 12, 12, INK, &tr_font_14
    );

    lv_obj_set_size(title, 180, 40);
    lv_label_set_long_mode(
        title, LV_LABEL_LONG_MODE_DOTS
    );

    lv_obj_t* body = label(
        card, message, 12, 55, MUTED
    );

    lv_obj_set_width(body, 180);

    lv_obj_t* close = rect(
        card, 119, 85, 73, 25, PURPLE, 12
    );

    lv_obj_add_flag(
        close, LV_OBJ_FLAG_CLICKABLE
    );

    lv_obj_center(
        label(close, "Kapat", 0, 0, WHITE)
    );

    lv_obj_add_event_cb(
        close, dismiss, LV_EVENT_CLICKED, overlay
    );
}

// -------------------------
// EKRAN GECISLERI
// -------------------------

void open_tasks(lv_event_t*)
{
    lv_async_call(
        [](void*) { create_tasks_screen(); },
        nullptr
    );
}

void open_cat(lv_event_t*)
{
    lv_async_call(
        [](void*) { create_cat_screen(); },
        nullptr
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

bool after(const Task& a, const Task& b)
{
    if (a.completed != b.completed) {
        return a.completed;
    }

    if (!a.time || !a.time[0]) {
        return b.time && b.time[0];
    }

    if (!b.time || !b.time[0]) {
        return false;
    }

    return std::strcmp(a.time, b.time) > 0;
}

void refresh_tasks()
{
    const auto& store = TaskStore::instance();
    const Task* tasks = store.tasks();

    int candidates[16];
    int count = 0;
    int done = 0;
    int total = 0;

    for (std::size_t i = 0; i < store.count(); ++i) {
        const Task& task = tasks[i];

        bool today = task.carried ||
            (
                task.date &&
                std::strcmp(task.date, DEMO_TODAY) <= 0
            );

        if (!today) {
            continue;
        }

        ++total;

        if (task.completed) {
            ++done;
        }

        if (count < 16) {
            candidates[count++] = static_cast<int>(i);
        }
    }

    for (int i = 0; i < count; ++i) {
        for (int j = i + 1; j < count; ++j) {
            if (after(
                tasks[candidates[i]],
                tasks[candidates[j]]
            )) {
                int temp = candidates[i];
                candidates[i] = candidates[j];
                candidates[j] = temp;
            }
        }
    }

    lv_label_set_text_fmt(
        counter, "%d/%d", done, total
    );

    for (int i = 0; i < MAX_VISIBLE; ++i) {
        visible_indices[i] =
            i < count ? candidates[i] : -1;

        if (visible_indices[i] < 0) {
            lv_obj_add_flag(
                rows[i], LV_OBJ_FLAG_HIDDEN
            );
            continue;
        }

        lv_obj_remove_flag(
            rows[i], LV_OBJ_FLAG_HIDDEN
        );

        const Task& task = tasks[visible_indices[i]];
        char display[160];

        if (task.time && task.time[0]) {
            lv_snprintf(
                display,
                sizeof(display),
                "%s  %s",
                task.time, task.title
            );
        } else {
            lv_snprintf(
                display,
                sizeof(display),
                "%s",
                task.title
            );
        }

        lv_label_set_text(names[i], display);

        lv_obj_set_style_text_color(
            names[i],
            clr(task.completed ? MUTED : INK),
            0
        );

        lv_obj_set_style_text_decor(
            names[i],
            task.completed
                ? LV_TEXT_DECOR_STRIKETHROUGH
                : LV_TEXT_DECOR_NONE,
            0
        );

        lv_obj_set_style_bg_color(
            circles[i],
            clr(task.completed ? PURPLE : WHITE),
            0
        );

        lv_obj_set_style_border_color(
            circles[i],
            clr(task.completed ? PURPLE : MUTED),
            0
        );

        if (task.completed) {
            lv_obj_remove_flag(
                checks[i], LV_OBJ_FLAG_HIDDEN
            );
        } else {
            lv_obj_add_flag(
                checks[i], LV_OBJ_FLAG_HIDDEN
            );
        }
    }

    lv_label_set_text(
        mood_label,
        (total > 0 && done == total)
            ? "Çok mutlu!"
            : "Dinleniyor..."
    );
}

void toggle_task(lv_event_t* event)
{
    auto* target = static_cast<lv_obj_t*>(
        lv_event_get_target(event)
    );

    for (int i = 0; i < MAX_VISIBLE; ++i) {
        if (
            rows[i] == target &&
            visible_indices[i] >= 0
        ) {
            const Task* tasks =
                TaskStore::instance().tasks();

            TaskStore::instance().toggle_completed(
                tasks[visible_indices[i]].id
            );

            refresh_tasks();
            return;
        }
    }
}

void task_detail(lv_event_t* event)
{
    auto* target = static_cast<lv_obj_t*>(
        lv_event_get_target(event)
    );

    for (int i = 0; i < MAX_VISIBLE; ++i) {
        if (
            rows[i] == target &&
            visible_indices[i] >= 0
        ) {
            const Task& task =
                TaskStore::instance()
                    .tasks()[visible_indices[i]];

            popup(
                task.title,
                task.time
                    ? task.time
                    : "Saat belirtilmedi"
            );

            return;
        }
    }
}

// -------------------------
// ANA SAYFADAKI KEDI AVATARI
// -------------------------

void blink(lv_timer_t* timer)
{
    static bool closed = false;

    if (!left_eye || !right_eye) {
        return;
    }

    closed = !closed;

    uint32_t color =
        closed ? 0x242128 : 0xE7B955;

    lv_obj_set_style_bg_color(
        left_eye, clr(color), 0
    );

    lv_obj_set_style_bg_color(
        right_eye, clr(color), 0
    );

    lv_timer_set_period(
        timer, closed ? 160 : 3000
    );
}

void create_cat_avatar(lv_obj_t* parent)
{
    lv_obj_t* canvas = rect(
        parent, 14, 2, 56, 42, LILAC
    );

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

    constexpr int pixel = 3;

    for (int y = 0; y < 12; ++y) {
        for (int x = 0; x < 16;) {
            char p = pattern[y][x];

            if (p == ' ') {
                ++x;
                continue;
            }

            int end = x + 1;

            while (
                end < 16 && pattern[y][end] == p
            ) {
                ++end;
            }

            uint32_t color =
                p == 'Y' ? 0xE7B955 :
                p == 'P' ? 0xD78DAF :
                           0x242128;

            rect(
                canvas,
                4 + x * pixel,
                3 + y * pixel,
                (end - x) * pixel,
                pixel,
                color
            );

            x = end;
        }
    }

    left_eye = rect(
        canvas,
        4 + 4 * pixel,
        3 + 5 * pixel,
        2 * pixel,
        2 * pixel,
        0xE7B955
    );

    right_eye = rect(
        canvas,
        4 + 10 * pixel,
        3 + 5 * pixel,
        2 * pixel,
        2 * pixel,
        0xE7B955
    );

    lv_timer_t* timer = lv_timer_create(
        blink, 3000, nullptr
    );

    lv_obj_add_event_cb(
        canvas,
        [](lv_event_t* event) {
            auto* timer = static_cast<lv_timer_t*>(
                lv_event_get_user_data(event)
            );

            if (timer) {
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
        screen, 10, 69, 220, 122, WHITE, 16
    );

    label(
        card,
        "Bugünkü görevler",
        13, 9, INK, &tr_font_14
    );

    lv_obj_t* badge = rect(
        card, 177, 9, 32, 20, BADGE, 10
    );

    counter = label(
        badge, "0/3", 0, 0, PURPLE
    );

    lv_obj_center(counter);

    for (int i = 0; i < MAX_VISIBLE; ++i) {
        rows[i] = rect(
            card,
            12, 34 + i * 23,
            197, 22, WHITE
        );

        lv_obj_add_flag(
            rows[i], LV_OBJ_FLAG_CLICKABLE
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
            rows[i], 1, 3, 16, 16,
            WHITE, LV_RADIUS_CIRCLE
        );

        lv_obj_set_style_border_width(
            circles[i], 2, 0
        );

        lv_obj_set_style_border_color(
            circles[i], clr(MUTED), 0
        );

        checks[i] = label(
            circles[i], LV_SYMBOL_OK,
            0, 0, WHITE, LV_FONT_DEFAULT
        );

        lv_obj_center(checks[i]);

        names[i] = label(
            rows[i], "", 25, 2, INK
        );

        lv_obj_set_size(names[i], 169, 19);

        lv_label_set_long_mode(
            names[i], LV_LABEL_LONG_MODE_DOTS
        );
    }

    lv_obj_t* more = rect(
        card, 13, 102, 175, 17, WHITE
    );

    lv_obj_add_flag(
        more, LV_OBJ_FLAG_CLICKABLE
    );

    label(
        more,
        "Tüm görevleri gör >",
        0, 0, PURPLE
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
        screen, 10, 196, 220, 46, LILAC, 14
    );

    create_cat_avatar(card);

    label(
        card, "Mırmır",
        86, 8, INK, &tr_font_14
    );

    mood_label = label(
        card, "Dinleniyor...",
        86, 27, MUTED
    );

    lv_obj_t* mic = rect(
        card, 177, 5, 37, 37,
        PURPLE, LV_RADIUS_CIRCLE
    );

    lv_obj_add_flag(
        mic, LV_OBJ_FLAG_CLICKABLE
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
// ALT MENU
// -------------------------

void create_cat_icon(
    lv_obj_t* parent,
    uint32_t color
)
{
    auto pixel = [&](int x, int y, int w, int h) {
        rect(parent, x, y, w, h, color);
    };

    pixel(31, 3, 4, 8);
    pixel(35, 7, 4, 3);

    pixel(45, 3, 4, 8);
    pixel(41, 7, 4, 3);

    pixel(35, 9, 11, 2);
    pixel(31, 10, 3, 7);
    pixel(47, 10, 3, 7);
    pixel(34, 17, 13, 2);

    pixel(36, 13, 2, 2);
    pixel(43, 13, 2, 2);
    pixel(40, 16, 2, 2);
}

void nav_button(
    lv_obj_t* bar,
    int x,
    const char* symbol,
    const char* title,
    lv_event_cb_t callback,
    bool active
)
{
    lv_obj_t* button = rect(
        bar, x, 0, 80, 36, WHITE
    );

    lv_obj_add_flag(
        button, LV_OBJ_FLAG_CLICKABLE
    );

    uint32_t color =
        active ? PURPLE : MUTED;

    if (std::strcmp(title, "Kedim") == 0) {
        create_cat_icon(button, color);
    } else {
        lv_obj_t* icon = label(
            button, symbol,
            0, 2, color, LV_FONT_DEFAULT
        );

        lv_obj_align(
            icon, LV_ALIGN_TOP_MID, 0, 2
        );
    }

    lv_obj_t* name = label(
        button, title, 0, 20, color
    );

    lv_obj_set_width(name, 76);

    lv_obj_set_style_text_align(
        name, LV_TEXT_ALIGN_CENTER, 0
    );

    lv_obj_align(
        name, LV_ALIGN_TOP_MID, 0, 20
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
    lv_obj_t* bar = rect(
        screen, 0, 244, 240, 40, WHITE
    );

    nav_button(
        bar, 0,
        LV_SYMBOL_HOME,
        "Ana sayfa",
        nullptr, true
    );

    nav_button(
        bar, 80,
        LV_SYMBOL_LIST,
        "Görevler",
        open_tasks, false
    );

    nav_button(
        bar, 160,
        "",
        "Kedim",
        open_cat, false
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
        screen, LV_OBJ_FLAG_SCROLLABLE
    );

    label(
        screen,
        "10 Kasım, Salı",
        23, 15, MUTED
    );

    rect(
        screen,
        184, 16, 12, 12,
        0xFFC247, LV_RADIUS_CIRCLE
    );

    label(
        screen,
        "18°",
        203, 14, MUTED
    );

    label(
        screen,
        "14:12",
        23, 37, INK, clock_font()
    );

    lv_obj_t* mood = label(
        screen,
        "Güzel bir gün!",
        146, 47, MUTED
    );

    lv_obj_set_width(mood, 86);

    lv_label_set_long_mode(
        mood, LV_LABEL_LONG_MODE_CLIP
    );

    create_tasks_card(screen);
    create_cat_card(screen);
    create_navigation(screen);

    refresh_tasks();
}

} // namespace cat::ui
