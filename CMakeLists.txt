#CMakeList.txt : CMake project for KJK_Engine_Sandbox, include source and define
cmake_minimum_required (VERSION 3.20)

#Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#Define project name
project ("KJK_Engine_Sandbox")

#Find required packages from vcpkg
find_package(SDL3 CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(onnxruntime CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

#Gather all source files
file(GLOB_RECURSE ENGINE_SOURCES "src/*.cpp" "src/*.h" "src/*.hpp")

#Setup C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Add sources to this project
add_executable(KJK_Engine_Sandbox ${ENGINE_SOURCES})

#Include Directories
target_include_directories(KJK_Engine_Sandbox PRIVATE "${PROJECT_SOURCE_DIR}/src")

#Link Libraries
target_link_libraries(KJK_Engine_Sandbox PRIVATE
    SDL3::SDL3
    EnTT::EnTT
    box2d::box2d
    glm::glm
    imgui::imgui
    glad::glad
    onnxruntime::onnxruntime
    cpr::cpr
)

#Copy DLLs to Output Directory if on Windows
if(WIN32)
    add_custom_command(TARGET KJK_Engine_Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:KJK_Engine_Sandbox>
        $<TARGET_FILE_DIR:KJK_Engine_Sandbox>
        COMMAND_EXPAND_LISTS
    )
endif()
