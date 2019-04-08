
add_library(mlib STATIC 
        _divsi3.S
        _udivsi3.S
        _umodsi3.S)
target_compile_definitions(mlib PRIVATE 
        __KERNEL__
        TEXT_BASE=0x00200000)
target_include_directories(mlib PRIVATE ${CMAKE_SOURCE_DIR}/include/)

