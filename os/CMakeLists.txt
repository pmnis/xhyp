
add_custom_command(OUTPUT freeRTOS_xhyp
  COMMAND "${CMAKE_COMMAND}" -E copy ${CMAKE_SOURCE_DIR}/os/freeRTOS_xhyp ${CMAKE_BINARY_DIR}/os/
  COMMENT "Copying to output directory")

add_custom_target(FreeRTOS DEPENDS freeRTOS_xhyp)
