
# aux_source_directory(src/ UVGUI_srcs)
# set(UVGUI_srcs2 "ug_port.c")
# set(UVGUI_include "include" ".")
# idf_component_register(SRCS src "${UVGUI_srcs2}"
#                        INCLUDE_DIRS "${UVGUI_include}")




file(GLOB_RECURSE SOURCES 
        core/*.c
        draw/*.c
        widget/*.c
        user/*.c
        font/*.c
        )

set(include_dirs 
    core
    draw
    widget
    user
    font
    )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs}
                    REQUIRES lcd)
