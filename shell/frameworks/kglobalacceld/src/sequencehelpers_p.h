#ifndef SEQUENCEHELPERS_H
#define SEQUENCEHELPERS_H

#include <kglobalaccel_export.h>

#include <QKeySequence>

// Some methods are exported for the unittest

namespace Utils
{
KGLOBALACCEL_EXPORT QKeySequence reverseKey(const QKeySequence &key);

KGLOBALACCEL_EXPORT QKeySequence cropKey(const QKeySequence &key, int count);

bool contains(const QKeySequence &key, const QKeySequence &other);

Qt::KeyboardModifiers keyToModifier(int key);

KGLOBALACCEL_EXPORT bool matchSequences(const QKeySequence &key, const QList<QKeySequence> &keys);

KGLOBALACCEL_EXPORT QKeySequence normalizeSequence(const QKeySequence &key);
}

#endif // SEQUENCEHELPERS_H
