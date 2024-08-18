# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "E:/espressif/esp-idf/components/bootloader/subproject"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/tmp"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/src/bootloader-stamp"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/src"
  "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/01-porject/01-petwater/02-code/03-freertos-test/freerots_test/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
