/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef USMABOUTDIALOG_H
#define USMABOUTDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QGSettings>
#include <QTextEdit>

class USMAboutDialog : public QDialog
{
    Q_OBJECT
public:
    explicit USMAboutDialog(QWidget *parent = nullptr);
    virtual ~USMAboutDialog();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

private:
    bool initWidgets();
    bool initTitleWidget();
    bool initContentWidget();
    bool initSupportWidget();
    bool initConnections();

    void initThemeStyle();
    void setFontSize(QLabel *label,int fontSize);
    void setFontSize(QTextEdit *txtEdit,int fontSize);
    void setFontSize(QPushButton *btn,int fontSize);
    void onThemeFontChange(float fFontSize);

private:
    QLabel *m_labelTitleIcon = nullptr;
    QLabel *m_labelTitleText = nullptr;
    QPushButton *m_btnClose = nullptr;
    QLabel *m_labelLogoIcon = nullptr;
    QLabel *m_labelAppName = nullptr;
    QLabel *m_labelVersion = nullptr;
    QLabel *m_labelIntroduce = nullptr;
    QLabel *m_labelSupport = nullptr;
    QPushButton *m_btnSupportUrl = nullptr;

    QHBoxLayout *m_logoIconLayout = nullptr;
    QHBoxLayout *m_appNameLayout = nullptr;
    QHBoxLayout *m_versionLayout = nullptr;
    QHBoxLayout *m_introduceLayout = nullptr;
    QHBoxLayout *m_supportLayout = nullptr;
    QHBoxLayout *m_titleLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;

    QGSettings *m_styleSettings = nullptr;
    float m_fFontSize;
};

#endif // USMABOUTDIALOG_H
