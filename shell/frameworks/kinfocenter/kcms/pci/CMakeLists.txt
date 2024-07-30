# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

# Linux has lcpci; effectively it's always there, but it's a runtime
# dependency anyway so we don't need to fail on it. `lspci -v` shows
# a verbose PCI tree.
#
# FreeBSD has pciconf in base; that means that it will always
# be there, so the `find_package()` is there mostly to demonstrate that
# we're looking for it (and not for lspci). `pciconf -lv` shows a verbose
# PCI tree.
if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    find_package(pciconf)
    set_package_properties(pciconf PROPERTIES TYPE RUNTIME)

    set(PCI_BACKEND pciconf)
    set(PCI_BACKEND_ARGUMENTS "-lv")
else()
    find_package(lspci)
    set_package_properties(lspci PROPERTIES TYPE RUNTIME)

    set(PCI_BACKEND lspci)
    set(PCI_BACKEND_ARGUMENTS "-v")
endif()

configure_file(kcm_pci.json.cmake kcm_pci.json)
configure_file(backend.h.cmake backend.h)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_pci\")

kinfocenter_add_kcm(kcm_pci main.cpp)
target_link_libraries(kcm_pci PRIVATE KF6::CoreAddons KF6::KCMUtilsQuick KF6::I18n KInfoCenterInternal)
