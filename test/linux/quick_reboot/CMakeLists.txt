
set(SOURCES quick_reboot.c)
add_executable(quick_reboot ${SOURCES})
target_link_libraries(quick_reboot soem)
install(TARGETS quick_reboot DESTINATION bin)
