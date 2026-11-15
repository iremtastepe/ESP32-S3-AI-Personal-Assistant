
idf_component_register(
    SRCS
        "src/HomeScreen.cpp"
        "src/TasksScreen.cpp"
        "src/CatAnimation.cpp"
    INCLUDE_DIRS "include"
    REQUIRES lvgl__lvgl
    PRIV_REQUIRES
        cat_assets
        cat_application
)
