file(GLOB_RECURSE C_SRC *.c)
file(GLOB_RECURSE CPP_SRC *.cpp)

idf_component_register(SRCS ${C_SRC} ${CPP_SRC}
                    INCLUDE_DIRS ".")