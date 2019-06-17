cmake_minimum_required(VERSION 3.13)

project(k3rmit-term C)

find_package(PkgConfig REQUIRED)

add_executable(k3rmit src/k3rmit.c src/k3rmit.h)

if (PKG_CONFIG_FOUND)
  pkg_check_modules(GTK "gtk+-3.0>=3.18.9")
  if (GTK_FOUND)    
    target_link_libraries(k3rmit ${GTK_LIBRARIES})
    add_definitions(${GTK_CFLAGS} ${GTK_CFLAGS_OTHER})
  endif()
  pkg_check_modules(VTE "vte-2.91>=0.42.5")
  if (VTE_FOUND)
    target_link_libraries(k3rmit ${VTE_LIBRARIES})
    add_definitions(${VTE_CFLAGS} ${VTE_CFLAGS_OTHER})
  endif()
endif()

#INSTALL(TARGETS k3rmit RUNTIME DESTINATION bin)

#gcc -O2 -Wall $(pkg-config --cflags vte-2.91) k3rmit.c -o k3rmit.o $(pkg-config --libs vte-2.91)