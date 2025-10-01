#include <QObject>
#include <QPoint>
#include <QCursor>

class MouseHelper : public QObject {
    Q_OBJECT
public:
    explicit MouseHelper(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QPoint cursorPos() const {
        return QCursor::pos();
    }
};