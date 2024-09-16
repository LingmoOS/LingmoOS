// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KETSETTINGSITEM_H
#define KETSETTINGSITEM_H
#include "keylabel.h"
#include "settingsitem.h"
#include "labels/shortenlabel.h"
#include "DLineEdit"
#include <QComboBox>
#include <QPushButton>

#include <fcitx/addon.h>

using namespace Dtk::Widget;
using namespace dcc_fcitx_configtool::widgets;
namespace dcc_fcitx_configtool {
namespace widgets {

class FcitxKeyLabelWidget : public QWidget
{
    Q_OBJECT
public:
    FcitxKeyLabelWidget(QStringList list = {}, QWidget *p = nullptr);
    virtual ~FcitxKeyLabelWidget();
    void setKeyId(const QString &id);
    void setList(const QStringList &list);
    QString getKeyToStr();
    void setEnableEdit(bool flag);
    /**
     * @brief 设置是否可以输入单个按键
     */
    void enableSingleKey();
signals:
    void editedFinish();
    void shortCutError(const QStringList &list, QString &name);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void clearShortcutKey();
    void setShortcutShow(bool flag);
    bool checkNewKey(bool isRelease = false);
    void initLableList(const QStringList &list);

private:
    QHBoxLayout *m_mainLayout {nullptr};
    QLineEdit *m_keyEdit {nullptr};
    QList<FcitxKeyLabel *> m_list;
    QString m_id;
    QStringList m_curlist;
    QStringList m_newlist;
    bool m_eidtFlag;
    bool m_isSingle {false};
};

class FcitxKeySettingsItem : public FcitxSettingsItem
{
    Q_OBJECT
public:
    FcitxKeySettingsItem(const QString &text = "", const QStringList &list = {}, QFrame *parent = nullptr);
    void setKeyId(const QString &id);
    void setList(const QStringList &list);
    QString getKeyToStr() { return m_keyWidget->getKeyToStr(); }
    void setEnableEdit(bool flag);
    QString getLabelText();
    void setText(const QString &text);
    /**
     * @brief 设置是否可以输入单个按键
     */
    void enableSingleKey();

protected:
    void paintEvent(QPaintEvent *event) override;

signals:
    void editedFinish();
    void shortCutError(const QString &curName, const QStringList &list, QString &name);

public slots:
    void doShortCutError(const QStringList &list, QString &name);

protected:
    void resizeEvent(QResizeEvent *event);
    //void paintEvent(QPaintEvent *event);

private:
    void updateSize();

private:
    FcitxShortenLabel *m_label {nullptr};
    QHBoxLayout *m_hLayout {nullptr};
    FcitxKeyLabelWidget *m_keyWidget {nullptr};
};

class FcitxHotKeySettingsItem : public FcitxSettingsItem
{
    Q_OBJECT
public:
    FcitxHotKeySettingsItem(const QString &text = "", const QStringList &list = {}, QFrame *parent = nullptr);
    void setKeyId(const QString &id);
    void setList(const QStringList &list);
    QString getKeyToStr() { return m_keyWidget->getKeyToStr(); }
    void setEnableEdit(bool flag);
    QString getLabelText();
    void setText(const QString &text);
    /**
     * @brief 设置是否可以输入单个按键
     */
    void enableSingleKey();

signals:
    void editedFinish();
    void shortCutError(const QString &curName, const QStringList &list, QString &name);

public slots:
    void doShortCutError(const QStringList &list, QString &name);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void updateSize();

private:
    FcitxShortenLabel *m_label {nullptr};
    QHBoxLayout *m_hLayout {nullptr};
    FcitxKeyLabelWidget *m_keyWidget {nullptr};
};


class FcitxComBoboxSettingsItem : public FcitxSettingsItem
{
    Q_OBJECT
public:
    FcitxComBoboxSettingsItem(const QString &text, const QStringList &list = {}, QFrame *parent = nullptr);
    virtual ~FcitxComBoboxSettingsItem() override;
    QComboBox *comboBox() { return m_combox; }
    QString getLabelText();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QHBoxLayout *m_mainLayout {nullptr};
    QComboBox *m_combox {nullptr};
    FcitxShortenLabel *m_label {nullptr};
};

class FcitxCheckBoxSettingsItem : public FcitxSettingsItem
{
    typedef struct ConfigDescSet {
        char *filename;
        FcitxConfigFileDesc *cfdesc;
        UT_hash_handle hh;
    } ConfigDescSet;

    Q_OBJECT
public:
    FcitxCheckBoxSettingsItem(FcitxAddon* addon, QWidget *parent = nullptr);
    virtual ~FcitxCheckBoxSettingsItem() override;
signals:
    void onChecked();

private:
    FcitxConfigFileDesc *getConfigDesc(char *filename);

private:
    ConfigDescSet* m_configDescSet;
};

class FcitxGlobalSettingsItem : public FcitxSettingsItem
{
    enum itemPosition{
        firstItem = 0,
        lastItem = -1,
        onlyoneItem = -2,
        otherItem = 1
    };
    Q_OBJECT
public:
    FcitxGlobalSettingsItem(QFrame *parent = nullptr);
    virtual ~FcitxGlobalSettingsItem() override;
    void setIndex(int index) {m_index = index;}
protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;
private:
    int m_index {1};
};

class PushLable : public QLabel {
    Q_OBJECT
public:
    PushLable(QWidget* parent = nullptr);
    void setOriginText(const QString& text);
protected:
    void mousePressEvent(QMouseEvent* ev) override;
    void resizeEvent(QResizeEvent *event) override;
signals:
    void clicked();
private:
    QString m_originText;
};

} // namespace widgets
} // namespace dcc_fcitx_configtool
#endif // KETSETTINGSITEM_H
