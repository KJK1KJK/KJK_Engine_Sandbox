#Define the project name
project(Playground)

#Gather all source files
file(GLOB_RECURSE PLAYGROUND_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.hpp")
add_executable(PlaygroundApp ${PLAYGROUND_SOURCES})

#Add a macro to disable the default entry point
target_compile_definitions(PlaygroundApp PRIVATE TEST_NO_ENTRYPOINT)

#Link to the Engine
target_link_libraries(PlaygroundApp PRIVATE KJK::KJK)

#Copy DLLs if on Windows
if(WIN32)
    add_custom_command(TARGET PlaygroundApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:PlaygroundApp>
        $<TARGET_FILE_DIR:PlaygroundApp>
        COMMAND_EXPAND_LISTS
    )
endif()