cmake_minimum_required(VERSION 3.16)
project(VMHumanizer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS UNICODE _UNICODE)
else()
    add_compile_options(-Wall -Wextra -municode)
    add_compile_definitions(UNICODE _UNICODE)
endif()

set(COMMON_SOURCES
    src/common/utils.cpp
    src/common/profile_loader.cpp
)

set(WINDOWS_SOURCES
    src/windows/winapi_wrapper.cpp
)

set(TIER1_SOURCES
    src/mouse_behavior.cpp
    src/keystroke_emulator.cpp
    src/registry_mru_engine.cpp
    src/event_log_injector.cpp
)

set(TIER2_SOURCES
    src/activity_engine.cpp
    src/prefetch_generator.cpp
    src/lnk_file_generator.cpp
    src/profile_engine.cpp
    src/network_simulator.cpp
    src/common/config_manager.cpp
    src/common/timer_utils.cpp
)

set(TIER4_SOURCES
    src/logger.cpp
    src/telemetry_pipeline.cpp
    src/debug_ui.cpp
    src/anomaly_detector.cpp
    src/anomaly_corrector.cpp
    src/common/metrics.cpp
    src/common/json_utils.cpp
)

set(ALL_SOURCES
    src/main.cpp
    ${COMMON_SOURCES}
    ${WINDOWS_SOURCES}
    ${TIER1_SOURCES}
)

# Check which tier sources exist and add them
foreach(src ${TIER2_SOURCES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND ALL_SOURCES ${src})
    endif()
endforeach()

foreach(src ${TIER4_SOURCES})
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
        list(APPEND ALL_SOURCES ${src})
    endif()
endforeach()

add_executable(VMHumanizer ${ALL_SOURCES})

target_include_directories(VMHumanizer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(VMHumanizer PRIVATE
    advapi32
    user32
    urlmon
    shell32
    ole32
    ws2_32
    dnsapi
    iphlpapi
)

# Copy data directory to build output
add_custom_command(TARGET VMHumanizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:VMHumanizer>/data"
)

install(TARGETS VMHumanizer DESTINATION bin)
install(DIRECTORY data/ DESTINATION bin/data)
