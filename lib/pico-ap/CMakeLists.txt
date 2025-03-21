add_library(pico_ap
        #picow_access_point.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        )

target_include_directories(pico_ap PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        )

target_link_libraries(pico_ap
        #pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_nosys
        pico_stdlib
        pico_cyw43_driver
        lwipopts_provider
        )

#pico_add_extra_outputs(pico_ap)

