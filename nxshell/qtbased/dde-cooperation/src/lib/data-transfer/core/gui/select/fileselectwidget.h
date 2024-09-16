#ifndef FILESELECTWIDGET_H
#define FILESELECTWIDGET_H

#include <QFrame>
#include <QListView>
#include <QMap>
#include <QStandardItemModel>
#include "../select/selectmainwidget.h"
#include "item.h"
class QLabel;
class SelectListView;
class QStackedWidget;
class ItemTitlebar;

class FileSelectWidget : public QFrame
{
    Q_OBJECT

public:
    FileSelectWidget(SidebarWidget *siderbarWidget, QWidget *parent = nullptr);
    ~FileSelectWidget();

    void changeText();
    void clear();
public slots:
    void nextPage();
    void backPage();

    void changeFileView(const QModelIndex &siderbarIndex);
    void selectOrDelAllItem();
    void updateFileSelectList(QStandardItem *item);
    void updateFileViewSize(quint64 fileSize, const QString &path);
    void updateFileViewData(QStandardItem *siderbarItem, const bool &isAdd);
    void selectOrDelAllItemFromSiderbar(QStandardItem *siderbarItem);
    void updateTitleSelectBtnState(QStandardItem *siderbarItem,ListSelectionState state);
    void sortListview();
    void sortListviewColumn2();

signals:
    void isOk(const SelectItemName &name);

private:
    void initUI();
    void initFileView();
    void sendOptions();
    void delOptions();
    SelectListView *addFileViewData(const QString &path, QStandardItem *siderbarIndex);
    void createFilesizeListen(QListView *listView);
    void startCalcluateFileSize(QList<QString> fileList);

private:
    SidebarWidget *sidebar{ nullptr };
    QMap<QStandardItem *, QListView *> sidebarFileViewList;
    QStackedWidget *stackedWidget{ nullptr };
    QLabel *titileLabel{ nullptr };
    QString InternetText{ tr("Select the file to transfer") };
    QString LocalText{ tr("Please select the files to back up") };
    ItemTitlebar *titlebar{ nullptr };
};

#endif
