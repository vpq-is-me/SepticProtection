set(srcs "main.c"
        "board.c")

idf_component_register(SRCS "loop.c" "${srcs}"
                    INCLUDE_DIRS  ".")
