// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include <DDialog>
#include <DWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DLineEdit>
#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
class RenameDialog : public DDialog
{
    Q_OBJECT
public:
    RenameDialog(const QString &filename, QWidget *parent = nullptr);
//    void onThemeChanged(ViewerThemeManager::AppTheme theme);
    DLineEdit *m_lineedt;
    DSuggestButton *okbtn;
    DPushButton *cancelbtn;
    QString GetFilePath();
    QString GetFileName();
    void InitDlg();

    void setCurrentTip();
    QString geteElidedText(QFont font, QString str, int MaxWidth);
private slots:
    void slotsFocusChanged(bool onFocus);
    void onThemeChanged(DGuiApplicationHelper::ColorType theme);
public:
    void slotsUpdate();
protected:
    void paintEvent(QPaintEvent *event);
private:
    QVBoxLayout *m_vlayout{nullptr};
    QVBoxLayout *m_labvlayout{nullptr};
    QHBoxLayout *m_hlayout{nullptr};
    QHBoxLayout *m_edtlayout{nullptr};
    DLabel *m_labformat{nullptr};
    QString m_filenamepath;
    QString m_filename;
    QString m_DirPath;
    QString m_basename;

};

#endif // RENAMEDIALOG_H
