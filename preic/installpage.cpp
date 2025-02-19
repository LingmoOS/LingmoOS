#include "installpage.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QEvent>
#include "translator.h"

InstallPage::InstallPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    connect(Translator::instance(), &Translator::languageChanged,
            this, &InstallPage::updateText);
}

void InstallPage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(40);
    layout->setContentsMargins(80, 60, 80, 60);
    
    titleLabel = new QLabel(tr("Installation Options"), this);
    titleLabel->setStyleSheet("font-size: 42px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel(tr("Choose how you want to proceed"), this);
    subtitleLabel->setStyleSheet("font-size: 20px; color: #cdd6f4;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // 选项容器
    QWidget *optionsContainer = new QWidget(this);
    optionsContainer->setStyleSheet("background-color: #313244; border-radius: 15px; padding: 30px;");
    QHBoxLayout *optionsLayout = new QHBoxLayout(optionsContainer);
    optionsLayout->setSpacing(40);
    
    // 安装选项
    QWidget *installWidget = new QWidget(this);
    installWidget->setStyleSheet("QWidget { background-color: #1e1e2e; border-radius: 10px; }");
    QVBoxLayout *installLayout = new QVBoxLayout(installWidget);
    installLayout->setSpacing(15);
    
    QLabel *installIcon = new QLabel(this);
    installIcon->setPixmap(QPixmap(":/images/install.svg").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    installIcon->setAlignment(Qt::AlignCenter);
    
    installTitle = new QLabel(tr("Install Lingmo Linux"), this);
    installTitle->setStyleSheet("font-size: 24px; font-weight: bold;");
    installTitle->setAlignment(Qt::AlignCenter);
    
    installDesc = new QLabel(tr("Install the system on your computer"), this);
    installDesc->setStyleSheet("font-size: 16px; color: #cdd6f4;");
    installDesc->setAlignment(Qt::AlignCenter);
    installDesc->setWordWrap(true);
    
    installLayout->addWidget(installIcon);
    installLayout->addWidget(installTitle);
    installLayout->addWidget(installDesc);
    
    // 试用选项（类似的设置）
    QWidget *tryWidget = new QWidget(this);
    QVBoxLayout *tryLayout = new QVBoxLayout(tryWidget);
    QLabel *tryIcon = new QLabel(this);
    tryIcon->setPixmap(QPixmap(":/images/systemlogo.svg").scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    tryText = new QLabel(tr("Try Lingmo Linux"), this);
    tryText->setStyleSheet("font-size: 24px; font-weight: bold;");
    tryText->setAlignment(Qt::AlignCenter);
    tryDesc = new QLabel(tr("Try without installing"), this);
    tryDesc->setStyleSheet("font-size: 16px; color: #cdd6f4;");
    tryDesc->setAlignment(Qt::AlignCenter);
    tryDesc->setWordWrap(true);
    
    tryLayout->addWidget(tryIcon);
    tryLayout->addWidget(tryText);
    tryLayout->addWidget(tryDesc);
    
    optionsLayout->addWidget(installWidget);
    optionsLayout->addWidget(tryWidget);
    
    // 主布局
    layout->addStretch();
    layout->addWidget(titleLabel);
    layout->addWidget(subtitleLabel);
    layout->addWidget(optionsContainer);
    layout->addStretch();
    
    // 修改选项卡悬停效果的实现
    auto addHoverAnimation = [](QWidget *widget) {
        widget->setProperty("hovered", false);
        widget->setCursor(Qt::PointingHandCursor);
        
        // 创建事件过滤器对象
        class HoverFilter : public QObject
        {
        public:
            explicit HoverFilter(QWidget *target) : QObject(target), widget(target) {}
            
            bool eventFilter(QObject *watched, QEvent *event) override {
                if (event->type() == QEvent::Enter) {
                    QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry");
                    anim->setDuration(200);
                    QRect geo = widget->geometry();
                    anim->setStartValue(geo);
                    anim->setEndValue(geo.adjusted(-5, -5, 5, 5));
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                    return true;
                }
                if (event->type() == QEvent::Leave) {
                    QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry");
                    anim->setDuration(200);
                    QRect geo = widget->geometry();
                    anim->setStartValue(geo);
                    anim->setEndValue(geo.adjusted(5, 5, -5, -5));
                    anim->start(QAbstractAnimation::DeleteWhenStopped);
                    return true;
                }
                return QObject::eventFilter(watched, event);
            }
            
        private:
            QWidget *widget;
        };
        
        widget->installEventFilter(new HoverFilter(widget));
    };
    
    addHoverAnimation(installWidget);
    addHoverAnimation(tryWidget);
}

void InstallPage::updateText()
{
    titleLabel->setText(tr("Installation Options"));
    subtitleLabel->setText(tr("Choose how you want to proceed"));
    installTitle->setText(tr("Install Lingmo OS"));
    installDesc->setText(tr("Install the system on your computer"));
    tryText->setText(tr("Try Lingmo OS"));
    tryDesc->setText(tr("Try without installing"));
}