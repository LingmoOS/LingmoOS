/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SELECT_MAKE_MODEL_H
#define SELECT_MAKE_MODEL_H

#include <QDBusMessage>
#include <QWidget>

#include <kcupslib_export.h>

#include "KCupsConnection.h"
#include "PPDModel.h"

namespace Ui
{
class SelectMakeModel;
}

class KCupsRequest;
class KCUPSLIB_EXPORT SelectMakeModel : public QWidget
{
    Q_OBJECT
public:
    explicit SelectMakeModel(QWidget *parent = nullptr);
    ~SelectMakeModel() override;

    void setDeviceInfo(const QString &deviceId, const QString &make, const QString &makeAndModel, const QString &deviceUri);
    void setMakeModel(const QString &make, const QString &makeAndModel);
    QString selectedPPDName() const;
    QString selectedPPDMakeAndModel() const;
    QString selectedPPDFileName() const;
    bool isFileSelected() const;

public slots:
    void checkChanged();
    void ppdsLoaded(KCupsRequest *request);

signals:
    void changed(bool);

private slots:
    void getBestDriversFinished(const QDBusMessage &message);
    void getBestDriversFailed(const QDBusError &error, const QDBusMessage &message);

private:
    void setModelData();
    void selectFirstMake();
    void selectMakeModelPPD();
    void selectRecommendedPPD();

    ReturnArguments m_ppds;
    DriverMatchList m_driverMatchList;
    QString m_make;
    QString m_makeAndModel;
    Ui::SelectMakeModel *const ui;
    PPDModel *m_sourceModel = nullptr;
    KCupsRequest *m_ppdRequest = nullptr;
    bool m_gotBestDrivers = false;
    bool m_hasRecommended = false;
};

#endif
