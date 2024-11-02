#include "swaps.h"
#include "../base_unit/file_unit.h"

namespace KServer {

SwapItemList parseSwaps()
{
    SwapItemList result = {};
    const QString content(KReadFile("/proc/swaps"));

    for(const QString& line : content.split('\n')) {
        if((!line.isEmpty()) && (!line.startsWith("Filename"))) {
            const QStringList parts(line.split(QRegExp("\\s+")));
            if(parts.length() == 5) {
                SwapItem item = {
                    parts.at(0),
                    parts.at(1) == "partition" ? SwapType::Partiton : SwapType::File,
                    parts.at(2).toLongLong() * 1000,
                    parts.at(3).toLongLong() * 1000,
                    parts.at(4).toInt(),
                };
                result.append(item);
            }
        }
    }
    return (SwapItemList)result;
}

}
