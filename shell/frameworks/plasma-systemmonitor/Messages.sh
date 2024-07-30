#! /usr/bin/env bash
$XGETTEXT `find . \( -name \*.cpp -o -name \*.h -o -name \*.qml \) -not -wholename ./src/faces/\*` -o $podir/plasma-systemmonitor.pot
$XGETTEXT `find src/faces/applicationstable -name \*.qml` -o $podir/ksysguard_face_org.kde.ksysguard.applicationstable.pot
$XGETTEXT `find src/faces/processtable -name \*.qml` -o $podir/ksysguard_face_org.kde.ksysguard.processtable.pot
