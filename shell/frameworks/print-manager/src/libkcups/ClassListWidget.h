/*
    SPDX-FileCopyrightText: 2010 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CLASS_LIST_WIDGET_H
#define CLASS_LIST_WIDGET_H

#include <QListView>
#include <QStandardItemModel>
#include <QTimer>

#include <KPixmapSequenceOverlayPainter>
#include <kcupslib_export.h>

class KCupsRequest;
class KCUPSLIB_EXPORT ClassListWidget : public QListView
{
    Q_OBJECT
    Q_PROPERTY(QString selectedPrinters READ selectedPrinters WRITE setSelectedPrinters USER true)
    Q_PROPERTY(bool showClasses READ showClasses WRITE setShowClasses)
public:
    explicit ClassListWidget(QWidget *parent = nullptr);
    ~ClassListWidget() override;

    bool hasChanges();
    void setPrinter(const QString &printer);
    QString selectedPrinters() const;
    void setSelectedPrinters(const QString &selected);
    bool showClasses() const;
    void setShowClasses(bool enable);
    QStringList currentSelected(bool uri) const;

signals:
    void changed(bool changed);

private slots:
    void init();
    void loadFinished(KCupsRequest *request);
    void modelChanged();

private:
    void updateItemState(QStandardItem *item) const;

    QString m_printerName;
    QStringList m_selectedPrinters;
    KPixmapSequenceOverlayPainter *m_busySeq = nullptr;
    KCupsRequest *m_request = nullptr;
    bool m_changed;
    bool m_showClasses = false;
    QStandardItemModel *m_model = nullptr;
    QTimer m_delayedInit;
};

#endif
