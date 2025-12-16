#Define name of the project
project(Sandbox)

#Define Executable
add_executable(Playground "src/main.cpp")

#Link against the Engine library
target_link_libraries(Playground PRIVATE KJK::KJK)

#Copy DLLs to Output Directory if on Windows
if(WIN32)
    add_custom_command(TARGET Playground POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:Playground>
        $<TARGET_FILE_DIR:Playground>
        COMMAND_EXPAND_LISTS
    )
endif()
