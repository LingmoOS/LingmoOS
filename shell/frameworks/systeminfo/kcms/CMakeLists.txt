
add_subdirectory( about-distro )
add_subdirectory(vulkan)
add_subdirectory(kwinsupportinfo)
add_subdirectory(cpu)
add_subdirectory(wayland)
add_subdirectory(interrupts)
add_subdirectory(xserver)
add_subdirectory(egl)
add_subdirectory(glx)
add_subdirectory(pci)
add_subdirectory(firmware_security)
add_subdirectory(opencl)
add_subdirectory(block_devices)
add_subdirectory(network)
add_subdirectory(audio_information)
add_subdirectory(memory)

add_subdirectory( samba )

find_package(USB1 MODULE)
set_package_properties(USB1 PROPERTIES DESCRIPTION "library that provides generic access to USB devicess"
                       URL "https://libusb.info/"
                       TYPE OPTIONAL
                       PURPOSE "View USB devices in kinfocenter"
                      )

if(USB1_FOUND)
   add_subdirectory( usbview )
endif()

if (CMAKE_SYSTEM_NAME MATCHES Linux)
    add_subdirectory(energy)
endif()

