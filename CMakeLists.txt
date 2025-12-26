#CMakeList.txt : CMake project for KJK_Engine_Sandbox, include source and define
cmake_minimum_required (VERSION 3.20)

#Define name of the project
project(KJK_Engine)

#Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#Setup platform macros
if(WIN32)
    add_compile_definitions(KJK_PLATFORM_WINDOWS)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(KJK_PLATFORM_LINUX)
elseif(APPLE)
    add_compile_definitions(KJK_PLATFORM_MACOS)
endif()

#Setup configuration macros
add_compile_definitions(
    $<$<CONFIG:Debug>:KJK_DEBUG>
    $<$<CONFIG:Release>:KJK_RELEASE>
    $<$<CONFIG:RelWithDebInfo>:KJK_DIST>
    $<$<CONFIG:MinSizeRel>:KJK_MINSIZE>
    $<$<PLATFORM_ID:Windows>:KJK_PLATFORM_WINDOWS>
    $<$<PLATFORM_ID:Darwin>:KJK_PLATFORM_MACOS>
    $<$<PLATFORM_ID:Linux>:KJK_PLATFORM_LINUX>
)

#Setup C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Find required packages from vcpkg
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(onnxruntime CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(unofficial-enet CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

#Add subdirectories for Engine and Sandbox
add_subdirectory(Engine)
add_subdirectory(Sandbox)
add_subdirectory(Playground)