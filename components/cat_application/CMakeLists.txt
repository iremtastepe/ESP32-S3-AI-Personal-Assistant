
idf_component_register(
    SRCS
        "src/TaskService.cpp"
        "src/TaskStore.cpp"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES cat_domain
)
