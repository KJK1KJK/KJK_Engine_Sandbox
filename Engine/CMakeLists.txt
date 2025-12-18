#Define project name
project(Engine)

#Gather all source files
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.hpp")

#Define as a static library
add_library(Engine STATIC ${ENGINE_SOURCES})
add_library(KJK::KJK ALIAS Engine)

#Include Directories
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

#Link Libraries
target_link_libraries(Engine PUBLIC
    SDL3::SDL3
    EnTT::EnTT
    box2d::box2d
    glm::glm
    imgui::imgui
    glad::glad
    onnxruntime::onnxruntime
    cpr::cpr
    unofficial::enet::enet
    spdlog::spdlog
)