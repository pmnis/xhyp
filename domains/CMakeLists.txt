

file(STRINGS config.domains CONFIG_DOMAINS REGEX "^[^ #]")

file(GLOB DOMAINS_DIR domain0?)

set(OFST "0x00100000")
set(DOM_ID "0")
foreach(config ${CONFIG_DOMAINS})
    math(EXPR DOM_ID "${DOM_ID}+1")
    # original file uses \t and spaces to indent fields, need to remove all this
    # before processing inputs
    string(REGEX REPLACE "[ \t]+" ";" config ${config})
    list(GET config 9 path)
    list(GET config 0 name)
    set(DOMAIN_NAME "domain${DOM_ID}")
    add_custom_command(OUTPUT ${DOMAIN_NAME}.ho
            DEPENDS ${name}
            COMMAND ${CMAKE_OBJCOPY} ${CMAKE_BINARY_DIR}/${path} -O binary ${DOMAIN_NAME}.bin
            COMMAND ${CMAKE_OBJCOPY} --input-target binary --output-target elf32-littlearm
                                     --binary-architecture arm --rename-section '.data=.dm${DOM_ID}'
                                     ${DOMAIN_NAME}.bin ${DOMAIN_NAME}.ho
            COMMENT "Building ${domain}")
    add_custom_target(${DOMAIN_NAME} DEPENDS ${DOMAIN_NAME}.ho)

    list(APPEND DOMAINS_DEPS ${DOMAIN_NAME})
    list(APPEND DOMAINS ${CMAKE_CURRENT_BINARY_DIR}/${DOMAIN_NAME}.ho)

    list(GET config 1 base)
    list(GET config 2 virt)
    list(GET config 3 size)
    list(GET config 4 flags)
    list(GET config 5 prio)
    list(GET config 6 state)
    list(GET config 7 dom_type)
    list(GET config 8 drv_type)
    set(start "${base} + ${OFST}")
    if(dom_type STREQUAL "DTYPE_GPOS")
       set(offset "0xc")
       set(device "0 /* not set */")
       set(device_size "0 /* not set */")
    elseif(dom_type STREQUAL "DTYPE_DRV")
       set(offset "0 /* not set */")
       set(device "CONFIG_PERIPH_BASE")
       set(device_size "0x00200000")
    else()
       set(offset "0 /* not set */")
       set(device "0 /* not set */")
       set(device_size "0 /* not set */")
     endif()
    configure_file(domain_table.c.entry.in domain_table.c.entry.${DOM_ID})
    list(APPEND generated_entries ${CMAKE_CURRENT_BINARY_DIR}/domain_table.c.entry.${DOM_ID})

    #linker script stuff
    string(APPEND PHDRS "  dom${DOM_ID} PT_LOAD;\n")
    string(APPEND SECTIONS "      .dm${DOM_ID} : { } > ram_dom${DOM_ID} : dom${DOM_ID}\n")
    string(APPEND MEMORY_LAYOUT "  ram_ctl${DOM_ID} : org = ${base}, l = ${OFST}\n")
    string(APPEND MEMORY_LAYOUT "  ram_dom${DOM_ID} : org = ${base} + ${OFST}, l = ${size} - ${OFST}\n")
endforeach()

# need to forward file names to paent scope. This is quite ugly, but I did not find any better solution
set(DOMAINS ${DOMAINS} PARENT_SCOPE)
add_custom_target(all_domains
        DEPENDS ${DOMAINS_DEPS})

configure_file(xhyp.ld.in xhyp.ld)

configure_file(domain_table.c.header.in domain_table.c)
foreach(entry ${generated_entries})
    file(READ ${entry} content)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/domain_table.c "${content}")
endforeach()

file(READ domain_table.c.footer.in footer)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/domain_table.c "${footer}")

add_library(domain_table STATIC ${CMAKE_CURRENT_BINARY_DIR}/domain_table.c)
add_dependencies(domain_table all_domains)
target_compile_definitions(domain_table PRIVATE 
        __KERNEL__
        TEXT_BASE=0x00200000)

target_include_directories(domain_table PRIVATE ${CMAKE_SOURCE_DIR}/include/)
