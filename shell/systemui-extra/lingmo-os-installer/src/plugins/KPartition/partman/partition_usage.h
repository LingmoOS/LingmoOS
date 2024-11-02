#ifndef PARTITION_USAGE_H
#define PARTITION_USAGE_H

#include <QString>
#include "filesystem.h"

namespace KInstaller {
namespace Partman {
    bool readUsage(const QString& partitionpath, FSType fs, qint64& freeface, qint64& total);
}


}

#endif // PARTITION_USAGE_H
