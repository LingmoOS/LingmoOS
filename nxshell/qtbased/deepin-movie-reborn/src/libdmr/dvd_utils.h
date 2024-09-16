// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_DVD_UTILS_H
#define _DMR_DVD_UTILS_H

#define _DMR_DVD_UTILS_H

#include <QtCore>
#include <QThread>

namespace dmr {
//add by xxj
#ifdef heyi
namespace dvd {
// device could be a dev node or a iso file
QString RetrieveDVDTitle(const QString &device);
/*
   class RetrieveDvdThread
   the class function DVD thread, Retrieve DVD and get DVD message
   todo Handle dvdnav_open blocking of the dvdnav library function
*/
class RetrieveDvdThread: public QThread
{
    Q_OBJECT

public:
    explicit RetrieveDvdThread();
    ~RetrieveDvdThread();

    static RetrieveDvdThread *get();
    void startDvd(const QString &dev);

    // device could be a dev node or a iso file
    QString getDvdMsg(const QString &device);

protected:
    void run();

signals:
    void sigData(const QString &title);

private:
    QAtomicInt _quit{0};
    QString m_dev {QString()};

};

}
#endif
}

#endif /* ifndef _DMR_DVD_UTILS_H */

