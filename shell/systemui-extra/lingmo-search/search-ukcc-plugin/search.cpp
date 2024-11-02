/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#include "search.h"
#include <unistd.h>
#include <QTranslator>
#include <QApplication>
#include <QString>
#include <QProcess>

static const QByteArray LINGMO_SEARCH_SCHEMAS = QByteArrayLiteral("org.lingmo.search.settings");
static const QString SEARCH_METHOD_KEY = QStringLiteral("fileIndexEnable");
static const QString WEB_ENGINE_KEY = QStringLiteral("webEngine");
static const QString CONTENT_FUZZY_SEARCH_KEY = QStringLiteral("contentFuzzySearch");
static const QString CONTENT_INDEX_ENABLE_KEY = QStringLiteral("contentIndexEnable");
static const QString CONTENT_INDEX_ENABLE_OCR_KEY = QStringLiteral("contentIndexEnableOcr");
static const QString AI_INDEX_ENABLE_KEY = QStringLiteral("aiIndexEnable");

bool Search::s_failed = false;
DataManagementSession Search::s_session = nullptr;

Search::Search()
{
    QTranslator* translator = new QTranslator(this);
    if(!translator->load("/usr/share/lingmo-search/search-ukcc-plugin/translations/" + QLocale::system().name())) {
        qWarning() << "/usr/share/lingmo-search/search-ukcc-plugin/translations/" + QLocale::system().name() << "load failed";
    }
    QApplication::installTranslator(translator);

    m_pluginName = tr("Search");
    m_pluginType = SEARCH_F;

    m_interface = new QDBusInterface("com.lingmo.search.fileindex.service",
                                     "/org/lingmo/search/privateDirWatcher",
                                     "org.lingmo.search.fileindex",
                                     QDBusConnection::sessionBus(),
                                     this);

    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(indexDirsChanged()), this, SLOT(refreshSearchDirsUi()));
    }

    m_setSearchDirInterface = new QDBusInterface("com.lingmo.search.fileindex.service",
                                                 "/org/lingmo/search/fileindex",
                                                 "org.lingmo.search.fileindex",
                                                 QDBusConnection::sessionBus(),
                                                 this);

    const QByteArray id(LINGMO_SEARCH_SCHEMAS);
    if (QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id, QByteArray(), this);
    } else {
        qCritical() << LINGMO_SEARCH_SCHEMAS << " not installed!\n";
    }
    m_lastBlockDialogDir = QDir::homePath();
    m_lastSearchDialogDir = QDir::homePath();
}

QString Search::plugini18nName()
{
    return m_pluginName;
}

int Search::pluginTypes()
{
    return m_pluginType;
}

QWidget *Search::pluginUi()
{
    initAiSettings();
    initUi();
    initSearchDirs();
    initBlockDirsList();
    connect(m_addSearchDirBtn, &QPushButton::clicked, this, &Search::onAddSearchDirBtnClicked);
    connect(m_addBlockDirWidget, &QPushButton::clicked, this, &Search::onBtnAddBlockFolderClicked);
    connect(m_getModelBtn, &QPushButton::clicked, this, &Search::onGetAiModelBtnClicked);

    if (m_gsettings) {
        //按钮状态初始化
        if (m_gsettings->keys().contains(SEARCH_METHOD_KEY)) {
            bool fileIndexOn = m_gsettings->get(SEARCH_METHOD_KEY).toBool();
            m_fileIndexBtn->setChecked(fileIndexOn);
        } else {
            m_fileIndexBtn->setEnabled(false);
        }

        if (m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
            //当前是否开启内容索引
            bool isContentIndexOn= m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
            m_contentIndexBtn->setChecked(isContentIndexOn);
            if (isContentIndexOn) {
                m_indexSetFrame->show();
            }
        } else {
            m_contentIndexBtn->setEnabled(false);
        }

        if (m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
            m_ocrSwitchBtn->setChecked(m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool());
        } else {
            m_ocrSwitchBtn->setEnabled(false);
        }

        if (m_gsettings->keys().contains(AI_INDEX_ENABLE_KEY) && isModelReady) {
            bool aiIndexOn = m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();
            m_aiIndexBtn->setChecked(aiIndexOn);
        } else {
            m_aiIndexBtn->setEnabled(false);
        }

        if (m_gsettings->keys().contains(WEB_ENGINE_KEY)) {
            //当前网页搜索的搜索引擎
            QString engine = m_gsettings->get(WEB_ENGINE_KEY).toString();
            m_webEngineFrame->mCombox->setCurrentIndex(m_webEngineFrame->mCombox->findData(engine));
        } else {
            m_webEngineFrame->mCombox->setEnabled(false);
        }

        if (m_gsettings->keys().contains(CONTENT_FUZZY_SEARCH_KEY)) {
            //是否为模糊搜索
            bool isFuzzy = m_gsettings->get(CONTENT_FUZZY_SEARCH_KEY).toBool();
            if (isFuzzy) {
                m_fuzzyBtn->setChecked(true);
            } else {
                m_preciseBtn->setChecked(true);
            }
        } else {
            m_fuzzyBtn->setEnabled(false);
            m_preciseBtn->setEnabled(false);
        }
        //监听gsettings值改变，更新控制面板UI
        connect(m_gsettings, &QGSettings::changed, this, [ = ](const QString &key) {
            if (key == SEARCH_METHOD_KEY) {
                bool isFileIndexOn = m_gsettings->get(SEARCH_METHOD_KEY).toBool();
                m_fileIndexBtn->blockSignals(true);
                m_fileIndexBtn->setChecked(isFileIndexOn);
                m_fileIndexBtn->blockSignals(false);
            } else if (key == CONTENT_INDEX_ENABLE_KEY) {
                bool isContentIndexOn = m_gsettings->get(CONTENT_INDEX_ENABLE_KEY).toBool();
                m_contentIndexBtn->blockSignals(true);
                m_contentIndexBtn->setChecked(isContentIndexOn);
                m_contentIndexBtn->blockSignals(false);
                if (isContentIndexOn) {
                    m_indexSetFrame->show();
                } else {
                    m_indexSetFrame->hide();
                    if (m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                        m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, false);
                    }
                }
            } else if (key == CONTENT_INDEX_ENABLE_OCR_KEY) {
                m_ocrSwitchBtn->blockSignals(true);
                m_ocrSwitchBtn->setChecked(m_gsettings->get(CONTENT_INDEX_ENABLE_OCR_KEY).toBool());
                m_ocrSwitchBtn->blockSignals(false);
            } else if (key == AI_INDEX_ENABLE_KEY) {
                bool isAiIndexOn = m_gsettings->get(AI_INDEX_ENABLE_KEY).toBool();
                m_aiIndexBtn->blockSignals(true);
                m_aiIndexBtn->setChecked(isAiIndexOn);
                m_aiIndexBtn->blockSignals(false);
            } else if (key == WEB_ENGINE_KEY) {
                QString engine = m_gsettings->get(WEB_ENGINE_KEY).toString();
                m_webEngineFrame->mCombox->blockSignals(true);
                m_webEngineFrame->mCombox->setCurrentIndex(m_webEngineFrame->mCombox->findData(engine));
                m_webEngineFrame->mCombox->blockSignals(false);
            } else if (key == CONTENT_FUZZY_SEARCH_KEY) {
                bool isFuzzy = m_gsettings->get(CONTENT_FUZZY_SEARCH_KEY).toBool();
                if (isFuzzy) {
                    m_fuzzyBtn->setChecked(true);
                } else {
                    m_preciseBtn->setChecked(true);
                }
            }
        });

        connect(m_fileIndexBtn, &kdk::KSwitchButton::stateChanged, this, [ = ](bool checked) {
            if (m_gsettings && m_gsettings->keys().contains(SEARCH_METHOD_KEY)) {
                m_gsettings->set(SEARCH_METHOD_KEY, checked);
            }
        });
        connect(m_contentIndexBtn, &kdk::KSwitchButton::pressed, this, [ & ] {
            if (!m_contentIndexBtn->isChecked()) {
                QMessageBox msgBox(m_pluginWidget);
                msgBox.setModal(true);
                msgBox.setIcon(QMessageBox::Warning);
                msgBox.setText(tr("Create file content index will increase the usage of memory and CPU, do you still want to open it?"));
                auto button = msgBox.addButton(tr("OK"),QMessageBox::AcceptRole);
                msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
                msgBox.exec();
                if (msgBox.clickedButton() == button) {
                    m_contentIndexBtn->setChecked(true);
                }
            }
        });

        connect(m_contentIndexBtn, &kdk::KSwitchButton::stateChanged, this, [ = ](bool checked) {
            if (m_gsettings && m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_KEY)) {
                m_gsettings->set(CONTENT_INDEX_ENABLE_KEY, checked);
            }
        });
        connect(m_ocrSwitchBtn, &kdk::KSwitchButton::stateChanged, this, [ = ](bool checked) {
            if (m_gsettings && m_gsettings->keys().contains(CONTENT_INDEX_ENABLE_OCR_KEY)) {
                m_gsettings->set(CONTENT_INDEX_ENABLE_OCR_KEY, checked);
            }
        });
        connect(m_indexMethodBtnGroup, QOverload<int, bool>::of(&QButtonGroup::buttonToggled),[ = ](int id, bool checked) {
            if (id == -3) {//fuzzyBtn's id
                if (m_gsettings && m_gsettings->keys().contains(CONTENT_FUZZY_SEARCH_KEY)) {
                    m_gsettings->set(CONTENT_FUZZY_SEARCH_KEY, checked);
                }
            }
        });
        connect(m_aiIndexBtn, &kdk::KSwitchButton::stateChanged, this, [ = ](bool checked) {
            if (m_gsettings && m_gsettings->keys().contains(AI_INDEX_ENABLE_KEY)) {
                m_gsettings->set(AI_INDEX_ENABLE_KEY, checked);
            }
        });
        connect(m_webEngineFrame->mCombox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] {
            if (m_gsettings && m_gsettings->keys().contains(WEB_ENGINE_KEY)) {
                m_gsettings->set(WEB_ENGINE_KEY, m_webEngineFrame->mCombox->currentData().toString());
            }
        });
    } else {
        qCritical() << "Gsettings of the search plugin for ukcc is not initialized!";
        m_contentIndexBtn->setEnabled(false);
        m_webEngineFrame->mCombox->setEnabled(false);
    }

    return m_pluginWidget;
}

const QString Search::name() const
{
    return QStringLiteral("Search");
}

QString Search::translationPath() const
{
    return QStringLiteral("/usr/share/lingmo-search/search-ukcc-plugin/translations/%1.ts");
}

bool Search::isShowOnHomePage() const
{
    return true;
}

QIcon Search::icon() const
{
    return QIcon::fromTheme("search-symbolic");
}

bool Search::isEnable() const
{
    return true;
}

/**
 * @brief Search::initUi 初始化此插件UI
 */
void Search::initUi()
{
    m_pluginWidget = new QWidget;
    m_pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
    m_mainLyt = new QVBoxLayout(m_pluginWidget);
    m_pluginWidget->setLayout(m_mainLyt);

    m_titleLabel = new TitleLabel(m_pluginWidget);
    //~ contents_path /Search/Search
    m_titleLabel->setText(tr("Search"));
    m_mainLyt->addWidget(m_titleLabel);

    //设置网页搜索引擎部分的ui
    //~ contents_path /Search/Default web searching engine
    m_webEngineFrame = new ComboxFrame(tr("Default web searching engine"), m_pluginWidget);
    m_webEngineFrame->setContentsMargins(8, 0, 8, 0);// ComboxFrame右侧自带8的边距，左右边距各是16所以分别设为8
    m_webEngineFrame->setFixedHeight(56);
    m_webEngineFrame->setMinimumWidth(550);
    m_webEngineFrame->mCombox->insertItem(0, QIcon("/usr/share/lingmo-search/search-ukcc-plugin/image/baidu.svg"), tr("baidu"), "baidu");
    m_webEngineFrame->mCombox->insertItem(1, QIcon("/usr/share/lingmo-search/search-ukcc-plugin/image/sougou.svg"), tr("sougou"), "sougou");
    m_webEngineFrame->mCombox->insertItem(2, QIcon("/usr/share/lingmo-search/search-ukcc-plugin/image/360.svg"), tr("360"), "360");
    m_mainLyt->addWidget(m_webEngineFrame);

    //设置索引开关
    m_indexTitleLabel = new TitleLabel(m_pluginWidget);
    //~ contents_path /Search/Create index
    m_indexTitleLabel->setText(tr("Create index"));
    m_mainLyt->addSpacing(32);
    m_mainLyt->addWidget(m_indexTitleLabel);

    m_fileIndexFrame = new QFrame(m_pluginWidget);
    m_fileIndexFrame->setFrameShape(QFrame::Shape::Box);
    m_fileIndexFrame->setFixedHeight(64);
    m_fileIndexLyt = new QHBoxLayout(m_fileIndexFrame);
    m_fileIndexLyt->setContentsMargins(16, 20, 16, 20);
    m_fileIndexFrame->setLayout(m_fileIndexLyt);

    m_fileIndexLabel = new QLabel(m_fileIndexFrame);
    //~ contents_path /Search/Create file index
    m_fileIndexLabel->setText(tr("Create file index"));
    m_fileIndexLabel->setContentsMargins(0, 0, 0, 0);
    m_fileIndexBtn = new kdk::KSwitchButton(m_fileIndexFrame);
    m_fileIndexLyt->addWidget(m_fileIndexLabel);
    m_fileIndexLyt->addStretch();
    m_fileIndexLyt->addWidget(m_fileIndexBtn);
    m_mainLyt->addWidget(m_fileIndexFrame);

    //设置内容索引部分的ui
    m_setFrame = new QFrame(m_pluginWidget);
    m_setFrame->setFrameShape(QFrame::Shape::Box);
    m_setFrameLyt = new QVBoxLayout(m_setFrame);
    m_setFrameLyt->setContentsMargins(0, 0, 0, 0);
    m_setFrameLyt->setSpacing(0);

    //索引开关UI
    m_contentIndexFrame = new QFrame(m_setFrame);
    m_contentIndexFrame->setMinimumWidth(550);
    m_contentIndexFrame->setFixedHeight(64);
    m_contentIndexLyt = new QHBoxLayout(m_contentIndexFrame);
    m_contentIndexLyt->setContentsMargins(16, 20, 16, 20);
    m_contentIndexFrame->setLayout(m_contentIndexLyt);

    m_contentIndexLabel = new QLabel(m_contentIndexFrame);
    //~ contents_path /Search/Create file content index
    m_contentIndexLabel->setText(tr("Create file content index"));
    m_contentIndexLabel->setContentsMargins(0, 0, 0, 0);
    m_contentIndexBtn = new kdk::KSwitchButton(m_contentIndexFrame);
    m_contentIndexLyt->addWidget(m_contentIndexLabel);
    m_contentIndexLyt->addStretch();
    m_contentIndexLyt->addWidget(m_contentIndexBtn);

    m_indexSetFrame = new QFrame(m_setFrame);
    m_indexSetFrame->setFixedHeight(228);
    m_indexSetLyt = new QVBoxLayout(m_indexSetFrame);
    m_indexSetLyt->setContentsMargins(0, 0, 0, 0);
    m_indexSetLyt->setSpacing(0);

    //分隔线
    QFrame *line = new QFrame(m_indexSetFrame);
    line->setFixedHeight(1);
    line->setContentsMargins(0, 0, 0, 0);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    //设置索引模式的ui
    m_indexMethodFrame = new QFrame(m_indexSetFrame);
    m_indexMethodFrame->setFixedHeight(103);
    m_indexMethodLyt = new QVBoxLayout(m_indexMethodFrame);
    m_indexMethodLyt->setContentsMargins(8, 0, 0, 0);// radiobutton本身左边有8间距
    m_indexMethodFrame->setLayout(m_indexMethodLyt);

    m_preciseBtnFrame = new QFrame(m_indexMethodFrame);
    m_preciseBtnFrame->setFixedHeight(50);
    m_preciseBtnLyt = new QHBoxLayout(m_preciseBtnFrame);
    m_preciseBtnFrame->setLayout(m_preciseBtnLyt);
    m_preciseBtn = new QRadioButton(tr("Precise Search"), m_indexMethodFrame);
    m_preciseDescLabel = new QLabel(m_indexMethodFrame);
    m_preciseDescLabel->setContentsMargins(0, 0, 0, 0);
    m_preciseDescLabel->setText(tr("show the results that exactly match the keyword"));
    m_preciseDescLabel->setEnabled(false);
    m_preciseBtnLyt->addWidget(m_preciseBtn);
    m_preciseBtnLyt->addWidget(m_preciseDescLabel);
    m_preciseBtnLyt->addStretch();

    m_fuzzyBtnFrame = new QFrame(m_indexMethodFrame);
    m_fuzzyBtnFrame->setFixedHeight(53);
    m_fuzzyBtnLyt = new QHBoxLayout(m_fuzzyBtnFrame);
    m_fuzzyBtnFrame->setLayout(m_fuzzyBtnLyt);
    m_fuzzyBtn = new QRadioButton(tr("Fuzzy Search"), m_indexMethodFrame);
    m_fuzzyDescLabel = new QLabel(m_indexMethodFrame);
    m_fuzzyDescLabel->setContentsMargins(0, 0, 0, 0);
    m_fuzzyDescLabel->setText(tr("show more results that match the keyword"));
    m_fuzzyDescLabel->setEnabled(false);
    m_fuzzyBtnLyt->addWidget(m_fuzzyBtn);
    m_fuzzyBtnLyt->addWidget(m_fuzzyDescLabel);
    m_fuzzyBtnLyt->addStretch();

    m_indexMethodBtnGroup = new QButtonGroup(m_indexSetFrame);
    m_indexMethodBtnGroup->addButton(m_preciseBtn);
    m_indexMethodBtnGroup->addButton(m_fuzzyBtn);
    m_indexMethodBtnGroup->setExclusive(true);

    m_indexMethodLyt->addWidget(m_preciseBtnFrame);
    m_indexMethodLyt->addWidget(m_fuzzyBtnFrame);
    m_indexMethodLyt->addSpacing(24);

    //第二条分隔线
    QFrame *ocrLine = new QFrame(m_indexSetFrame);
    ocrLine->setFixedHeight(1);
    ocrLine->setContentsMargins(0, 0, 0, 0);
    ocrLine->setLineWidth(0);
    ocrLine->setFrameShape(QFrame::HLine);
    ocrLine->setFrameShadow(QFrame::Sunken);

    //OCR开关
    m_ocrSwitchFrame = new QFrame(m_indexSetFrame);
    m_ocrSwitchFrame->setMinimumWidth(550);
    m_ocrSwitchFrame->setFixedHeight(60);
    m_ocrSwitchLyt = new QHBoxLayout(m_contentIndexFrame);
    m_ocrSwitchLyt->setContentsMargins(16, 15, 16, 21);
    m_ocrSwitchFrame->setLayout(m_ocrSwitchLyt);

    m_ocrSwitchLabel = new QLabel(m_ocrSwitchFrame);
    //~ contents_path /Search/Use OCR to index the text in pictures
    m_ocrSwitchLabel->setText(tr("Index the text in pictures"));
    m_ocrSwitchLabel->setContentsMargins(0, 0, 0, 0);
    m_ocrSwitchBtn = new kdk::KSwitchButton(m_ocrSwitchFrame);
    m_ocrSwitchLyt->addWidget(m_ocrSwitchLabel);
    m_ocrSwitchLyt->addStretch();
    m_ocrSwitchLyt->addWidget(m_ocrSwitchBtn);

    m_indexSetLyt->addWidget(line);
    m_indexSetLyt->addWidget(m_indexMethodFrame);
    m_indexSetLyt->addWidget(ocrLine);
    m_indexSetLyt->addWidget(m_ocrSwitchFrame);

    m_setFrameLyt->addWidget(m_contentIndexFrame);
    m_setFrameLyt->addWidget(m_indexSetFrame);
    m_indexSetFrame->hide();//默认隐藏，根据是否开索引来初始化

    m_mainLyt->addWidget(m_setFrame);

    //Ai索引开关
    m_aiIndexFrame = new QFrame(m_pluginWidget);
    m_aiIndexFrame->setFrameShape(QFrame::Shape::Box);
    m_aiIndexFrame->setFixedHeight(64);
    m_aiIndexLyt = new QHBoxLayout(m_aiIndexFrame);
    m_aiIndexLyt->setContentsMargins(16, 20, 16, 20);
    m_aiIndexFrame->setLayout(m_aiIndexLyt);

    m_aiIndexLabel = new QLabel(m_aiIndexFrame);
    //~ contents_path /Search/Create AI index
    m_aiIndexLabel->setText(tr("Create AI index"));
    m_aiIndexLabel->setContentsMargins(0, 0, 0, 0);
    m_getModelBtn = new kdk::KBorderlessButton(m_aiIndexFrame);
    if (isModelReady) {
        m_getModelBtn->setText(tr("AI model is ready"));
    } else {
        m_getModelBtn->setText(tr("AI model is not ready"));
    }

    m_getModelBtn->setCursor(QCursor(Qt::PointingHandCursor));
    m_aiIndexBtn = new kdk::KSwitchButton(m_aiIndexFrame);
    m_aiIndexLyt->addWidget(m_aiIndexLabel);
    m_aiIndexLyt->addStretch();
    m_aiIndexLyt->addWidget(m_getModelBtn);
    m_aiIndexLyt->addWidget(m_aiIndexBtn);
    m_mainLyt->addWidget(m_aiIndexFrame);

    //添加搜索目录部分ui
    m_searchDirTitleLabel = new TitleLabel(m_pluginWidget);
    //~ contents_path /Search/Search Folders
    m_searchDirTitleLabel->setText(tr("Search Folders"));

    m_searchDirDescLabel = new QLabel(m_pluginWidget);
    m_searchDirDescLabel->setContentsMargins(16, 0, 0, 0);    //TitleLabel自带16边距,QLabel需要自己设
    m_searchDirDescLabel->setEnabled(false);
    m_searchDirDescLabel->setWordWrap(true);
    m_searchDirDescLabel->setText(tr("Following folders will be searched. You can set it by adding and removing folders."));


    m_searchDirsFrame = new QFrame(m_pluginWidget);
    m_searchDirsFrame->setFrameShape(QFrame::Shape::Box);
    m_searchDirLyt = new QVBoxLayout(m_searchDirsFrame);
    m_searchDirsFrame->setLayout(m_searchDirLyt);
    m_searchDirLyt->setContentsMargins(0, 0, 0, 0);
    m_searchDirLyt->setSpacing(2);
    m_searchDirLyt->setDirection(QBoxLayout::BottomToTop);

    m_addSearchDirFrame = new QFrame(m_searchDirsFrame);
    m_addSearchDirFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_addSearchDirFrame->setFixedHeight(60);

    m_addSearchDirBtn = new AddBtn(m_addSearchDirFrame);
    m_searchDirLyt->addWidget(m_addSearchDirBtn);

    m_mainLyt->addSpacing(32);
    m_mainLyt->addWidget(m_searchDirTitleLabel);
    m_mainLyt->addWidget(m_searchDirDescLabel);
    m_mainLyt->addWidget(m_searchDirsFrame);

    //设置黑名单文件夹部分的ui
    m_blockDirTitleLabel = new TitleLabel(m_pluginWidget);

    //~ contents_path /Search/Block Folders
    m_blockDirTitleLabel->setText(tr("Block Folders"));
    m_blockDirDescLabel = new QLabel(m_pluginWidget);
    m_blockDirDescLabel->setContentsMargins(16, 0, 0, 0);    //TitleLabel自带16边距,QLabel需要自己设
    m_blockDirDescLabel->setEnabled(false);
    m_blockDirDescLabel->setWordWrap(true);
    m_blockDirDescLabel->setText(tr("Following folders will not be searched. You can set it by adding and removing folders."));

    m_blockDirsFrame = new QFrame(m_pluginWidget);
    m_blockDirsFrame->setFrameShape(QFrame::Shape::Box);
    m_blockDirsLyt = new QVBoxLayout(m_blockDirsFrame);
    m_blockDirsLyt->setDirection(QBoxLayout::BottomToTop);
    m_blockDirsFrame->setLayout(m_blockDirsLyt);
    m_blockDirsLyt->setContentsMargins(0, 0, 0, 0);
    m_blockDirsLyt->setSpacing(2);

    QFrame * m_addBlockDirFrame = new QFrame(m_blockDirsFrame);
    m_addBlockDirFrame->setFrameShape(QFrame::Shape::NoFrame);
    m_addBlockDirFrame->setFixedHeight(60);

    m_addBlockDirWidget = new AddBtn(m_addBlockDirFrame);

    m_blockDirsLyt->addWidget(m_addBlockDirWidget);

    m_mainLyt->addSpacing(32);
    m_mainLyt->addWidget(m_blockDirTitleLabel);
    m_mainLyt->addWidget(m_blockDirDescLabel);
    m_mainLyt->addWidget(m_blockDirsFrame);
    m_mainLyt->addStretch();
    m_mainLyt->setContentsMargins(0, 0, 0, 0);
}

void Search::refreshSearchDirsUi()
{
    qWarning() << "==========refreshUi!!!!";
    while (m_searchDirLyt->count() - 1) {
        QWidget *widget = m_searchDirLyt->itemAt(m_searchDirLyt->count() - 1)->widget();
        m_searchDirLyt->removeWidget(widget);
        widget->deleteLater();
    }
    initSearchDirs();
}

/**
 * @brief Search::getBlockDirs 从配置文件获取黑名单并将黑名单列表传入
 */
void Search::getBlockDirs()
{
    if (m_interface->isValid()) {
        QDBusReply<QStringList> reply = m_interface->call("blockDirsForUser");
        if (reply.isValid()) {
            m_blockDirs = reply.value();
        }
    }
}

/**
 * @brief Search::setBlockDir 尝试写入新的黑名单文件夹
 * @param dirPath 待添加到黑名单的文件夹路径
 * @param is_add 是否是在添加黑名单
 * @return 0成功 !0添加失败的错误代码
 */
int Search::setBlockDir(const QString &dirPath, const bool &is_add)
{
    if (!QFile::exists(dirPath)) {
        removeBlockDirFromList(dirPath);
        return ReturnCode::NotExists;
    }

    QStringList pathSections = dirPath.split("/");
    for (const QString &section : pathSections) {
        if (section.startsWith(".")) {
            return ReturnCode::Hidden;
        }
    }

    if (!m_interface->isValid()) {
        return ReturnCode::DirWatcherError;
    }

    if (!is_add) {
        //删除黑名单目录
        m_interface->call("removeBlockDirOfUser", dirPath);
        removeBlockDirFromList(dirPath);
        return ReturnCode::Successful;
    }

    QStringList oldBlockList = m_blockDirs;
    getBlockDirs();
    for (const QString& oldBlockDir : oldBlockList) {
        if (!m_blockDirs.contains(oldBlockDir)) {
            removeBlockDirFromList(oldBlockDir);
        }
    }

    for (QString dir : m_blockDirs) {
        if (dirPath == dir) {
            return ReturnCode::HasBeenBlocked;
        }

        if (dirPath.startsWith(dir + "/") || dir == "/") {
            return ReturnCode::Duplicated;
        }

        //有它的子文件夹已被添加，删除这些子文件夹
        if (dir.startsWith(dirPath + "/") || dirPath == "/") {
            removeBlockDirFromList(dir);
        }
    }
    m_interface->call("addBlockDirOfUser", dirPath);
    appendBlockDirToList(dirPath);
    return ReturnCode::Successful;
}

/**
 * @brief Search::initBlockDirsList 初始化黑名单列表
 */
void Search::initBlockDirsList()
{
    getBlockDirs();
    for (QString path: m_blockDirs) {
        if (QFileInfo(path).isDir() /*&& path.startsWith("home")*/) {
            appendBlockDirToList(path);
        }
    }
}

void Search::initSearchDirs()
{
    if (m_interface->isValid()) {
        QDBusReply<QStringList> reply = m_interface->call("currentIndexableDir");
        if (reply.isValid()) {
            for (const QString &path : reply.value()) {
                appendSearchDirToList(path);
            }
        } else {
            qCritical() << "Fail to call currentIndexableDir.";
        }
    } else {
        qCritical() << "fileindex dbus error:" << m_interface->lastError();
    }
}

int Search::setSearchDir(const QString &dirPath, const bool isAdd)
{
    QFileInfo info(dirPath);
    if (!(info.isExecutable() && info.isReadable())) {
        //路径不存在时从ui上删除
        if (!isAdd) {
            this->removeSearchDirFromList(dirPath);
        }
        return ReturnCode::PermissionDenied;
    }

    if (!m_setSearchDirInterface->isValid()) {
        return ReturnCode::DirWatcherError;
    }

    QStringList pathSections = dirPath.split("/");
    for (const QString &section : pathSections) {
        if (section.startsWith(".")) {
            return ReturnCode::Hidden;
        }
    }

    if (isAdd) {
        QDBusReply<QStringList> indexDirsRpl = m_interface->call("currentSearchDirs");
        QStringList indexDirs;
        if (indexDirsRpl.isValid()) {
            indexDirs = indexDirsRpl.value();
        }

        QDBusReply<int> appendIndexRpl = m_setSearchDirInterface->call("appendSearchDir", dirPath);
        if (appendIndexRpl.isValid()) {
            if (appendIndexRpl.value() == 0) {
                this->appendSearchDirToList(dirPath);
                if (!indexDirs.isEmpty()) {
                    indexDirsRpl = m_interface->call("currentSearchDirs");
                    if (indexDirsRpl.isValid() && (indexDirsRpl.value().size() < indexDirs.size() + 1)) {
                        QStringList dirsAfterAppend = indexDirsRpl.value();
                        for (const QString& dir : indexDirs) {
                            if (!dirsAfterAppend.contains(dir)) {
                                this->removeSearchDirFromList(dir);
                            }
                        }
                    }
                }
            }
            return appendIndexRpl.value();
        }
    } else {
        this->removeSearchDirFromList(dirPath);
        m_setSearchDirInterface->call("removeSearchDir", dirPath);
    }

    return ReturnCode::Successful;
}

void Search::appendSearchDirToList(const QString &path)
{
    HoverWidget * dirWidget = new HoverWidget(path, m_searchDirsFrame);
    dirWidget->setObjectName(path);
    dirWidget->setMinimumWidth(550);
    QHBoxLayout * dirWidgetLyt = new QHBoxLayout(dirWidget);
    dirWidgetLyt->setSpacing(8);
    dirWidgetLyt->setContentsMargins(0, 0, 0, 0);
    dirWidget->setLayout(dirWidgetLyt);
    QFrame * dirFrame = new QFrame(dirWidget);
    dirFrame->setFrameShape(QFrame::Shape::Box);
    dirFrame->setFixedHeight(50);
    QHBoxLayout * dirFrameLayout = new QHBoxLayout(dirFrame);
    dirFrameLayout->setSpacing(16);
    dirFrameLayout->setContentsMargins(16, 0, 16, 0);
    QLabel * iconLabel = new QLabel(dirFrame);
    QLabel * pathLabel = new QLabel(dirFrame);

    dirFrameLayout->addWidget(iconLabel);
    iconLabel->setPixmap(QIcon::fromTheme("inode-directory").pixmap(QSize(24, 24)));
    pathLabel->setText(path);
    dirFrameLayout->addWidget(pathLabel);
    dirFrameLayout->addStretch();
    QPushButton * delBtn = new QPushButton(dirFrame);
    delBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    delBtn->setProperty("useButtonPalette", true);
    delBtn->setFixedSize(30, 30);
    delBtn->setToolTip(tr("delete"));
    delBtn->setFlat(true);


    delBtn->hide();
    dirFrameLayout->addWidget(delBtn);
    dirWidgetLyt->addWidget(dirFrame);

    QFrame *line = new QFrame(dirWidget);
    line->setObjectName(path);
    line->setFixedHeight(1);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    m_searchDirLyt->addWidget(line);
    m_searchDirLyt->addWidget(dirWidget);
    connect(delBtn, &QPushButton::clicked, this, [ = ]() {
        setSearchDir(path, false);
    });
    connect(dirWidget, &HoverWidget::enterWidget, this, [ = ]() {
        delBtn->show();
    });
    connect(dirWidget, &HoverWidget::leaveWidget, this, [ = ]() {
        delBtn->hide();
    });
}

void Search::removeSearchDirFromList(const QString &path)
{
    HoverWidget *delDirWidget = m_searchDirsFrame->findChild<HoverWidget *>(path);
    if (delDirWidget) {
        m_searchDirLyt->removeWidget(delDirWidget);
        delDirWidget->deleteLater();
        qDebug() << "Delete folder of search succeed! path = " << path;
    }
    QFrame *line = m_searchDirsFrame->findChild<QFrame*>(path);
    if (line) {
        m_searchDirLyt->removeWidget(line);
        line->deleteLater();
        qDebug()<< "Delete line of search folder:" << path;
    }
}

void Search::appendBlockDirToList(const QString &path)
{
    HoverWidget * dirWidget = new HoverWidget(path, m_blockDirsFrame);
    dirWidget->setObjectName(path);
    dirWidget->setMinimumWidth(550);
    QHBoxLayout * dirWidgetLyt = new QHBoxLayout(dirWidget);
    dirWidgetLyt->setSpacing(8);
    dirWidgetLyt->setContentsMargins(0, 0, 0, 0);
    dirWidget->setLayout(dirWidgetLyt);
    QFrame * dirFrame = new QFrame(dirWidget);
    dirFrame->setFrameShape(QFrame::Shape::Box);
    dirFrame->setFixedHeight(50);
    QHBoxLayout * dirFrameLayout = new QHBoxLayout(dirFrame);
    dirFrameLayout->setSpacing(16);
    dirFrameLayout->setContentsMargins(16, 0, 16, 0);
    QLabel * iconLabel = new QLabel(dirFrame);
    QLabel * pathLabel = new QLabel(dirFrame);

    dirFrameLayout->addWidget(iconLabel);
    iconLabel->setPixmap(QIcon::fromTheme("inode-directory").pixmap(QSize(24, 24)));
    pathLabel->setText(path);
    dirFrameLayout->addWidget(pathLabel);
    dirFrameLayout->addStretch();
    QPushButton * delBtn = new QPushButton(dirFrame);
    delBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    delBtn->setProperty("useButtonPalette", true);
    delBtn->setFixedSize(30, 30);
    delBtn->setToolTip(tr("delete"));
    delBtn->setFlat(true);


    delBtn->hide();
    dirFrameLayout->addWidget(delBtn);
    dirWidgetLyt->addWidget(dirFrame);
//    dirWidgetLyt->addWidget(delBtn);

    QFrame *line = new QFrame(m_blockDirsFrame);
    line->setObjectName(path);
    line->setFixedHeight(1);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
//    m_setFrameLyt->addWidget(line);

    m_blockDirsLyt->addWidget(line);
    m_blockDirsLyt->addWidget(dirWidget);
    connect(delBtn, &QPushButton::clicked, this, [ = ]() {
        setBlockDir(path, false);
        getBlockDirs();
    });
    connect(dirWidget, &HoverWidget::enterWidget, this, [ = ]() {
        delBtn->show();
    });
    connect(dirWidget, &HoverWidget::leaveWidget, this, [ = ]() {
        delBtn->hide();
    });
}

void Search::removeBlockDirFromList(const QString &path)
{
    HoverWidget * delDirWidget = m_blockDirsFrame->findChild<HoverWidget *>(path);
    if (delDirWidget) {
        qDebug() << "Delete blocked folder succeed! path = " << path;
        m_blockDirsLyt->removeWidget(delDirWidget);
        delDirWidget->deleteLater();
    }
    QFrame *line = m_blockDirsFrame->findChild<QFrame*>(path);
    if (line) {
        m_blockDirsLyt->removeWidget(line);
        line->deleteLater();
        qDebug() << "Delete line of blocked folder:" << path;
    }
}

void Search::setupConnection()
{
    connect(m_addBlockDirWidget, &QPushButton::clicked, this, &Search::onBtnAddBlockFolderClicked);
}

void Search::onBtnAddBlockFolderClicked()
{
    auto blockDirDialog = new QFileDialog;
    blockDirDialog->setFileMode(QFileDialog::DirectoryOnly); //只允许查看文件夹
//    fileDialog->setViewMode(QFileDialog::Detail);
    blockDirDialog->setDirectory(m_lastBlockDialogDir);
    blockDirDialog->setNameFilter(tr("Directories"));
    blockDirDialog->setWindowTitle(tr("select blocked folder"));
    blockDirDialog->setLabelText(QFileDialog::Accept, tr("Select"));
    blockDirDialog->setLabelText(QFileDialog::LookIn, tr("Position: "));
    blockDirDialog->setLabelText(QFileDialog::FileName, tr("FileName: "));
    blockDirDialog->setLabelText(QFileDialog::FileType, tr("FileType: "));
    blockDirDialog->setLabelText(QFileDialog::Reject, tr("Cancel"));
    connect(blockDirDialog, &QFileDialog::finished, [ = ] (int result) {
        if (result == QDialog::Accepted) {
            QString selectedDir = blockDirDialog->selectedFiles().first();
            m_lastBlockDialogDir = selectedDir.left(selectedDir.lastIndexOf("/") + 1);
            qWarning() << "======history of block dialog changed into " << m_lastBlockDialogDir;
            qDebug() << "Selected a folder in onBtnAddClicked(): " << selectedDir;
            int returnCode = setBlockDir(selectedDir, true);
            switch (returnCode) {
            case ReturnCode::Successful :
                qDebug() << "Add blocked folder succeed! path = " << selectedDir;
                getBlockDirs();
                break;
            case ReturnCode::Duplicated :
                qWarning() << "Add blocked folder failed, its parent dir is exist! path = " << selectedDir;
                QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add blocked folder failed, its parent dir has been added!"));
                break;
            case ReturnCode::NotExists :
                qWarning() << "Add blocked folder failed, it's not exist! path = " << selectedDir;
                QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add blocked folder failed, choosen path is not exist!"));
                break;
            case ReturnCode::HasBeenBlocked :
                qWarning() << "Add blocked folder failed, it has been already blocked! path = " << selectedDir;
                QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add blocked folder failed, it has already been blocked!"));
                break;
            case ReturnCode::Hidden :
                qWarning() << "Add blocked folder failed, it has been hidden! path = " << selectedDir;
                QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, hidden　path is not supported!"));
                break;
            default:
                break;
            }
        }
    });
    blockDirDialog->exec();
    if (blockDirDialog) {
        delete blockDirDialog;
        blockDirDialog = nullptr;
    }
}

void Search::onAddSearchDirBtnClicked()
{
    //添加搜索目录对话框
    auto searchDirDialog = new QFileDialog;
    searchDirDialog->setFileMode(QFileDialog::DirectoryOnly); //只允许查看文件夹
    searchDirDialog->setDirectory(m_lastSearchDialogDir);
    searchDirDialog->setNameFilter(tr("Directories"));
    searchDirDialog->setWindowTitle(tr("select search folder"));
    searchDirDialog->setLabelText(QFileDialog::Accept, tr("Select"));
    searchDirDialog->setLabelText(QFileDialog::LookIn, tr("Position: "));
    searchDirDialog->setLabelText(QFileDialog::FileName, tr("FileName: "));
    searchDirDialog->setLabelText(QFileDialog::FileType, tr("FileType: "));
    searchDirDialog->setLabelText(QFileDialog::Reject, tr("Cancel"));
    connect(searchDirDialog, &QFileDialog::finished, this, [ = ] (int result) {
        if (result == QDialog::Accepted) {
            QString selectedDir = searchDirDialog->selectedFiles().first();
            m_lastSearchDialogDir = selectedDir.left(selectedDir.lastIndexOf("/") + 1);
            qDebug() << "======history of search dialog changed into " << m_lastSearchDialogDir;
            qDebug() << "Selected a folder in onAddSearchDirBtnClicked(): " << selectedDir;
            int returnCode = setSearchDir(selectedDir, true);
            switch (returnCode) {
                case ReturnCode::Successful:
                    qDebug() << "Add search folder succeed! path = " << selectedDir;
                    break;
                case ReturnCode::Duplicated:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, choosen path or its parent dir has been added!"));
                    break;
                case ReturnCode::UnderBlackList:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, choosen path is not supported currently!"));
                    break;
                case ReturnCode::RepeatMount1:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, choosen path is in repeat mounted devices and another path which is in the same device has been added!"));
                    break;
                case ReturnCode::RepeatMount2:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, another path which is in the same device has been added!"));
                    break;
                case ReturnCode::NotExists:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, choosen path is not exists!"));
                    break;
                case ReturnCode::Hidden :
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, hidden　path is not supported!"));
                    break;
                case ReturnCode::PermissionDenied:
                    QMessageBox::warning(m_pluginWidget, tr("Warning"), tr("Add search folder failed, permission denied!"));
                    break;
                default:
                    break;
            }
        }
    });
    //open()会导致cpu占用拉满
//    searchDirDialog->open();
    searchDirDialog->exec();

    if (searchDirDialog) {
        delete searchDirDialog;
        searchDirDialog = nullptr;
    }
}

void Search::initAiSettings() {
    if (!s_session || s_failed) {
        if (data_management_create_session(&s_session, DataManagementType::SYS_SEARCH, getuid()) != DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
            s_failed = true;
            qWarning() << "===Create data management session failed!===" << s_session;
            return;
        }
    }
    FeatureStatus status;
    DataManagementResult result = data_management_get_feature_status(s_session, &status);
    if (result == DataManagementResult::DATA_MANAGEMENT_SUCCESS) {
        if (status == FeatureStatus::AVAILABLE) {
            isModelReady = true;
        } else {
            qWarning() << "===Model is not available";
        }
    } else {
        qWarning() << "===Get feature status failed, result is" << result;
    }
}

void Search::onGetAiModelBtnClicked() {
    bool res(false);
    auto appLaunchInterface = new QDBusInterface(QStringLiteral("com.lingmo.ProcessManager"),
                                                 QStringLiteral("/com/lingmo/ProcessManager/AppLauncher"),
                                                 QStringLiteral("com.lingmo.ProcessManager.AppLauncher"),
                                                 QDBusConnection::sessionBus());
    if(!appLaunchInterface->isValid()) {
        qWarning() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        res = false;
    } else {
        appLaunchInterface->setTimeout(10000);
        QDBusReply<void> reply = appLaunchInterface->call("LaunchApp", "/usr/share/applications/lingmo-ai-model-manager.desktop");
        if(reply.isValid()) {
            res = true;
        } else {
            qWarning() << "ProcessManager dbus called failed!" << reply.error();
            res = false;
        }
    }
    delete appLaunchInterface;
    appLaunchInterface = nullptr;

    if (!res) {
        QProcess::startDetached("lingmo-ai-model-manager", {});
    }
}
