// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drawboardtoolmgr.h"
#include "drawboard.h"
#include "utils/baseutils.h"
#include "drawtoolfactory.h"

#include "pagecontext.h"
#include "pageview.h"
#include "pagescene.h"
#include "drawtool.h"
#include <cundoredocommand.h>

#include <QFileDialog>
#include <QWidget>
#ifdef USE_DTK
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DApplicationHelper>
DGUI_USE_NAMESPACE
#endif

#include <QVBoxLayout>
#include <QDebug>
#include <QAction>
#include <QGraphicsDropShadowEffect>

#include <QButtonGroup>
const int BTN_SPACING = 12;
bool blocked = false;



class DrawBoardToolMgr::DrawBoardToolMgr_private
{
public:
    explicit DrawBoardToolMgr_private(DrawBoardToolMgr *qq, DrawBoard *db): q(qq), _db(db) {}

    void initUI()
    {
        q->setWindowTitle("Tools");
        setWgtAccesibleName(q, "draw tool bar");
#ifdef USE_DTK
        //设置颜色
        //QPalette pa = q->palette();
        //pa.setColor(QPalette::Background, QPalette::Base);
        //q->setPalette(pa);
        //q->setAutoFillBackground(true);
#endif

        auto mainLayout = new QVBoxLayout(q);
        mainLayout->setContentsMargins(10, 10, 10, 10);

        m_layout = new QVBoxLayout;
        m_layout->setSpacing(BTN_SPACING);
        m_layout->setMargin(0);
        m_layout->setContentsMargins(0, 0, 0, 0);
        mainLayout->addLayout(m_layout, Qt::AlignCenter);
    }

    void initToolBtnGroup()
    {
        toolButtonGroup = new QButtonGroup(q);
        toolButtonGroup->setExclusive(true);
        connect(toolButtonGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), q, [ = ](int id) {
            emit q->toolClicked(id);
            q->setCurrentTool(id);
        });
    }

    DrawBoardToolMgr *q;

    QLayout  *m_layout = nullptr;
    QButtonGroup *toolButtonGroup = nullptr;
    DrawTool *_currentTool = nullptr;
    QMap<int, DrawTool *> _tools;
    DrawBoard *_db = nullptr;
};

DrawBoardToolMgr::DrawBoardToolMgr(DrawBoard *db, QWidget *parent)
    : QWidget(parent), DrawBoardToolMgr_d(new DrawBoardToolMgr_private(this, db))
{
#ifdef USE_DTK
    // this->setFrameRounded(false);
#endif
    //this->setFrameShape(QFrame::NoFrame);

    d_DrawBoardToolMgr()->initUI();

    d_DrawBoardToolMgr()->initToolBtnGroup();

    addTool(DrawToolFactory::createTool(selection));
    addDefaultTools();
    setCurrentTool(selection);

//    QGraphicsDropShadowEffect  *effect = new QGraphicsDropShadowEffect(this);
//    effect->setBlurRadius(20);
//    effect->setOffset(QPoint(0, 0));
//    this->setGraphicsEffect(effect);
}

DrawBoardToolMgr::~DrawBoardToolMgr()
{

}

bool DrawBoardToolMgr::setCurrentTool(int tool, bool force)
{
    return setCurrentTool(this->tool(tool), force);
}

bool DrawBoardToolMgr::setCurrentTool(DrawTool *tool, bool force)
{
    if (blocked)
        return false;

    if (tool == nullptr)
        return false;

    if (tool == d_DrawBoardToolMgr()->_currentTool)
        return false;

    bool ret = true;

    if (!tool->isEnable()) {
        return false;
    }

    //qWarning() << "_currentTool == " << (_currentTool == nullptr ? 0 : _currentTool->getDrawToolMode()) << "want to tool == " << tool->getDrawToolMode();
    auto current = d_DrawBoardToolMgr()->_currentTool;
    if (tool != current) {
        bool doChange = true;
        if (current != nullptr && current->status() == DrawTool::EWorking) {
            if (force) {
                current->interrupt();
            } else {
                qWarning() << "can not active another tool when one tool is working!";
                doChange = false;
            }
        }
        if (doChange) {
            d_DrawBoardToolMgr()->_currentTool = tool;

            extern void _setPageTool(Page * page, int tool);
            if (d_DrawBoardToolMgr()->_db != nullptr && d_DrawBoardToolMgr()->_db->currentPage() != nullptr)
                _setPageTool(d_DrawBoardToolMgr()->_db->currentPage(), tool->toolType());

            //emit currentToolChanged(current != nullptr ? current->toolType() : 0, tool->toolType());

            if (current != nullptr && current->toolButton() != nullptr) {
                blocked = true;
                current->toolButton()->setChecked(false);
                blocked = false;
                current->changeStatusFlagTo(DrawTool::EIdle);
            }

            if (tool->toolButton() != nullptr) {
                blocked = true;
                tool->toolButton()->setChecked(true);
                blocked = false;
                tool->changeStatusFlagTo(DrawTool::EReady);
            }
            emit currentToolChanged(current != nullptr ? current->toolType() : 0, tool->toolType());
        }
        ret = doChange;
    }
    return ret;
}

int DrawBoardToolMgr::currentTool() const
{
    //return toolButtonGroup->checkedId();
    if (d_DrawBoardToolMgr()->_currentTool != nullptr) {
        return d_DrawBoardToolMgr()->_currentTool->toolType();
    }
    return 0;
}

DrawTool *DrawBoardToolMgr::tool(int tool) const
{
    auto itf = d_DrawBoardToolMgr()->_tools.find(tool);
    if (itf != d_DrawBoardToolMgr()->_tools.end()) {
        return itf.value();
    }
    return nullptr;
}

bool DrawBoardToolMgr::addTool(int tool)
{
    return addTool(DrawToolFactory::createTool(tool));
}

DrawBoard *DrawBoardToolMgr::drawBoard() const
{
    return d_DrawBoardToolMgr()->_db;
}

bool DrawBoardToolMgr::addTool(DrawTool *pTool)
{
    if (pTool == nullptr)
        return false;

    auto itf = d_DrawBoardToolMgr()->_tools.find(pTool->toolType());
    if (itf == d_DrawBoardToolMgr()->_tools.end()) {
        auto button = pTool->toolButton();
        button->setCheckable(true);
        d_DrawBoardToolMgr()->toolButtonGroup->addButton(button, pTool->toolType());
        d_DrawBoardToolMgr()->m_layout->addWidget(button);
        pTool->setParent(this);
        //pTool->setToolManager(this);
        button->show();
        d_DrawBoardToolMgr()->_tools.insert(pTool->toolType(), pTool);
        extern void DrawTool_SetToolManagerHelper(DrawTool * tool, DrawBoardToolMgr * pManager);
        DrawTool_SetToolManagerHelper(pTool, this);
        return true;
    } else {
        qWarning() << QString("there is one same type tool(type=%1),if you want replace it,please remove first.").arg(itf.key());
    }
    return false;
}

void DrawBoardToolMgr::removeTool(int tool)
{
    auto findit = d_DrawBoardToolMgr()->_tools.find(tool);
    if (findit != d_DrawBoardToolMgr()->_tools.end()) {
        removeTool(findit.value());
    }
}

void DrawBoardToolMgr::removeTool(DrawTool *tool)
{
    auto button = tool->toolButton();
    d_DrawBoardToolMgr()->toolButtonGroup->removeButton(button);
    d_DrawBoardToolMgr()->m_layout->removeWidget(button);
    d_DrawBoardToolMgr()->_tools.remove(tool->toolType());

    button->setParent(this);
    button->hide();
}

void DrawBoardToolMgr::addDefaultTools()
{
    addTool(DrawToolFactory::createTool(picture));
    addTool(DrawToolFactory::createTool(rectangle));
    addTool(DrawToolFactory::createTool(ellipse));
    addTool(DrawToolFactory::createTool(triangle));
    addTool(DrawToolFactory::createTool(polygonalStar));
    addTool(DrawToolFactory::createTool(polygon));
    addTool(DrawToolFactory::createTool(line));
    addTool(DrawToolFactory::createTool(text));
    addTool(DrawToolFactory::createTool(pen));
    addTool(DrawToolFactory::createTool(eraser));
    addTool(DrawToolFactory::createTool(blur));

//    addTool(DrawToolFactory::createTool(rectangle));
//    addTool(DrawToolFactory::createTool(ellipse));
//    addTool(DrawToolFactory::createTool(line));
//    addTool(DrawToolFactory::createTool(text));
//    //addTool(DrawToolFactory::createTool(blur));
//    addTool(DrawToolFactory::createTool(EUndoTool));
//    addTool(DrawToolFactory::createTool(ERedoTool));
//    addTool(DrawToolFactory::createTool(EClearTool));
//    addTool(DrawToolFactory::createTool(ESaveTool));
//    addTool(DrawToolFactory::createTool(ECloseTool));
    addPluginTools();
}

void DrawBoardToolMgr::addPluginTools()
{
    foreach (auto f, DrawToolFactory::genInterfacesForPluginTool()) {
        addTool(f());
    }
}

void DrawBoardToolMgr::clearAllTools(bool dlt)
{
    if (dlt) {
        foreach (auto t, d_DrawBoardToolMgr()->_tools) {
            delete t;
        }
    }
    d_DrawBoardToolMgr()->_tools.clear();
}

void DrawBoardToolMgr::changeButtonLayout(QLayout *layout)
{
    if (layout == d_DrawBoardToolMgr()->m_layout)
        return;

    if (d_DrawBoardToolMgr()->m_layout != nullptr) {
        delete d_DrawBoardToolMgr()->m_layout;
    }
    foreach (auto tool, d_DrawBoardToolMgr()->_tools) {
        layout->addWidget(tool->toolButton());
    }
    d_DrawBoardToolMgr()->m_layout = layout;
    this->layout()->addItem(d_DrawBoardToolMgr()->m_layout);
    this->layout()->setAlignment(d_DrawBoardToolMgr()->m_layout, Qt::AlignCenter);
}

QLayout *DrawBoardToolMgr::buttonLayout() const
{
    return d_DrawBoardToolMgr()->m_layout;
}

QMap<int, DrawTool *> DrawBoardToolMgr::allTools() const
{
    return d_DrawBoardToolMgr()->_tools;
}

void DrawBoardToolMgr::mouseMoveEvent(QMouseEvent *event)
{
    //禁止拖动
    Q_UNUSED(event)
}

void DrawBoardToolMgr::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
}

void DrawBoardToolMgr::paintEvent(QPaintEvent *event)
{
    //return QWidget::paintEvent(event);
    //DFrame::paintEvent(event);s
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(palette().background().color());
    painter.setRenderHint(QPainter::Antialiasing);
    //painter.drawRoundRect(rect(), 10, 60);
//    QPainterPath path;
//    path.addRoundedRect(rect(), 20, 20);
//    painter.drawPath(path);
    painter.end();
}



