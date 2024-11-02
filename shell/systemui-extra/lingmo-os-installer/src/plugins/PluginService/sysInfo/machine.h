#ifndef MACHINE_H
#define MACHINE_H

#include <QObject>
namespace KServer {
enum class MachineArch
{
    ARM,
    LOONGSON,
    SW,
    X86,
    UnKnown
};

MachineArch getMachineArch();
QString getMachineArchName();

}
#endif // MACHINE_H
