#include "string_unit.h"

namespace KServer {

QString regexpLabel(const QString& pattern, const QString& str)
{
    QRegularExpression reg(pattern, QRegularExpression::MultilineOption);
    QRegularExpressionMatch match = reg.match(str);

    if (match.hasMatch()) {
        return match.captured(1);
    } else {
        return QString();
    }
}

}
