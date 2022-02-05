# Min. CMake Version: 3.13
cmake_minimum_required(VERSION 3.13)
# Set project and language
project(kermit-term C)
# Set target name
set(TARGET "kermit")
# Add project source as executable
add_executable(${TARGET} src/${TARGET}.c src/${TARGET}.h)
# Check PkgConfig
find_package(PkgConfig REQUIRED)
if (PKG_CONFIG_FOUND)
  # Check, add and link GTK3
  pkg_check_modules(GTK REQUIRED "gtk+-3.0>=3.18.9")
  if (GTK_FOUND)
    target_link_libraries(${TARGET} ${GTK_LIBRARIES})
    add_definitions(${GTK_CFLAGS} ${GTK_CFLAGS_OTHER})
  endif()
  # Check, add and link VTE
  pkg_check_modules(VTE REQUIRED "vte-2.91>=0.42.5")
  if (VTE_FOUND)
    target_link_libraries(${TARGET} ${VTE_LIBRARIES})
    add_definitions(${VTE_CFLAGS} ${VTE_CFLAGS_OTHER})
  endif()
endif()
# Compile options
target_compile_options(${TARGET} PRIVATE -Wall -Wno-deprecated-declarations)
# Install target to bin directory
install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
# Install manpage
install(FILES man/${TARGET}.1 DESTINATION share/man/man1)
