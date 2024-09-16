// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef JOBMANAGERWINDOW_H
#define JOBMANAGERWINDOW_H

#include <DMainWindow>
#include <DDialog>
DWIDGET_USE_NAMESPACE

#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QDBusMessage>

DWIDGET_BEGIN_NAMESPACE
class DIconButton;
class DButtonBox;
class DButtonBoxButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QMenu;
class QAction;
class QPoint;
class QAbstractItemView;
class QTimer;
class QLabel;
QT_END_NAMESPACE

class JobManagerWindow;

class JobsDataModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit JobsDataModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

protected:
    unsigned int getActionStatus(int iRow) const;
    int getJobId(int iRow);
    int getHighestPriority();
    void setHighestPriority(int id, int iPriority);

    void setWhichJob(int which);
    void updateJobState(int id, int state, const QString &message);

    void deleteJobItem(int jobId);
    void addJobItem(const QMap<QString, QVariant> &job);
    void setJobAttributes(int index, const QMap<QString, QVariant> &job);
    void doItemAction(int jobId, unsigned int iAction);

signals:
    void signalJobsCountChanged(int count);
    void signalDoActionFailed(int jobId, unsigned int iAction);

private:
    void sortJobs();

protected slots:
    void slotRefreshJobsList();
    void slotRefreshJobItems();

private:
    QList<QMap<QString, QVariant>> m_jobs;
    int m_iHighestPriority;
    QTimer *m_refreshTimer;
    int m_iWhichJob;

    friend class JobManagerWindow;
    friend class JobListView;
};

class JobItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit JobItemDelegate(QObject *parent = nullptr);
    virtual ~JobItemDelegate() override;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
};

class JobListView : public QTableView
{
    Q_OBJECT

public:
    explicit JobListView(QWidget *parent = nullptr);

protected:
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    bool askDeleteJobs(unsigned int flag);

    void processAction(int index);

    QString getActionName(unsigned int iAction);

    void setLabelContentVisable(bool bShow);

    friend class JobManagerWindow;

protected slots:
    void slotShowContextMenu(const QPoint &pos);
    void slotMenuTriggered(QAction *action);
    void slotShowTips();

private:
    QMenu *m_contextMenu;
    QList<QAction *> m_atctions;
    JobItemDelegate *m_itemDelegate;
    QTimer *m_tipsTimer;
    QPoint m_tipsPos;
    QStringList m_actionNames;
    QLabel *m_label;
};

class JobManagerWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit JobManagerWindow(QWidget *parent = nullptr);

protected:
    void createUi();
    void initUi();
    void initConnect();

protected slots:
    void slotJobsCountChanged(int count);
    void slotWhichBoxClicked(QAbstractButton *whichbut);
    void slotJobStateChanged(const QDBusMessage &msg);
    void slotDoActionFailed(int jobId, unsigned int iAction);

private:
    JobListView *m_jobsView;
    JobsDataModel *m_jobsModel;
    DIconButton *m_refreshBut;
    DButtonBox *m_whichButBox;
    QList<DButtonBoxButton *> m_whichList;
    QLabel *m_jobCountLabel;
};

#endif //JOBMANAGERWINDOW_H
