// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOPTOOLBAR_H
#define TOPTOOLBAR_H

#include "globaldefine.h"

#include <DFrame>
#include <DMenu>
#include <DLabel>

#include <DWidget>
#include <DStackedWidget>
#include <QKeyEvent>


DWIDGET_USE_NAMESPACE

class CommonshapeWidget;
class PolygonalStarAttributeWidget;
class PolygonAttributeWidget;
class CCutWidget;
class CPenWidget;
class LineWidget;
class CPictureWidget;
class TextWidget;
class BlurWidget;
class CTitleWidget;

class ArrowRectangle;
class ColorPanel;
class CMenu;
class DZoomMenuComboBox;
class QToolButton;
namespace DrawAttribution {
class CAttributeManagerWgt;
}

class TopTilte : public DFrame
{
    Q_OBJECT

public:
    explicit TopTilte(DWidget *parent = nullptr);
    ~TopTilte() override;

    /* 主菜单 */
    DMenu *mainMenu();

    DrawAttribution::CAttributeManagerWgt *attributionsWgt();

    /**
     * @brief setTitleName　设置标题名称
     */
    void setTitleName(const QString &title_name);

    inline QToolButton *editSceneRectButton() {return m_editDrawBorad;}

signals:
    /**
     * @brief signalSaveToDDF 保存ＤＤＦ文件信号
     */
    void toSave();
    /**
     * @brief signalSaveAs 另存为信号
     */
    void toSaveAs();
    /**
     * @brief signalPrint　打印信号
     */
    void toPrint();
    /**
     * @brief signalImport　导入信号
     */
    void toOpen();
    /**
     * @brief signalZoom　放大缩小信号
     */
    void zoomTo(qreal);
    /**
     * @brief signalShowExportDialog　显示导出窗口信号
     */
    void toExport();
    /**
     * @brief signalNew　新建信号
     */
    void creatOnePage();
    /**
     * @brief editProportion　显示比例窗口信号
     */
    void editProportion();

public slots:

    /**
     * @brief slotZoom　缩放
     * @param scale　缩放因子
     */
    void slotZoom(const QString &scale);
    void slotZoom(const qreal &scale);
    /**
     * @brief slotSetScale　显示缩放比例
     * @param scale　缩放因子
     */
    void slotSetScale(const qreal scale);

    /**
     * @brief signalIsAllPictureItem　选中图元图元是否都为图片
     * @param isEnable　自适应按钮是否可用
     * @param single 是否选中单个图片
     */
    //void slotIsAllPictureItem(bool isEnable, bool single);

private slots:
    /**
     * @brief slotIsCutMode　判断是否为裁剪模式
     * @param action
     */
    void slotIsCutMode(QAction *action);
    /**
     * @brief slotOnImportAction　导入按钮触发函数
     */
    void slotOnImportAction();
    /**
     * @brief slotOnNewConstructAction　新建按钮触发函数
     */
    void slotOnNewConstructAction();
    /**
     * @brief slotOnSaveAction　保存按钮触发函数
     */
    void slotOnSaveAction();
    /**
     * @brief slotOnSaveAsAction　另存为按钮触发函数
     */
    void slotOnSaveAsAction();
    /**
     * @brief slotMenuShow　显示主菜单触发函数
     */
    void slotMenuShow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    virtual void enterEvent(QEvent *event) override;                      //进入QWidget瞬间事件

    void paintEvent(QPaintEvent *event) override;

private:
    QString m_path;
    QStringList m_paths;
    int m_textFontsize = 12;


    CMenu *m_mainMenu;
    DZoomMenuComboBox *m_zoomMenuComboBox; // 缩放菜单组件
    QToolButton *m_editDrawBorad;//裁剪按钮
    QLabel *m_label;//标题

    QFont ft; // 全局默认字体

    QAction *m_saveAction;
    QAction *m_newAction;

    DrawAttribution::CAttributeManagerWgt *m_pAttriManaWgt = nullptr;
private:
    /**
     * @brief initUI 初始化ＵＩ
     */
    void initUI();

    /**
     * @brief initMenu　初始化主菜单
     */
    void initMenu();

    /**
     * @brief initComboBox　初始化缩放下拉列表
     */
    void initComboBox();
};

#endif // TOPTOOLBAR_H
