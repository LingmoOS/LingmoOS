#include <QString>
#include <ksortablelist.h>

int main(int /*argc*/, char ** /*argv*/)
{
    KSortableList<QString> list;
    list.insert(1, QStringLiteral("FOO           (1)"));
    list.insert(2, QStringLiteral("Test          (2)"));
    list.insert(1, QStringLiteral("Huba!         (1)"));
    list.insert(5, QStringLiteral("MAAOOAM!      (5)"));
    list.insert(10, QStringLiteral("Teeheeest    (10)"));
    list.insert(2, QStringLiteral("I was here :) (2)"));
    list.insert(4, QStringLiteral("Yeehaa...     (4)"));

    QList<KSortableItem<QString>>::iterator it = list.begin();

    qDebug("Insertion order:");
    qDebug("================");
    for (; it != list.end(); ++it) {
        qDebug("%i: %s", (*it).key(), (*it).value().toLatin1().constData());
    }

    list.sort();

    qDebug("\nSorted:");
    qDebug("=======");

    it = list.begin();
    for (; it != list.end(); ++it) {
        qDebug("%i: %s", (*it).key(), (*it).value().toLatin1().constData());
    }

    return 0;
}
