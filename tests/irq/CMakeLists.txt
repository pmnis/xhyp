add_executable(IRQ_TEST 
        lib/fifo.c
        lib/stdlib.c
        lib/_udivsi3.S
        domain.c
        main.S)

target_compile_definitions(IRQ_TEST PRIVATE 
        __KERNEL__
        TEXT_BASE=0x00200000)


set_target_properties(IRQ_TEST PROPERTIES LINK_FLAGS "-nostdlib -Bstatic -T ${CMAKE_CURRENT_SOURCE_DIR}/domain.ld")

target_include_directories(IRQ_TEST PRIVATE include/)

set_target_properties(IRQ_TEST PROPERTIES POSITION_INDEPENDENT_CODE OFF)

set_target_properties(IRQ_TEST PROPERTIES SUFFIX ".dom")

set_target_properties(IRQ_TEST PROPERTIES OUTPUT_NAME "irq_test")

set_target_properties(IRQ_TEST PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/os)

