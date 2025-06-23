# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/benvh/esp/v5.4.1/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/home/benvh/esp/v5.4.1/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader"
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix"
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/tmp"
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/src/bootloader-stamp"
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/src"
  "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/benvh/projects/minstro/firmware/esp32s3/bringup/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
