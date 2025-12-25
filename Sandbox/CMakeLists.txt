#Define name of the project
project(Sandbox)

#Gather all source files
file(GLOB_RECURSE SANDBOX_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.h" "src/*.hpp")
add_executable(SandboxApp ${SANDBOX_SOURCES})

#Reuse precompile headers from the Engine
target_precompile_headers(SandboxApp REUSE_FROM Engine)

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

#Add the license file to the generated output directory
if(MSVC)
    #Define the output file
    set(LICENSE_FILE "${CMAKE_BINARY_DIR}/out/install/x64-debug/bin/ThirdPartyNotices.txt")

    #Scrape all licenses and put them into the defined file
    set(PS_CMD "Get-ChildItem -Path '${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/share/*/copyright' | ForEach-Object { Add-Content -Path '${LICENSE_FILE}' -Value \"`n`n--- LICENSE FOR $($_.Directory.Name) ---`n\"; Get-Content $_ | Add-Content -Path '${LICENSE_FILE}' }")
endif()