# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.0/components/bootloader/subproject"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/tmp"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/src"
  "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/equipo3/Desktop/ESP32_Projects/02_Oled/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
