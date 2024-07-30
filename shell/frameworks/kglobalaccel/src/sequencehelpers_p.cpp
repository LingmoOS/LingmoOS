#include <QKeySequence>

#include "kglobalshortcutinfo_p.h"
#include "sequencehelpers_p.h"

namespace Utils
{
QKeySequence reverseKey(const QKeySequence &key)
{
    int k[maxSequenceLength] = {0, 0, 0, 0};
    int count = key.count();
    for (int i = 0; i < count; i++) {
        k[count - i - 1] = key[i].toCombined();
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

QKeySequence cropKey(const QKeySequence &key, int count)
{
    if (count < 1) {
        return key;
    }

    // Key is shorter than count we want to cut off
    if (key.count() < count) {
        return QKeySequence();
    }

    int k[maxSequenceLength] = {0, 0, 0, 0};
    // cut from beginning
    for (int i = count; i < key.count(); i++) {
        k[i - count] = key[i].toCombined();
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

bool contains(const QKeySequence &key, const QKeySequence &other)
{
    int minLength = std::min(key.count(), other.count());

    // There's an empty key, assume it matches nothing
    if (!minLength) {
        return false;
    }

    bool ret = false;
    for (int i = 0; i <= other.count() - minLength; i++) {
        QKeySequence otherCropped = cropKey(other, i);
        if (key.matches(otherCropped) == QKeySequence::PartialMatch || reverseKey(key).matches(reverseKey(otherCropped)) == QKeySequence::PartialMatch) {
            ret = true;
            break;
        }
    }

    return ret;
}

bool matchSequences(const QKeySequence &key, const QList<QKeySequence> &keys)
{
    // Since we're testing sequences, we need to check for all possible matches
    // between existing and new sequences.

    // Let's assume we have (Alt+B, Alt+F, Alt+G) assigned. Examples of bad shortcuts are:
    // 1) Exact matching: (Alt+B, Alt+F, Alt+G)
    // 2) Sequence shadowing: (Alt+B, Alt+F)
    // 3) Sequence being shadowed: (Alt+B, Alt+F, Alt+G, <any key>)
    // 4) Shadowing at the end: (Alt+F, Alt+G)
    // 5) Being shadowed from the end: (<any key>, Alt+B, Alt+F, Alt+G)

    for (const QKeySequence &otherKey : keys) {
        if (otherKey.isEmpty()) {
            continue;
        }
        if (key.matches(otherKey) == QKeySequence::ExactMatch || contains(key, otherKey) || contains(otherKey, key)) {
            return true;
        }
    }
    return false;
}

QKeySequence mangleKey(const QKeySequence &key)
{
    // Qt triggers both shortcuts that include Shift+Backtab and Shift+Tab
    // when user presses Shift+Tab. Make no difference here.
    int k[maxSequenceLength] = {0, 0, 0, 0};
    for (int i = 0; i < key.count(); i++) {
        // Qt triggers both shortcuts that include Shift+Backtab and Shift+Tab
        // when user presses Shift+Tab. Make no difference here.
        int keySym = key[i].toCombined() & ~Qt::KeyboardModifierMask;
        int keyMod = key[i].toCombined() & Qt::KeyboardModifierMask;
        if ((keyMod & Qt::SHIFT) && (keySym == Qt::Key_Backtab || keySym == Qt::Key_Tab)) {
            k[i] = keyMod | Qt::Key_Tab;
        } else {
            k[i] = key[i].toCombined();
        }
    }

    return QKeySequence(k[0], k[1], k[2], k[3]);
}

} // namespace Utils
