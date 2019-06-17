cmake_minimum_required(VERSION 3.13)

project(k3rmit-term C)

SET(TARGET "k3rmit")

find_package(PkgConfig REQUIRED)

add_executable(${TARGET} src/${TARGET}.c src/${TARGET}.h)

if (PKG_CONFIG_FOUND)
  pkg_check_modules(GTK "gtk+-3.0>=3.18.9")
  if (GTK_FOUND)    
    target_link_libraries(${TARGET} ${GTK_LIBRARIES})
    add_definitions(${GTK_CFLAGS} ${GTK_CFLAGS_OTHER})
  endif()
  pkg_check_modules(VTE "vte-2.91>=0.42.5")
  if (VTE_FOUND)
    target_link_libraries(${TARGET} ${VTE_LIBRARIES})
    add_definitions(${VTE_CFLAGS} ${VTE_CFLAGS_OTHER})
  endif()
endif()

TARGET_COMPILE_OPTIONS(${TARGET} PRIVATE -Wall)

INSTALL(TARGETS RUNTIME DESTINATION bin)