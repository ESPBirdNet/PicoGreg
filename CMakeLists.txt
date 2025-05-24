 cmake_minimum_required(VERSION 3.13)
 include(~/pico/pico-sdk/external/pico_sdk_import.cmake)

 project(PicoGreg C CXX ASM)
 set(CMAKE_C_STANDARD 11)
 set(CMAKE_CXX_STANDARD 17)

 pico_sdk_init()

# Add the pico_i2c_slave library
add_subdirectory(pico_i2c_slave/i2c_slave)                             # :contentReference[oaicite:5]{index=5}

 # your executable
 add_executable(PicoGreg
  src/main.cpp
  src/display/st7789.c
  src/heatmap/heatmap.c
 )

 # link in stdlib + SPI + DMA
 target_link_libraries(PicoGreg
   pico_stdlib
   hardware_spi
   hardware_dma
   pico_i2c_slave                                          # Link the I2C‑slave lib :contentReference[oaicite:6]{index=6}
 )

 pico_enable_stdio_usb(PicoGreg 1)
 pico_enable_stdio_uart(PicoGreg 0)

 pico_add_extra_outputs(PicoGreg)
