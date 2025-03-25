string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_compile_definitions(
    CYW43_HOST_NAME=\"pong\"
    CYW43_SPI_PIO_PREFERRED_PIO=0
)

set(STATIC_HTML_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(STATIC_HTML_FILENAME "static.html")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico-ws-server pico-ws-server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico-ap pico-ap)
add_library(lwipopts_provider INTERFACE)
target_include_directories(lwipopts_provider INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    pico_sync
    hardware_flash
    pico_cyw43_arch_lwip_poll
    pico_ws_server
    pico_ap
)

