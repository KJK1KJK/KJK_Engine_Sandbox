#Define name of the project
project(Sandbox)

#Gather all source files
file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.hpp")
add_executable(SandboxApp ${SANDBOX_SOURCES})

#Link against the Engine library
target_link_libraries(SandboxApp PRIVATE KJK::KJK)

#Copy DLLs to Output Directory if on Windows
if(WIN32)
    add_custom_command(TARGET SandboxApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:SandboxApp>
        $<TARGET_FILE_DIR:SandboxApp>
        COMMAND_EXPAND_LISTS
    )
endif()