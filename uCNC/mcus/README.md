# µCNC
µCNC - A universal CNC firmware for microcontrollers

## How to add a custom MCU to µCNC
µCNC can be extended to other custom MCU as well. For that you only need to create two files:
  1. A correspondent mcumap.h file that defines the pins equivalent masks in your MCU and some custom definitions to access ROM/Flash memory and special math operations (check [mcumap_virtual.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/virtual/mcumap_virtual.h) and [mcumap_uno_grbl.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus/avr/mcumap_uno_grbl.h) to get an idea).
  2. A file that implements all the functions defined in the [mcu.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcu.h) header file. mcu.h already has implemented the necessary instructions to perform direct IO operations to digital pins.
  3. You must also edit [mcus.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcus.h) and [mcudefs.h](https://github.com/Paciente8159/uCNC/blob/master/uCNC/mcudefs.h) to add you custom MCU.

  Compile it and test it.
  