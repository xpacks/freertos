# FreeRTOS

An xPack with a selection of ARM related FreeRTOS files.

## Package

[FreeRTOS](https://github.com/xpacks/freertos) package details:

* type: [xcdl](http://xcdl.github.io)
* yotta name: `ilg-freertos`
* yotta repo: http://yotta.mbed.com/#/module/ilg-freertos/0.1.1
* git repo: https://github.com/xpacks/freertos.git
* homepage: https://github.com/xpacks/freertos
* latest archive: https://github.com/xpacks/freertos/archive/xpack.zip

## Version

* 0.1.1

## Releases

* none

## Package maintainers

* [Liviu Ionescu](http://liviusdotnet.worldpress.com) [&lt;ilg@livius.net&gt;](mailto:ilg@livius.net)

## Content contributors

* Richard Barry
* [Lix Paulian](https://github.com/lixpaulian) [&lt;lix@paulian.net&gt;](mailto:lix@paulian.net)
* [Liviu Ionescu](http://liviusdotnet.worldpress.com) [&lt;ilg@livius.net&gt;](mailto:ilg@livius.net)

## Support

* [all issues](https://github.com/xpacks/freertos/issues)

## Dependencies

* none

## Keywords

* rtos
* arm
* embedded

## License

* custom, see `LICENSE.txt` in package root

--- 
## Documentation

To use the FreeRTOS port of CMSIS++, the following steps should be followed:

- add `freertos-xpack/FreeRTOS/Source` to the source folders
- exclude all folders below `portable/GCC` except the desired one (for ex. `ARM-CM3`)
- exclude all folders below `portable/MemMang` except the deired one (for ex. `heap_4.c`)
- add `-Wno-conversion -Wno-sign-conversion` to `portable/GCC/ARM-CM3`
- add `freertos-xpack/cmsis-plus/src` to the source folders
- add `freertos-xpack/FreeRTOS/Source/include` to the include folders
- add `freertos-xpack/FreeRTOS/Source/portable/GCC/ARM-CM3` to the include folders
- add `freertos-xpack/FreeRTOS/cmsis-plus/include` to the include folders
- copy `include/FreeRTOSConfig.h` to the application includes

## Tests

- native - a simple test using native FreeRTOS calls.

More relevant tests are in separate projects 
- arm-cmsis-rtos-validator-xpack (a version of the ARM CMSIS RTOS validator running on top of FreeRTOS)
- cmsis-plus-xpack (a multi API test).

## Warnings

To silence warnings when compiling various ports, use:

* `freertos-xpack/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c`

```
-Wno-conversion -Wno-sign-conversion
```

--- 
