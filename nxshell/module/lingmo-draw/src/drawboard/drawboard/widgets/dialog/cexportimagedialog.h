// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CEXPORTIMAGEDIALOG_H
#define CEXPORTIMAGEDIALOG_H
#include "globaldefine.h"

#include <QWidget>
#include "drawboard.h"
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#ifdef USE_DTK
#include <DDialog>
#include <DLineEdit>
#include <DMessageBox>
#define LINEEDITOR DLineEdit
#define EXPORTFATHER DDialog
DWIDGET_USE_NAMESPACE
#else
#include <QCheckBox>
#include <QDialog>
#define LINEEDITOR QLineEdit
#define EXPORTFATHER QDialog
#endif

class Page;
class DRAWLIB_EXPORT CExportImageDialog : public EXPORTFATHER
{
    Q_OBJECT
public:

public:
    explicit CExportImageDialog(DrawBoard *drawbord, DWidget *parent = nullptr);
    ~CExportImageDialog();

    int getImageType() const;
    QString getImageFormate() const;
    int getQuality() const;

    int exec() override;

    int execFor(const Page *page);

    QString resultFile()const;
    QSize desImageSize()const;

private slots:
    void slotOnSavePathChange(int index);
    void slotOnFormatChange(int index);
    int  execCheckFile(const QString &text);
    void slotOnQualityChanged(int value);
    /**
     * @brief 根据布局模式(紧凑)变更更新界面布局
     */
    void updateSizeMode();
signals:
    void signalDoSave(QString);

protected:
    void showEvent(QShowEvent *event) override;
    void saveSetting();
    void resizeEvent(QResizeEvent *event) override;


private:
    LINEEDITOR *m_fileNameEdit;

    QComboBox *m_savePathCombox;
    QComboBox *m_formatCombox;
    QSlider *m_qualitySlider;
    QLabel *m_qualityLabel;

    QString m_savePath;
    QString m_saveFormat;
    int m_quality;
    int m_saveBtnId = -1;
    int quitRet = 1;

private:
    void initUI();
    void initConnection();

    void showDirChoseDialog();
    int  execFileIsExists(const QString &path);
    QString getCompleteSavePath()const;

    void keyPressEvent(QKeyEvent *event) override;
    DrawBoard   *m_drawBoard = nullptr;

    PRIVATECLASS(CExportImageDialog)
};

#endif // CEXPORTIMAGEDIALOG_H
