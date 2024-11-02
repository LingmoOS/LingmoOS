#include "machine.h"

#include <sys/utsname.h>
#include <QString>

namespace KServer {

MachineArch getMachineArch()
{
    struct utsname uts;

    if(uname(&uts) != 0) {
        return MachineArch::UnKnown;
    }
    const QString machine(uts.machine);

    if(machine.startsWith("arm", Qt::CaseInsensitive)) {
        return MachineArch::ARM;//arm64
    }
    if(machine.startsWith("mips", Qt::CaseInsensitive)) {
        return MachineArch::LOONGSON;//mips64
    }
    if(machine.startsWith("sw", Qt::CaseInsensitive)) {
        return MachineArch::SW;//
    }
    if(machine.startsWith("x86", Qt::CaseInsensitive)) {
        return MachineArch::X86;//x86 i386
    }
    return MachineArch::UnKnown;

}

QString getMachineArchName()
{
    switch (getMachineArch()) {
    case MachineArch::ARM:
        return QString("arm");
    case MachineArch::LOONGSON:
        return QString("loongson");
    case MachineArch::SW:
        return QString("sw");
    case MachineArch::X86:
        return QString("x86");
    default:
        return QString("unknown");
    }
}



}
