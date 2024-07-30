
#include <QApplication>
#include <QStringList>

#include <kcolorcollection.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("KColorCollectionTest"));
    QApplication app(argc, argv);

    QStringList collections = KColorCollection::installedCollections();
    for (QStringList::ConstIterator it = collections.constBegin(); it != collections.constEnd(); ++it) {
        printf("Palette = %s\n", (*it).toLatin1().constData());

        KColorCollection myColorCollection = KColorCollection(*it);

        printf("Palette Name = \"%s\"\n", myColorCollection.name().toLatin1().constData());
        printf("Description:\n\"%s\"\n", myColorCollection.description().toLatin1().constData());
        printf("Nr of Colors = %d\n", myColorCollection.count());
        for (int i = 0; i < myColorCollection.count(); i++) {
            int r;
            int g;
            int b;
            myColorCollection.color(i).getRgb(&r, &g, &b);
            printf("#%d Name = \"%s\" #%02x%02x%02x\n", i, myColorCollection.name(i).toLatin1().constData(), r, g, b);
        }
    }
    return EXIT_SUCCESS;
}
