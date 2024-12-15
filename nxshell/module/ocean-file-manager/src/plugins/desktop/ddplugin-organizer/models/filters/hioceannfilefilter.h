// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HIOCEANNFILEFILTER_H
#define HIOCEANNFILEFILTER_H

#include "models/modeldatahandler.h"

namespace ddplugin_organizer {

class HioceannFileFilter : public QObject, public ModelDataHandler
{
    Q_OBJECT
public:
    explicit HioceannFileFilter();
    ~HioceannFileFilter();
    inline bool showHioceannFiles() const {return show;}
    void refreshModel();
    bool acceptInsert(const QUrl &url) override;
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override;
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override;
    bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {}) override;
protected slots:
    void updateFlag();
    void hioceannFlagChanged(bool showHioceann);
protected:
    bool show = false;
};

}
#endif // HIOCEANNFILEFILTER_H
