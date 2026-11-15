idf_component_register(
    SRCS
        "src/cat_sleep.c"
        "src/cat_wake_frames.c"
    INCLUDE_DIRS "include"
    REQUIRES lvgl__lvgl
)