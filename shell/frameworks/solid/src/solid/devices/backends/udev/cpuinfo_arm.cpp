/*
    SPDX-FileCopyrightText: 2021 Alexey Minnekhanov <alexeymin@postmarketos.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "cpuinfo_arm.h"

// don't even build the following code for non-ARM platforms
#ifdef BUILDING_FOR_ARM_TARGET

namespace Solid
{
namespace Backends
{
namespace UDev
{
/**
 * IDs list is based on code from lscpu-arm.c from util-linux project.
 * You can also find them in Linux kernel source:
 * https://elixir.bootlin.com/linux/latest/source/arch/arm64/include/asm/cputype.h
 * https://elixir.bootlin.com/linux/latest/source/arch/arm/include/asm/cputype.h
 * There is also maintained list at https://github.com/bp0/armids
 **/

// clang-format off
static const struct ArmIdPart armParts[] = {
    {0x810, "ARM810"},
    {0x920, "ARM920"},
    {0x922, "ARM922"},
    {0x926, "ARM926"},
    {0x940, "ARM940"},
    {0x946, "ARM946"},
    {0x966, "ARM966"},
    {0xa20, "ARM1020"},
    {0xa22, "ARM1022"},
    {0xa26, "ARM1026"},
    {0xb02, "ARM11 MPCore"},
    {0xb36, "ARM1136"},
    {0xb56, "ARM1156"},
    {0xb76, "ARM1176"},
    {0xc05, "Cortex-A5"},
    {0xc07, "Cortex-A7"},
    {0xc08, "Cortex-A8"},
    {0xc09, "Cortex-A9"},
    {0xc0d, "Cortex-A17"},
    {0xc0e, "Cortex-A17"},
    {0xc0f, "Cortex-A15"},
    {0xc14, "Cortex-R4"},
    {0xc15, "Cortex-R5"},
    {0xc17, "Cortex-R7"},
    {0xc18, "Cortex-R8"},
    {0xc20, "Cortex-M0"},
    {0xc21, "Cortex-M1"},
    {0xc23, "Cortex-M3"},
    {0xc24, "Cortex-M4"},
    {0xc27, "Cortex-M7"},
    {0xc60, "Cortex-M0+"},
    {0xd01, "Cortex-A32"},
    {0xd03, "Cortex-A53"},
    {0xd04, "Cortex-A35"},
    {0xd05, "Cortex-A55"},
    {0xd06, "Cortex-A65"},
    {0xd07, "Cortex-A57"},
    {0xd08, "Cortex-A72"},
    {0xd09, "Cortex-A73"},
    {0xd0a, "Cortex-A75"},
    {0xd0b, "Cortex-A76"},
    {0xd0c, "Neoverse-N1"},
    {0xd0d, "Cortex-A77"},
    {0xd0e, "Cortex-A76AE"},
    {0xd13, "Cortex-R52"},
    {0xd20, "Cortex-M23"},
    {0xd21, "Cortex-M33"},
    {0xd40, "Neoverse-V1"},
    {0xd41, "Cortex-A78"},
    {0xd42, "Cortex-A78AE"},
    {0xd44, "Cortex-X1"},
    {0xd46, "Cortex-A510"},
    {0xd47, "Cortex-A710"},
    {0xd48, "Cortex-X2"},
    {0xd49, "Neoverse-N2"},
    {0xd4a, "Neoverse-E1"},
    {0xd4b, "Cortex-A78C"},
    {0xd4d, "Cortex-A715"},
    {-1, "unknown"},
};
// clang-format on

static const struct ArmIdPart brcmParts[] = {
    {0x0f, "Brahma B15"},
    {0x100, "Brahma B53"},
    {0x516, "ThunderX2 (Vulcan)"},
    {-1, "unknown"},
};

static const struct ArmIdPart decParts[] = {
    {0xa10, "SA110"},
    {0xa11, "SA1100"},
    {-1, "unknown"},
};

static const struct ArmIdPart caviumParts[] = {
    {0x0a0, "ThunderX"},
    {0x0a1, "ThunderX 88XX"},
    {0x0a2, "ThunderX 81XX"},
    {0x0a3, "ThunderX 83XX"},
    {0x0af, "ThunderX2 99xx"},
    {-1, "unknown"},
};

static const struct ArmIdPart apmParts[] = {
    {0x000, "X-Gene"},
    {-1, "unknown"},
};

static const struct ArmIdPart qcomParts[] = {
    {0x00f, "Scorpion"},
    {0x02d, "Scorpion"},
    {0x04d, "Krait"},
    {0x06f, "Krait 400"},
    {0x200, "Kryo"},
    {0x201, "Kryo Silver"},
    {0x205, "Kryo Gold"},
    {0x211, "Kryo"},
    {0x800, "Falkor V1/Kryo 2xx Gold"},
    {0x801, "Kryo 2xx Silver"},
    {0x802, "Kryo 3xx Gold"},
    {0x803, "Kryo 3xx Silver"},
    {0x804, "Kryo 4xx/5xx Gold"},
    {0x805, "Kryo 4xx/5xx Silver"},
    {0xc00, "Falkor"},
    {0xc01, "Saphira"},
    {-1, "unknown"},
};

static const struct ArmIdPart samsungParts[] = {
    {0x001, "exynos-m1"},
    {-1, "unknown"},
};

static const struct ArmIdPart nvidiaParts[] = {
    {0x000, "Denver"},
    {0x003, "Denver 2"},
    {0x004, "Carmel"},
    {-1, "unknown"},
};

static const struct ArmIdPart marvellParts[] = {
    {0x131, "Feroceon 88FR131"},
    {0x581, "PJ4/PJ4b"},
    {0x584, "PJ4B-MP"},
    {-1, "unknown"},
};

static const struct ArmIdPart faradayParts[] = {
    {0x526, "FA526"},
    {0x626, "FA626"},
    {-1, "unknown"},
};

// clang-format off
static const struct ArmIdPart intelParts[] = {
    {0x200, "i80200"},
    {0x210, "PXA250A"},
    {0x212, "PXA210A"},
    {0x242, "i80321-400"},
    {0x243, "i80321-600"},
    {0x290, "PXA250B/PXA26x"},
    {0x292, "PXA210B"},
    {0x2c2, "i80321-400-B0"},
    {0x2c3, "i80321-600-B0"},
    {0x2d0, "PXA250C/PXA255/PXA26x"},
    {0x2d2, "PXA210C"},
    {0x411, "PXA27x"},
    {0x41c, "IPX425-533"},
    {0x41d, "IPX425-400"},
    {0x41f, "IPX425-266"},
    {0x682, "PXA32x"},
    {0x683, "PXA930/PXA935"},
    {0x688, "PXA30x"},
    {0x689, "PXA31x"},
    {0xb11, "SA1110"},
    {0xc12, "IPX1200"},
    {-1, "unknown"},
};
// clang-format on

static const struct ArmIdPart fujitsuParts[] = {
    {0x001, "A64FX"},
    {-1, "unknown"},
};

static const struct ArmIdPart hisiParts[] = {
    {0xd01, "Kunpeng-920 (TSV110)"},
    {-1, "unknown"},
};

static const struct ArmIdPart appleParts[] = {
    {0x020, "Icestorm (A14)"},
    {0x021, "Firestorm (A14)"},
    {0x022, "Icestorm (M1)"},
    {0x023, "Firestorm (M1)"},
    {0x024, "Icestorm (M1 Pro)"},
    {0x025, "Firestorm (M1 Pro)"},
    {0x028, "Icestorm (M1 Max)"},
    {0x029, "Firestorm (M1 Max)"},
    {0x030, "Blizzard (A15)"},
    {0x031, "Avalanche (A15)"},
    {0x032, "Blizzard (M2)"},
    {0x033, "Avalanche (M2)"},
    {0x034, "Blizzard (M2 Pro)"},
    {0x035, "Avalanche (M2 Pro)"},
    {0x038, "Blizzard (M2 Max)"},
    {0x039, "Avalanche (M2 Max)"},
    {-1, "unknown"},
};

static const struct ArmIdPart unknownPart[] = {
    {-1, "unknown"},
};

static const struct ArmCpuImplementer armHwImplementers[] = {
    {0x41, armParts, "ARM"},
    {0x42, brcmParts, "Broadcom"},
    {0x43, caviumParts, "Cavium"},
    {0x44, decParts, "DEC"},
    {0x46, fujitsuParts, "FUJITSU"},
    {0x48, hisiParts, "HiSilicon"},
    {0x49, unknownPart, "Infineon"},
    {0x4d, unknownPart, "Motorola/Freescale"},
    {0x4e, nvidiaParts, "NVIDIA"},
    {0x50, apmParts, "APM"},
    {0x51, qcomParts, "Qualcomm"},
    {0x53, samsungParts, "Samsung"},
    {0x56, marvellParts, "Marvell"},
    {0x61, appleParts, "Apple"},
    {0x66, faradayParts, "Faraday"},
    {0x69, intelParts, "Intel"},
    {0xc0, unknownPart, "Ampere"},
    {-1, unknownPart, "unknown"},
};

const ArmCpuImplementer *findArmCpuImplementer(int vendorId)
{
    const ArmCpuImplementer *ret = nullptr;
    for (int i = 0; armHwImplementers[i].id != -1; i++) {
        if (armHwImplementers[i].id == vendorId) {
            ret = &armHwImplementers[i];
            break;
        }
    }
    return ret;
}

/*
 * The CPU part number is meaningless without referring to the CPU
 * implementer: implementers are free to define their own part numbers
 * which are permitted to clash with other implementer part numbers.
 */
QString findArmCpuModel(int vendorId, int modelId)
{
    const ArmCpuImplementer *impl = findArmCpuImplementer(vendorId);
    if (!impl) {
        return QString();
    }
    for (int i = 0; impl->parts[i].id != -1; i++) {
        if (impl->parts[i].id == modelId) {
            return QString::fromUtf8(impl->name) + QLatin1Char(' ') + QString::fromUtf8(impl->parts[i].name);
        }
    }
    return QString();
}

} // namespace UDev
} // namespace Backends
} // namespace Solid

#endif // BUILDING_FOR_ARM_TARGET
