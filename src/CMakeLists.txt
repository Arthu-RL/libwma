file(GLOB_RECURSE WMA_HEADERS
    CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/include/*.hpp
)

file(GLOB_RECURSE WMA_SOURCES
    CONFIGURE_DEPENDS
    *.cpp
)

add_library(wma STATIC)

add_library(wma::wma ALIAS wma)

target_sources(wma
    PRIVATE
        ${WMA_SOURCES}

    PUBLIC
        FILE_SET public_headers
        TYPE HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}/include
        FILES
            ${WMA_HEADERS}
)

target_include_directories(wma
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(wma
    PUBLIC
        cxx_std_23
)

target_compile_options(wma PRIVATE
    $<$<CONFIG:Debug>:
        -Wall
        -Wextra
        -g
        -O0
    >

    $<$<CONFIG:Release>:-O3>

    $<$<AND:$<CONFIG:Release>,$<BOOL:${WMA_NATIVE_OPTIMIZE}>>:
        -march=native
        -mtune=native
    >
)

target_link_libraries(wma PUBLIC ink::ink)

# Backend compile definitions and link libraries
if(WMA_ENABLE_SDL)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_SDL)
    target_link_libraries(wma PRIVATE SDL3::SDL3)
endif()

if(WMA_ENABLE_GLFW)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_GLFW)
    target_link_libraries(wma PRIVATE glfw)
endif()

if(WMA_ENABLE_X11)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_X11)
    target_link_libraries(wma PRIVATE X11::X11)
endif()

if(WMA_ENABLE_WAYLAND)
    target_compile_definitions(wma PRIVATE WMA_ENABLE_WAYLAND)
    target_link_libraries(wma PRIVATE PkgConfig::WAYLAND)
endif()
