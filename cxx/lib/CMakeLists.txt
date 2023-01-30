
#=================== IMGUI ===================

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
add_library(IMGUI STATIC)

target_sources( IMGUI
        PRIVATE
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp

        PRIVATE
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        )

target_include_directories( IMGUI
        PUBLIC ${IMGUI_DIR}
        PUBLIC ${IMGUI_DIR}/backends
        PUBLIC ${SDL2_DIR}/include
        )

find_package(OpenGL REQUIRED)
target_link_libraries(IMGUI PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(IMGUI PUBLIC SDL2main SDL2 ${CMAKE_DL_LIBS})


