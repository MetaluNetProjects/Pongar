string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_compile_definitions(
    CYW43_HOST_NAME=\"pong\"
    CYW43_SPI_PIO_PREFERRED_PIO=0
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico_https pico-https)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../lib/pico-ap pico-ap)

add_library(lwipopts_provider INTERFACE)
target_include_directories(lwipopts_provider INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    PICO_STACK_SIZE=0x8000 # 32K
    PICO_STDIO_STACK_BUFFER_SIZE=1024
)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    pico_stdlib
    pico_sync
    hardware_flash
    pico_cyw43_arch_lwip_poll
    pico_ap
    pico_http
    pico_tls
    pico_logger
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/config)
#target_include_directories(pico_ap PUBLIC ${CMAKE_CURRENT_LIST_DIR}/config)

