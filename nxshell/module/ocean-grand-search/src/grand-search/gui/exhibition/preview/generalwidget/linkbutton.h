#ifndef LINKBUTTON
#define LINKBUTTON

#include <DCommandLinkButton>

namespace GrandSearch {

// 透明背景 icon可左可右
class LinkButton : public DTK_WIDGET_NAMESPACE::DCommandLinkButton
{
    Q_OBJECT
public:
    explicit LinkButton(const QString& text, QWidget * parent = nullptr) : DTK_WIDGET_NAMESPACE::DCommandLinkButton(text, parent) {}
    QSize sizeHint() const override;
};
}

#endif // LINKBUTTON
