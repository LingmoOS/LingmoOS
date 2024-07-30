/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PAGE_CHOOSE_PPD_H
#define PAGE_CHOOSE_PPD_H

#include "GenericPage.h"

#include <QStackedLayout>

#include <KJob>

namespace Ui
{
class PageChoosePPD;
}

class SelectMakeModel;
class PageChoosePPD : public GenericPage
{
    Q_OBJECT
public:
    explicit PageChoosePPD(const QVariantMap &args = QVariantMap(), QWidget *parent = nullptr);
    ~PageChoosePPD() override;

    bool canProceed() const override;
    void setValues(const QVariantMap &args) override;
    QVariantMap values() const override;
    bool isValid() const override;

private slots:
    void checkSelected();
    void selectDefault();
    void resultJob(KJob *job);

private:
    void removeTempPPD();

    Ui::PageChoosePPD *const ui;
    bool m_isValid = false;
    SelectMakeModel *m_selectMM = nullptr;
    QStackedLayout *m_layout = nullptr;
    QString m_ppdFile;
};

#endif
