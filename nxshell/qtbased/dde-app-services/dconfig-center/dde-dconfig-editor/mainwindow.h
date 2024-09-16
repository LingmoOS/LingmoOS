// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>

#include <DTreeWidget>
#include <DListView>
#include <QVBoxLayout>
#include <DAbstractDialog>
#include <DDialog>
class ValueHandler;
class ConfigGetter;
class ExportDialog;
class OEMDialog;
DWIDGET_USE_NAMESPACE

#define APP_ICON "://appicon/dde-dconfig-editor.svg"

class LevelDelegate : public DStyledItemDelegate {
    Q_OBJECT
public:
    explicit LevelDelegate(QAbstractItemView *parent);
    ~LevelDelegate();
protected slots:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class KeyContent : public QWidget{
    Q_OBJECT
public:
    KeyContent(const QString &key, QWidget *parent = nullptr);
    void setBaseInfo(ConfigGetter *getter, const QString &language);
    QString key() const;
    void updateContent(ConfigGetter *getter);

private Q_SLOTS:
    void onDoubleValueChanged(double value);

Q_SIGNALS:
    void valueChanged(const QVariant &value);

private:
    QString handleModificationInfomation(const QString &text, bool isModified) const;
    QString m_key;
    QHBoxLayout *m_hLay = nullptr;
};

class Content : public QWidget {
    Q_OBJECT
public:
    explicit Content(QWidget *parent = nullptr);

    ~Content();

    void refreshResourceKeys(const QString &appid, const QString &resourceId, const QString &subpath, const QString &matchKeyId = QString());

    void clear();

    ValueHandler *getter();
    static bool isVisible(ConfigGetter *manager, const QString &key);

    static void remove(QLayout *layout);

    void setLanguage(const QString &language);
    QString language() { return m_language; }
Q_SIGNALS:
    void sendValueUpdated(const QStringList &keyid, const QVariant &pre, const QVariant &now);

    void languageChanged();
    void requestRefreshResourceKeys();

private Q_SLOTS:
    void onValueChanged(const QVariant &value);
    void onCustomContextMenuRequested(QWidget *widget, const QString &appid, const QString &resource, const QString &subpath, const QString &key);
private:
    QVBoxLayout *m_contentLayout = nullptr;
    QScopedPointer<ValueHandler> m_getter;
    QString m_language;
};

class HistoryDialog : public DDialog {
    Q_OBJECT
public:
    explicit HistoryDialog(QWidget *parent = nullptr);

public Q_SLOTS:
    void onSendValueUpdated(const QStringList &key, const QVariant &pre, const QVariant &now);

Q_SIGNALS:
    void refreshResourceKeys(const QString &appid, const QString &resourceId, const QString &subpath);
private:

    DListView *historyView = nullptr;
    int maxRows = 100;
};

class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void refreshApps(const QString &matchAppid = QString());

    void refreshAppResources(const QString &appid, const QString &matchResource = QString());

    void refreshResourceSubpaths(QStandardItemModel *model, const QString &appid, const QString &resourceId);

    void refreshResourceKeys(const QString &appid, const QString &resourceId, const QString &subpath, const QString &matchKeyId = QString());

    void onCustomResourceMenuRequested(const QString &appid, const QString &resource, const QString &subpath);
private:
    void installTranslate();

    void translateAppName();

    void refreshAppTranslate();

private:
    QWidget *centralwidget;
    DListView *appListView;
    DListView *resourceListView;
    Content *contentView;

    HistoryDialog *historyView = nullptr;
    ExportDialog *exportView = nullptr;
    OEMDialog *oemView = nullptr;
    QMap<QString, QString> appIdToNameMaps;


};

#endif // MAINWINDOW_H
