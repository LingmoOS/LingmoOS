// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PATHSETTINTWGT_H
#define PATHSETTINTWGT_H

#include <DWidget>
#include <DCheckBox>
#include <DSuggestButton>
#include <DLineEdit>

#include <QButtonGroup>

DWIDGET_USE_NAMESPACE
class PathSettingWgt : public DWidget
{
    Q_OBJECT
public:
    enum CheckBoxType{
        LastOptBox,
        CurFileBox,
        CustomBox
    };

public:
    PathSettingWgt(QWidget* parent = nullptr);
    virtual ~PathSettingWgt();

public Q_SLOTS:
    void onSaveIdChanged(int id);
    void setEditText(const QString& text);

private:
    void init();
    void connections();

Q_SIGNALS:
    void savingPathChanged(CheckBoxType id,const QString& path);

private Q_SLOTS:
    void onBoxClicked(int);
    void onBtnClicked();
    void updateSizeMode();

private:
    DCheckBox* m_curFileBox = nullptr;
    DCheckBox* m_lastOptBox = nullptr;
    DCheckBox* m_customBox = nullptr;
    DLineEdit* m_customEdit = nullptr;
    DSuggestButton* m_customBtn = nullptr;
    QButtonGroup* m_group = nullptr;
};

#endif
