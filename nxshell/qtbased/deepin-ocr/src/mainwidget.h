// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WIDGET_H
#define WIDGET_H

#include <DWidget>
#include <DIconButton>
#include <DPlainTextEdit>
#include <DLabel>
#include <DGuiApplicationHelper>
#include <DStackedWidget>
#include <DFrame>
#include <DComboBox>

#include <QMutex>
#include <DToolButton>

#include "resulttextview.h"

#include "textloadwidget.h"
#include "engine/OCREngine.h"

class Frame;
class QThread;
class QGridLayout;
class QHBoxLayout;
class ImageView;
class loadingWidget;
class QShortcut;
DWIDGET_USE_NAMESPACE

class MainWidget : public DWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

    void setupUi(QWidget *Widget);
    void setupConnect();
    void retranslateUi(QWidget *Widget);

    void createLoadingUi();
    void deleteLoadingUi();
    void loadingUi();
    //初始化快捷键
    void initShortcut();

    bool openImage(const QString &path);
    void openImage(const QImage &img, const QString &name = "");

    void loadHtml(const QString &html);
    void loadString(const QString &string);
    void resultEmpty();

    //缩放显示label
    void initScaleLabel();
protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
private slots:
    void setIcons(DGuiApplicationHelper::ColorType themeType);
    void slotCopy();
    void slotExport();
    void runRec(bool needSetImage);
private:
    QGridLayout *m_mainGridLayout{nullptr};
    QHBoxLayout *m_horizontalLayout{nullptr};
    ResultTextView *m_plainTextEdit{nullptr};
    QHBoxLayout *m_buttonHorizontalLayout{nullptr};
    QHBoxLayout *m_tipHorizontalLayout{nullptr};
    DLabel *m_tiplabel{nullptr};
    DIconButton *m_copyBtn{nullptr};
    DIconButton *m_exportBtn{nullptr};
    ImageView *m_imageview{nullptr};

    QHBoxLayout *m_frameLayout{nullptr};
    QFrame *m_frame{nullptr};

    QHBoxLayout *m_frameStackLayout{nullptr};
    Frame *m_frameStack{nullptr};

    DLabel *m_tipIconLabel{nullptr};
    DHorizontalLine *m_line{nullptr};

    QString m_imgName;  //当前图片绝对路径
    QWidget *m_pwidget{nullptr};
    TextLoadWidget *m_loadingWidget{nullptr};
    DLabel *m_loadingTip{nullptr};

    bool m_isLoading{false};

    QThread *m_loadImagethread{nullptr};
    QMutex m_mutex;
    QString m_result;
    QImage *m_currentImg{nullptr};

    DStackedWidget *m_resultWidget{nullptr};
    DLabel *m_noResult{nullptr};
    loadingWidget *m_loadingOcr{nullptr};
    QShortcut *m_scAddView = nullptr;
    QShortcut *m_scReduceView = nullptr;

    QWidget *m_emptyWidget;

    int m_isEndThread = 1;
    QSettings *ocrSetting;
    std::atomic_bool m_needReRunRec = false;

    DComboBox *languageSelectBox {nullptr}; // 语言选择框

signals:
    void sigResult(const QString &);

};

#endif // WIDGET_H
