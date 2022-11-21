# idf_component_register(SRCS "lcd_st7735s.c"
#                     INCLUDE_DIRS ".")


# 
file(GLOB_RECURSE SOURCES ./st7789/*.c)

set(include_dirs 
    ./st7789/
    )
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${include_dirs})