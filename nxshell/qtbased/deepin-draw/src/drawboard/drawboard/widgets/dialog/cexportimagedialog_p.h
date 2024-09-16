// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CEXPORTIMAGEDIALOG_P_H
#define CEXPORTIMAGEDIALOG_P_H

#include "cexportimagedialog.h"
#include "cvalidator.h"

#include <QSpinBox>
#include <QCheckBox>

#include <QFormLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QTimer>

//这里模仿QtBase，把d指针的内容挪到_p.h文件中，增强软件可测试性
class CSpinBox;
class CExportImageDialog::CExportImageDialog_private
{
public:
    enum ESizeSettingModel {ERadioModel, EPixelModel, ESettingModelCount};
    explicit CExportImageDialog_private(CExportImageDialog *q): _q(q) {}

    enum EKeepBase {EKeepBaseW, EKeepBaseH, EKeepBaseRadioValue, EFreeSetting};
    enum EAlertReason {ETooSmall = 0, ETooBig, ENoAlert};

    void initSizeSettingLayoutUi(QFormLayout *fLayout, QWidget *contentWidget);
    void updateSettingModelUi();
    void resetImageSettingSizeTo(const QSize &sz, qreal raido = 1.0, bool keepRadio = true, ESizeSettingModel settingModel = ERadioModel);
    void setSizeSettingModel(ESizeSettingModel model);
    bool autoKeepSize(EKeepBase base);
    static QSizeF autoKeepWHRadio(int base, qreal precentRadio,
                                  const QSize &wantedSize,
                                  const QSizeF &preSize,
                                  const QSize &orgSz,
                                  EAlertReason &aler);
    void showTip(EAlertReason alertReson = ETooBig);
    bool isFocusInEditor() const;

    CExportImageDialog *_q;
    QFormLayout *_formLayout = nullptr;

    QRadioButton *_radioRadioBtn = nullptr;
    QRadioButton *_radioPiexlBtn = nullptr;

    CSpinBox  *_radioSpinBox = nullptr;
    QLabel *_precentStuff = nullptr;
    QLabel *_precentpix = nullptr;

    QCheckBox *_keepRaidoCheckBox = nullptr;

    CSpinBox *_widthEditor = nullptr;
    CSpinBox *_heightEditor = nullptr;

    QLabel *_tipLabelForOutOfBounds = nullptr;
    QTimer *timer = nullptr;
    QWidget *piexlWgt = nullptr;
    QWidget *PercentageWgt = nullptr;

    ESizeSettingModel settingModel = ERadioModel;

    QSize originSize;
    qreal  curPrecent = 1.0;
    QSizeF curSize[ESettingModelCount] = {QSizeF(), QSizeF()};
    bool   keepRadioIfPixelModel = true;
    QSize  curShowIntSize;
};

#endif // CEXPORTIMAGEDIALOG_P_H
