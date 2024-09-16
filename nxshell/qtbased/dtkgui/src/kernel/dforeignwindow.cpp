// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dforeignwindow.h"
#include <DObjectPrivate>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformintegration.h>

#include <QEvent>
#include <QDynamicPropertyChangeEvent>
#include <QDebug>

DGUI_BEGIN_NAMESPACE

#define DEFINE_CONST_CHAR(Name) const char _##Name[] = "_d_" #Name

// propertys
DEFINE_CONST_CHAR(WmClass);
DEFINE_CONST_CHAR(ProcessId);

class DForeignWindowPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    explicit DForeignWindowPrivate(DForeignWindow *qq)
        : DObjectPrivate(qq) {}
};

/*!
  \class Dtk::Gui::DForeignWindow
  \inmodule dtkgui

  \brief 一个用于获取本地窗口信息的类.

  继承于 QWindow，支持 QWindow::geometry
  QWindow::x QWindow::y QWindow::width QWindow::height
  QWindow::title QWindow::flags QWindow::visibility QWindow::type
  QWindow::windowStates QWindow::windowState 等接口的使用，另外扩展
  增加了一部分接口，方面更加详细的获取窗口信息。依赖于 dxcb 插件，在未加载
  dxcb 插件的应用中使用时结果未知

  \sa DWindowManagerHelper::currentWorkspaceWindows
  \sa Dtk::Widget::DApplication::loadDXcbPlugin
  \sa Dtk::Widget::DApplication::isDXcbPlatform
 */

/*!
  \property DForeignWindow::wmClass
  \brief 窗口 WM_CLASS 的值
  \note 只读
  \ref {https://tronche.com/gui/x/icccm/sec-4.html#WM_CLASS}{WM_CLASS}
 */

/*!
  \property DForeignWindow::pid
  \brief 窗口所属进程的 pid
  \note 只读
  \ref {https://specifications.freedesktop.org/wm-spec/1.3/ar01s05.html}{_NET_WM_PID}
 */

/*!
  \fn void DForeignWindow::wmClassChanged()
  \brief 信号会在 wmClass 属性改变时被发送.
  \sa DForeignWindow::wmClass
 */

/*!
  \fn void DForeignWindow::pidChanged()
  \brief 信号会在 pid 属性的值改变时被发送.
  \sa DForeignWindow::pid
 */

/*!
  \brief 直接构造一个 DForeignWindow 对象和使用 QWindow 对象没有区别.

  \a parent
  \sa DForeignWindow::fromWinId
 */
DForeignWindow::DForeignWindow(QWindow *parent)
    : QWindow(parent)
    , DObject(*new DForeignWindowPrivate(this))
{

}

/*!
  \brief DForeignWindow::fromWinId.
  使用这个窗口id创建一个 DForeignWindow 对象，此对象不会被加到 QGuiApplication::allWindows
  中。一般应用在需要获取一个本地窗口信息的场景。示例：
  \code
  // a.cpp
  int main(int argc, char *argv[])
  {
      DApplication a(argc, argv);
  
      QWidget w;
  
      w.setWindowTitle("deepin");
      w.show();
  
      QFile app_win_id("/tmp/window_id.txt");
      if (app_win_id.open(QFile::WriteOnly)) {
          app_win_id.write(QByteArray::number(w.winId()));
          app_win_id.close();
      }
  
      return a.exec();
  }
  \endcode
  
  \code
  // b.cpp
  int main(int argc, char *argv[])
  {
      DApplication::loadDXcbPlugin();
      DApplication a(argc, argv);
  
      DForeignWindow *fw = nullptr;
      QFile app_win_id("/tmp/window_id.txt");
      if (app_win_id.open(QFile::ReadOnly)) {
          fw = DForeignWindow::fromWinId(app_win_id.readAll().toInt());
      }
  
      if (fw) {
          qDebug() << fw->title();
  
          fw->connect(fw, &DForeignWindow::widthChanged, [&] {
              qDebug() << fw->width();
          });
      }
  
      return a.exec();
  }
  \endcode
  
  先启动应用 a
  再启动应用 b
  
  在应用 b 启动后将看到如下输出：
  \code
  "deepin"
  \endcode
  当改变应用 a 中的窗口宽度时，在应用 b 中会看到宽度的输出
  \a id
  \return
  \warning 不要尝试对由本应用创建的窗口调用此接口，可能会导致窗口行为发生不可逆转的变化
 */
DForeignWindow *DForeignWindow::fromWinId(WId id)
{
    if (!QGuiApplicationPrivate::platformIntegration()->hasCapability(QPlatformIntegration::ForeignWindows)) {
        qWarning() << "DForeignWindow::fromWinId(): platform plugin does not support foreign windows.";
        return 0;
    }

    DForeignWindow *window = new DForeignWindow;
    window->setFlags(Qt::ForeignWindow);
    window->setProperty("_q_foreignWinId", QVariant::fromValue(id));
    window->create();
    return window;
}

QString DForeignWindow::wmClass() const
{
    return property(_WmClass).toString();
}

quint32 DForeignWindow::pid() const
{
    return qvariant_cast<quint32>(property(_ProcessId));
}

bool DForeignWindow::event(QEvent *e)
{
    if (e->type() == QEvent::DynamicPropertyChange) {
        QDynamicPropertyChangeEvent *event = static_cast<QDynamicPropertyChangeEvent*>(e);

        if (event->propertyName() == _WmClass) {
            Q_EMIT wmClassChanged();

            return true;
        } else if (event->propertyName() == _ProcessId) {
            Q_EMIT pidChanged();

            return true;
        }
    }

    return false;
}

DGUI_END_NAMESPACE
