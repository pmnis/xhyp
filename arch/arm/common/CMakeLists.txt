
add_library(common STATIC 
        abort.c
        cache.c
        exceptions.c
        mmu.c
        mmu_dbg.c
        pl190.c
        serial.c
        sp804.c
        tlb.c
        bss.S)
target_compile_definitions(common PRIVATE 
        __KERNEL__
        TEXT_BASE=0x00200000)


target_include_directories(common PRIVATE ${CMAKE_SOURCE_DIR}/include/)

