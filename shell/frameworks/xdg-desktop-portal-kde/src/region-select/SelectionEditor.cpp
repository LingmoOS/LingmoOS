#include "SelectionEditor.h"

#include <KLocalizedContext>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickWindow>
#include <QScreen>

class SelectionEditorPrivate
{
public:
    SelectionEditorPrivate(SelectionEditor *q);
    ~SelectionEditorPrivate();

    SelectionEditor *const q;

    void beginDragOperation(const QPointF &pos);
    void endDragOperation();
    void reset();

    bool setMouseCursor(const QPointF &pos);

    bool isDragging() const;

    QRectF rect() const;

private:
    QPointF startPosition;
    bool dragging = false;
    QRectF selectedRegion;
};

SelectionEditorPrivate::SelectionEditorPrivate(SelectionEditor *q)
    : q(q)
{
}

SelectionEditorPrivate::~SelectionEditorPrivate()
{
}

void SelectionEditorPrivate::beginDragOperation(const QPointF &pos)
{
    dragging = true;
    startPosition = pos;
}

void SelectionEditorPrivate::endDragOperation()
{
    dragging = false;
}

void SelectionEditorPrivate::reset()
{
    endDragOperation();
    startPosition = {0.f, 0.f};
    selectedRegion = {0.f, 0.f, 0.f, 0.f};
}

bool SelectionEditorPrivate::setMouseCursor(const QPointF &pos)
{
    if (dragging) {
        selectedRegion =
            QRectF(qMin(startPosition.x(), pos.x()), qMin(startPosition.y(), pos.y()), qAbs(pos.x() - startPosition.x()), qAbs(pos.y() - startPosition.y()));

        return true;
    }

    return false;
}

bool SelectionEditorPrivate::isDragging() const
{
    return dragging;
}

QRectF SelectionEditorPrivate::rect() const
{
    return selectedRegion;
}

SelectionEditor::SelectionEditor(QObject *parent)
    : QObject(parent)
    , d(new SelectionEditorPrivate(this))
    , m_engine(new QQmlApplicationEngine(this))
{
    auto context = new KLocalizedContext(m_engine);
    context->setTranslationDomain(TRANSLATION_DOMAIN);
    m_engine->rootContext()->setContextObject(context);
    m_engine->rootContext()->setContextProperty(QStringLiteral("SelectionEditor"), QVariant::fromValue<QObject *>(this));

    setObjectName(QStringLiteral("selectionEditor"));

    showViews();
    connect(qApp, &QGuiApplication::screenAdded, this, &SelectionEditor::showViews);
    connect(qApp, &QGuiApplication::screenRemoved, this, &SelectionEditor::showViews);
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &SelectionEditor::showViews);
    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow *newWindow) {
        bool focusLost = true;

        if (newWindow) {
            for (auto *view : m_views) {
                if (view && newWindow->winId() == view->winId()) {
                    focusLost = false;
                    break;
                }
            }
        }
        if (focusLost) {
            this->reject();
        }
    });
}

void SelectionEditor::showViews()
{
    // delete existing views, since this function
    // may be called when workspace geometry changes
    for (auto *view : m_views) {
        view->deleteLater();
    }

    for (auto *screen : qApp->screens()) {
        auto view = new QQuickView(m_engine, nullptr);

        view->setScreen(screen);

        view->setSource(QUrl(QStringLiteral("qrc:/region-select/RegionSelectOverlay.qml")));
        view->installEventFilter(this);

        view->setResizeMode(QQuickView::SizeRootObjectToView);
        view->setColor(Qt::transparent);
        view->setFlags({
            Qt::Window, // the default window flag
            Qt::FramelessWindowHint,
            Qt::NoDropShadowWindowHint,
            Qt::MaximizeUsingFullscreenGeometryHint, // also use the areas where system UIs are
        });
        view->setWindowStates(Qt::WindowFullScreen);
        view->setVisibility(QWindow::Visibility::FullScreen);

        m_views << view;
    }
}

bool SelectionEditor::isDragging() const
{
    return d->isDragging();
}

QRect SelectionEditor::rect() const
{
    return d->rect().toRect();
}

bool SelectionEditor::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    default:
        break;
    }
    return false;
}

void SelectionEditor::dragStart(const QString &screenName, int x, int y)
{
    const auto screens = QGuiApplication::screens();
    auto screenIt = std::find_if(screens.begin(), screens.end(), [&screenName](auto screen) {
        return screen->name() == screenName;
    });
    d->beginDragOperation((*screenIt)->geometry().topLeft() + QPoint{x, y});
    Q_EMIT isDraggingChanged();
}

void SelectionEditor::setMousePosition(const QString &screenName, int x, int y)
{
    const auto screens = QGuiApplication::screens();
    auto screenIt = std::find_if(screens.begin(), screens.end(), [&screenName](auto screen) {
        return screen->name() == screenName;
    });
    if (d->setMouseCursor((*screenIt)->geometry().topLeft() + QPoint{x, y})) {
        Q_EMIT rectChanged();
    }
}

void SelectionEditor::dragRelease(const QString &screenName, int x, int y)
{
    const auto screens = QGuiApplication::screens();
    auto screenIt = std::find_if(screens.begin(), screens.end(), [&screenName](auto screen) {
        return screen->name() == screenName;
    });
    if (d->isDragging() && d->setMouseCursor((*screenIt)->geometry().topLeft() + QPoint{x, y})) {
        d->endDragOperation();
        Q_EMIT isDraggingChanged();

        // portal will error out if we try to stream an invalid region
        if (d->rect().isValid()) {
            accept();
        }
    }
}

void SelectionEditor::dragReset()
{
    d->reset();
    Q_EMIT rectChanged();
    Q_EMIT isDraggingChanged();
}

void SelectionEditor::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        reject();
        event->accept();
        break;
    default:
        break;
    }
}

SelectionEditor::~SelectionEditor() noexcept
{
    for (auto *view : m_views) {
        view->deleteLater();
    }
    delete d;
    delete m_engine;
}

bool SelectionEditor::exec()
{
    return m_execLoop.exec() == 0;
}

void SelectionEditor::accept()
{
    m_execLoop.quit();
}
void SelectionEditor::reject()
{
    m_execLoop.exit(1);
}