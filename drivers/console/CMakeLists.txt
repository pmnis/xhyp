
add_executable(Console 
        lib/_udivsi3.S
        lib/stdlib.c
        lib/fifo.c
        domain.c
        serial.c
        main.S)

target_compile_definitions(Console PRIVATE 
        __KERNEL__
        TEXT_BASE=0x00200000)


set_target_properties(Console PROPERTIES LINK_FLAGS "-nostdlib -Bstatic -T ${CMAKE_CURRENT_SOURCE_DIR}/domain.ld")

target_include_directories(Console PRIVATE include/)

set_target_properties(Console PROPERTIES POSITION_INDEPENDENT_CODE OFF)

set_target_properties(Console PROPERTIES SUFFIX ".dom")

set_target_properties(Console PROPERTIES OUTPUT_NAME "console")

set_target_properties(Console PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/os)

