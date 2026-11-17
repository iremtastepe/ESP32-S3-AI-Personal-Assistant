
#include "cat/ui/TasksScreen.hpp"
#include "cat/ui/HomeScreen.hpp"
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
constexpr uint32_t TRACK = 0xDCCBDD;

constexpr const char* DEMO_TODAY = "2026-11-10";
constexpr const char* DEMO_TOMORROW = "2026-11-11";

using cat::application::Task;
using cat::application::TaskStore;

lv_obj_t* box(
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

lv_obj_t* txt(
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
    lv_obj_set_style_text_color(
        obj, lv_color_hex(color), 0
    );
    lv_obj_set_style_text_font(obj, font, 0);

    return obj;
}

void go_home(lv_event_t*)
{
    lv_async_call(
        [](void*) { create_home_screen(); },
        nullptr
    );
}

void go_cat(lv_event_t*)
{
    lv_async_call(
        [](void*) { create_cat_screen(); },
        nullptr
    );
}

// -------------------------
// GOREV SISTEMI
// -------------------------

int group_of(const Task& task)
{
    if (
        task.carried ||
        !task.date ||
        std::strcmp(task.date, DEMO_TODAY) <= 0
    ) {
        return 0;
    }

    if (
        std::strcmp(task.date, DEMO_TOMORROW) == 0
    ) {
        return 1;
    }

    return 2;
}

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

void toggle_task(lv_event_t* event)
{
    auto* task = static_cast<const Task*>(
        lv_event_get_user_data(event)
    );

    if (!task) {
        return;
    }

    if (TaskStore::instance().toggle_completed(task->id)) {
        lv_async_call(
            [](void*) { create_tasks_screen(); },
            nullptr
        );
    }
}

// -------------------------
// GOREV SATIRI
// -------------------------

void add_task(
    lv_obj_t* parent,
    int y,
    const Task& task
)
{
    lv_obj_t* row = box(
        parent, 2, y, 216, 30, WHITE, 10
    );

    lv_obj_add_flag(
        row, LV_OBJ_FLAG_CLICKABLE
    );

    lv_obj_add_event_cb(
        row,
        toggle_task,
        LV_EVENT_SHORT_CLICKED,
        const_cast<Task*>(&task)
    );

    lv_obj_t* circle = box(
        row,
        8, 7, 16, 16,
        task.completed ? PURPLE : WHITE,
        LV_RADIUS_CIRCLE
    );

    lv_obj_set_style_border_width(
        circle,
        task.completed ? 0 : 2,
        0
    );

    lv_obj_set_style_border_color(
        circle, lv_color_hex(TRACK), 0
    );

    if (task.completed) {
        lv_obj_center(
            txt(
                circle, LV_SYMBOL_OK,
                0, 0, WHITE, LV_FONT_DEFAULT
            )
        );
    }

    char display[160];

    if (task.time && task.time[0]) {
        lv_snprintf(
            display,
            sizeof(display),
            "%s  %s",
            task.time,
            task.title
        );
    } else {
        lv_snprintf(
            display,
            sizeof(display),
            "%s",
            task.title
        );
    }

    lv_obj_t* title = txt(
        row,
        display,
        32, 8,
        task.completed ? MUTED : INK
    );

    lv_obj_set_width(
        title,
        task.carried ? 110 : 173
    );

    lv_label_set_long_mode(
        title, LV_LABEL_LONG_MODE_DOTS
    );

    if (task.completed) {
        lv_obj_set_style_text_decor(
            title,
            LV_TEXT_DECOR_STRIKETHROUGH,
            0
        );
    }

    if (task.carried) {
        txt(
            row,
            "↻ Devreden",
            143, 8, PURPLE
        );
    }
}

// -------------------------
// BASLIK VE ILERLEME
// -------------------------

void create_header(lv_obj_t* screen)
{
    txt(
        screen,
        "10 Kasım, Salı",
        20, 10, MUTED
    );

    txt(
        screen,
        "Görevlerim",
        18, 32, INK, &tr_font_14
    );

    lv_obj_t* add = box(
        screen,
        197, 29, 32, 32,
        PURPLE, LV_RADIUS_CIRCLE
    );

    lv_obj_center(
        txt(
            add, "+",
            0, 0, WHITE, &tr_font_14
        )
    );
}

void create_progress(lv_obj_t* screen)
{
    const auto& store = TaskStore::instance();

    int total = 0;
    int done = 0;

    for (std::size_t i = 0; i < store.count(); ++i) {
        const Task& task = store.tasks()[i];

        if (group_of(task) != 0) {
            continue;
        }

        ++total;

        if (task.completed) {
            ++done;
        }
    }

    lv_obj_t* card = box(
        screen, 10, 67, 220, 45, LILAC, 13
    );

    txt(
        card,
        "Günlük ilerleme",
        11, 7, INK
    );

    char progress[24];

    lv_snprintf(
        progress,
        sizeof(progress),
        "%d/%d",
        done,
        total
    );

    txt(
        card,
        progress,
        176, 7, PURPLE
    );

    box(
        card, 11, 29, 198, 8, TRACK, 5
    );

    int width =
        total > 0 ? 198 * done / total : 0;

    if (width > 0) {
        box(
            card,
            11, 29, width, 8, PURPLE, 5
        );
    }
}

// -------------------------
// GOREV LISTESI
// -------------------------

void create_task_list(lv_obj_t* screen)
{
    lv_obj_t* list = box(
        screen,
        10, 118, 220, 124, BG
    );

    lv_obj_add_flag(
        list, LV_OBJ_FLAG_SCROLLABLE
    );

    lv_obj_set_scroll_dir(
        list, LV_DIR_VER
    );

    lv_obj_set_scrollbar_mode(
        list, LV_SCROLLBAR_MODE_AUTO
    );

    const auto& store = TaskStore::instance();
    const Task* tasks = store.tasks();

    const char* headings[3] = {
        "Bugün", "Yarın", "Yaklaşan"
    };

    int y = 2;

    for (int group = 0; group < 3; ++group) {
        int indices[16];
        int count = 0;

        for (std::size_t i = 0; i < store.count(); ++i) {
            if (
                group_of(tasks[i]) == group &&
                count < 16
            ) {
                indices[count++] = static_cast<int>(i);
            }
        }

        if (!count) {
            continue;
        }

        for (int i = 0; i < count; ++i) {
            for (int j = i + 1; j < count; ++j) {
                if (after(
                    tasks[indices[i]],
                    tasks[indices[j]]
                )) {
                    int temp = indices[i];
                    indices[i] = indices[j];
                    indices[j] = temp;
                }
            }
        }

        txt(
            list,
            headings[group],
            3, y,
            INK, &tr_font_14
        );

        y += 21;

        for (int i = 0; i < count; ++i) {
            add_task(
                list, y, tasks[indices[i]]
            );

            y += 34;
        }

        y += 6;
    }

    lv_obj_t* history = box(
        list, 2, y, 216, 30, LILAC, 10
    );

    txt(
        history,
        "Geçmiş görevleri gör  >",
        12, 8, PURPLE
    );
}

// -------------------------
// ALT MENU
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
        bar, x, 0, 80, 39, WHITE
    );

    lv_obj_add_flag(
        button, LV_OBJ_FLAG_CLICKABLE
    );

    uint32_t color =
        active ? PURPLE : MUTED;

    lv_obj_t* icon = txt(
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

    lv_obj_t* caption = txt(
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
        screen, 0, 245, 240, 39, WHITE
    );

    nav_button(
        bar, 0,
        LV_SYMBOL_HOME,
        "Ana sayfa",
        false,
        go_home
    );

    nav_button(
        bar, 80,
        LV_SYMBOL_LIST,
        "Görevler",
        true,
        nullptr
    );

    nav_button(
        bar, 160,
        LV_SYMBOL_IMAGE,
        "Kedim",
        false,
        go_cat
    );
}

} // namespace

// -------------------------
// GOREVLER EKRANI
// -------------------------

void create_tasks_screen()
{
    lv_obj_t* screen = lv_screen_active();

    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(
        screen, lv_color_hex(BG), 0
    );

    lv_obj_set_style_bg_opa(
        screen, LV_OPA_COVER, 0
    );

    lv_obj_remove_flag(
        screen, LV_OBJ_FLAG_SCROLLABLE
    );

    create_header(screen);
    create_progress(screen);
    create_task_list(screen);
    create_navigation(screen);
}

} // namespace cat::ui
