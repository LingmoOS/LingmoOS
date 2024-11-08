#include "highlightmodel.h"
#include <QDebug>

HighlightModel::HighlightModel(QObject *parent)
    : QObject(parent)
{
    for (KSyntaxHighlighting::Definition def : m_repository.definitions()) {
        if (def.isHidden())
            continue;
    }
}
