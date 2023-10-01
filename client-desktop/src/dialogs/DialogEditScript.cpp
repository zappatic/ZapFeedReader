/*
    ZapFeedReader - RSS/Atom feed reader
    Copyright (C) 2023-present  Kasper Nauwelaerts (zapfr at zappatic dot net)

    ZapFeedReader is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ZapFeedReader is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ZapFeedReader.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <Poco/JSON/Parser.h>
#include <QClipboard>
#include <QMessageBox>
#include <QMimeData>

#include "SyntaxHighlighterLua.h"
#include "ZapFR/lua/ScriptLua.h"
#include "delegates/ItemDelegateEditScriptDialogSource.h"
#include "dialogs/DialogEditScript.h"
#include "ui_DialogEditScript.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::DialogEditScript::DialogEditScript(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEditScript)
{
    ui->setupUi(this);

    mSyntaxHighlighterLua = std::make_unique<SyntaxHighlighterLua>(ui->textEditScript->document());

    mFeedsModel = std::make_unique<QStandardItemModel>(this);
    ui->treeViewRunOnFeedIDs->setModel(mFeedsModel.get());
    ui->treeViewRunOnFeedIDs->setItemDelegate(new ItemDelegateEditScriptDialogSource(ui->treeViewRunOnFeedIDs));

    mTestEnclosuresModel = std::make_unique<QStandardItemModel>(this);
    ui->tableViewTestEnclosures->setModel(mTestEnclosuresModel.get());
    mDialogEditEnclosure = std::make_unique<DialogTestScriptEditEnclosure>(this);

    connect(ui->tabWidget, &QTabWidget::currentChanged,
            [this](int32_t index)
            {
                if (index == DialogEditScriptPane::Test)
                {
                    auto mimeData = QGuiApplication::clipboard()->mimeData();
                    ui->pushButtonPasteTestPost->setVisible(mimeData != nullptr && mimeData->hasFormat(MimeType::TestPost));
                }
            });
    connect(ui->pushButtonPasteTestPost, &QPushButton::clicked, this, &DialogEditScript::pasteTestPost);

    connect(ui->treeViewRunOnFeedIDs, &TreeViewEditScriptDialogSources::feedClicked,
            [&](const QModelIndex& index)
            {
                auto item = mFeedsModel->itemFromIndex(index);
                if (item != nullptr)
                {
                    auto current = item->data(Qt::CheckStateRole);
                    item->setData(current == Qt::Checked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
                }
                markDirty();
            });
    connect(ui->checkBoxRunOnAllFeeds, &QCheckBox::clicked, [&](bool checked) { ui->treeViewRunOnFeedIDs->setEnabled(!checked); });

    connect(ui->lineEditTitle, &QLineEdit::textChanged, this, &DialogEditScript::markDirty);
    connect(ui->textEditScript, &QTextEdit::textChanged, this, &DialogEditScript::markDirty);
    connect(ui->checkBoxEnabled, &QCheckBox::stateChanged, this, &DialogEditScript::markDirty);
    connect(ui->checkBoxRunOnAllFeeds, &QCheckBox::stateChanged, this, &DialogEditScript::markDirty);
    connect(ui->checkBoxRunOnNewPost, &QCheckBox::stateChanged, this, &DialogEditScript::markDirty);
    connect(ui->checkBoxRunOnUpdatePost, &QCheckBox::stateChanged, this, &DialogEditScript::markDirty);

    connect(ui->pushButtonRunScript, &QPushButton::clicked, this, &DialogEditScript::runTestScript);
    connect(ui->pushButtonResetTestValues, &QPushButton::clicked, this, &DialogEditScript::resetTestValues);
    connect(ui->pushButtonClearLog, &QPushButton::clicked, this, &DialogEditScript::clearLog);

    ui->widgetFlagBlue->setFlagColor(ZapFR::Engine::FlagColor::Blue);
    ui->widgetFlagGreen->setFlagColor(ZapFR::Engine::FlagColor::Green);
    ui->widgetFlagYellow->setFlagColor(ZapFR::Engine::FlagColor::Yellow);
    ui->widgetFlagOrange->setFlagColor(ZapFR::Engine::FlagColor::Orange);
    ui->widgetFlagRed->setFlagColor(ZapFR::Engine::FlagColor::Red);
    ui->widgetFlagPurple->setFlagColor(ZapFR::Engine::FlagColor::Purple);

    static std::vector<PopupFlag*> flags{ui->widgetFlagBlue, ui->widgetFlagGreen, ui->widgetFlagYellow, ui->widgetFlagOrange, ui->widgetFlagRed, ui->widgetFlagPurple};
    for (const auto& flag : flags)
    {
        connect(flag, &PopupFlag::flagClicked, [&](PopupFlag* clickedFlag) { clickedFlag->toggleStyle(); });
    }

    connect(ui->pushButtonAddTestEnclosure, &QPushButton::clicked, this, &DialogEditScript::addEnclosure);
    connect(ui->pushButtonRemoveTestEnclosure, &QPushButton::clicked, this, &DialogEditScript::removeEnclosure);
    connect(ui->tableViewTestEnclosures, &QTableView::doubleClicked, this, &DialogEditScript::editEnclosure);

    connect(mDialogEditEnclosure.get(), &DialogEditScript::accepted,
            [&]()
            {
                switch (mDialogEditEnclosure->displayMode())
                {
                    case DialogTestScriptEditEnclosure::DisplayMode::Add:
                    {
                        mDummyPost->addEnclosure(mDialogEditEnclosure->url().toStdString(), mDialogEditEnclosure->mimeType().toStdString(), mDialogEditEnclosure->size());
                        break;
                    }
                    case DialogTestScriptEditEnclosure::DisplayMode::Edit:
                    {
                        mDummyPost->updateEnclosure(static_cast<uint64_t>(ui->tableViewTestEnclosures->currentIndex().row()), mDialogEditEnclosure->url().toStdString(),
                                                    mDialogEditEnclosure->mimeType().toStdString(), mDialogEditEnclosure->size());
                        break;
                    }
                }
                updateTestUI();
            });
}

ZapFR::Client::DialogEditScript::~DialogEditScript()
{
    delete ui;
}

ZapFR::Client::DialogEditScript::DisplayMode ZapFR::Client::DialogEditScript::displayMode() const noexcept
{
    return mDisplayMode;
}

void ZapFR::Client::DialogEditScript::reset(DisplayMode dm, uint64_t sourceID, uint64_t id, const QString& title, bool isEnabled,
                                            const std::unordered_set<ZapFR::Engine::Script::Event>& runOnEvents,
                                            const std::optional<std::unordered_set<uint64_t>>& runOnFeedIDs, const QString& script)
{
    mDisplayMode = dm;
    mCurrentSourceID = sourceID;
    QString buttonCaption;
    switch (dm)
    {
        case DisplayMode::Add:
        {
            setWindowTitle(tr("Add script"));
            buttonCaption = tr("Add script");
            break;
        }
        case DisplayMode::Edit:
        {
            setWindowTitle(tr("Edit script"));
            buttonCaption = tr("Save");
            break;
        }
    }

    for (const auto& button : ui->buttonBox->buttons())
    {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ButtonRole::AcceptRole)
        {
            button->setText(buttonCaption);
            break;
        }
    }

    mCurrentID = id;
    ui->lineEditTitle->setText(title);
    ui->checkBoxEnabled->setCheckState(isEnabled ? Qt::Checked : Qt::Unchecked);

    ui->checkBoxRunOnNewPost->setCheckState(Qt::Unchecked);
    ui->checkBoxRunOnUpdatePost->setCheckState(Qt::Unchecked);
    if (runOnEvents.contains(ZapFR::Engine::Script::Event::NewPost))
    {
        ui->checkBoxRunOnNewPost->setCheckState(Qt::Checked);
    }
    if (runOnEvents.contains(ZapFR::Engine::Script::Event::UpdatePost))
    {
        ui->checkBoxRunOnUpdatePost->setCheckState(Qt::Checked);
    }

    ui->checkBoxRunOnAllFeeds->setCheckState(Qt::Unchecked);
    ui->treeViewRunOnFeedIDs->setEnabled(true);
    if (!runOnFeedIDs.has_value())
    {
        ui->checkBoxRunOnAllFeeds->setCheckState(Qt::Checked);
        ui->treeViewRunOnFeedIDs->setEnabled(false);
    }

    // clone the feeds and folders from the sources tree view, so we don't have to requery
    mFeedsModel->clear();
    qobject_cast<MainWindow*>(parent())->cloneSourceTreeContents(mCurrentSourceID, mFeedsModel.get(), runOnFeedIDs);

    // auto expand all the folders
    std::function<void(QStandardItem*)> expandChildren;
    expandChildren = [&](QStandardItem* parent)
    {
        for (int32_t k = 0; k < parent->rowCount(); ++k)
        {
            auto child = parent->child(k, 0);
            ui->treeViewRunOnFeedIDs->setExpanded(mFeedsModel->indexFromItem(child), true);
            expandChildren(child);
        }
    };
    expandChildren(mFeedsModel->invisibleRootItem());

    ui->textEditScript->setText(script);
    ui->tabWidget->setCurrentIndex(0);
    ui->lineEditTitle->setFocus();
    clearLog();
    mIsDirty = false;

    initializeTestEnvironment();
}

uint64_t ZapFR::Client::DialogEditScript::scriptID() const noexcept
{
    return mCurrentID;
}

uint64_t ZapFR::Client::DialogEditScript::scriptSourceID() const noexcept
{
    return mCurrentSourceID;
}

QString ZapFR::Client::DialogEditScript::title() const noexcept
{
    return ui->lineEditTitle->text();
}

QString ZapFR::Client::DialogEditScript::script() const noexcept
{
    return ui->textEditScript->toPlainText();
}

bool ZapFR::Client::DialogEditScript::isScriptEnabled() const noexcept
{
    return (ui->checkBoxEnabled->checkState() == Qt::Checked);
}

std::unordered_set<ZapFR::Engine::Script::Event> ZapFR::Client::DialogEditScript::runOnEvents() const
{
    std::unordered_set<ZapFR::Engine::Script::Event> events;
    if (ui->checkBoxRunOnNewPost->checkState() == Qt::Checked)
    {
        events.insert(ZapFR::Engine::Script::Event::NewPost);
    }
    if (ui->checkBoxRunOnUpdatePost->checkState() == Qt::Checked)
    {
        events.insert(ZapFR::Engine::Script::Event::UpdatePost);
    }
    return events;
}

bool ZapFR::Client::DialogEditScript::runOnAllFeeds() const noexcept
{
    return (ui->checkBoxRunOnAllFeeds->checkState() == Qt::Checked);
}

std::unordered_set<uint64_t> ZapFR::Client::DialogEditScript::runOnFeedIDs() const
{
    std::unordered_set<uint64_t> feedIDs;

    std::function<void(QStandardItem*)> findCheckedFeedIDs;
    findCheckedFeedIDs = [&](QStandardItem* parent)
    {
        for (int32_t i = 0; i < parent->rowCount(); ++i)
        {
            auto child = parent->child(i, 0);
            auto index = mFeedsModel->indexFromItem(child);
            switch (index.data(TreeViewSources::Role::Type).toULongLong())
            {
                case TreeViewSources::EntryType::Feed:
                {
                    if (index.data(Qt::CheckStateRole) == Qt::Checked)
                    {
                        feedIDs.insert(index.data(TreeViewSources::Role::ID).toULongLong());
                    }
                    break;
                }
                case TreeViewSources::EntryType::Folder:
                {
                    findCheckedFeedIDs(child);
                    break;
                }
            }
        }
    };
    findCheckedFeedIDs(mFeedsModel->invisibleRootItem());

    return feedIDs;
}

void ZapFR::Client::DialogEditScript::markDirty()
{
    mIsDirty = true;
}

void ZapFR::Client::DialogEditScript::closeEvent(QCloseEvent* e)
{
    if (mIsDirty)
    {
        auto m = QMessageBox(this);
        m.setIcon(QMessageBox::Icon::Warning);
        auto cancelButton = m.addButton(QMessageBox::Cancel);
        m.addButton(QMessageBox::Ignore);
        auto saveButton = m.addButton(QMessageBox::Save);
        m.setWindowTitle(tr("Unsaved changes"));
        m.setText(tr("You have made changes that haven't been saved yet."));
        m.setDefaultButton(QMessageBox::Cancel);
        m.exec();
        auto cb = m.clickedButton();
        if (cb == cancelButton)
        {
            e->ignore();
            return;
        }
        else if (cb == saveButton)
        {
            e->ignore();
            accept();
            return;
        }
    }
    QDialog::closeEvent(e);
}

void ZapFR::Client::DialogEditScript::initializeTestEnvironment()
{
    mDummySource = std::make_unique<ZapFR::Engine::SourceDummy>(1);
    mDummySource->setTitle("Dummy source");

    mDummyFeed = std::make_unique<ZapFR::Engine::FeedDummy>(1, mDummySource.get());
    mDummyFeed->setTitle("Dummy feed");
    mDummyFeed->setLink("https://zapfeedreader.zappatic.net");

    mDummyPost = std::make_unique<ZapFR::Engine::PostDummy>(1);
    mDummyPost->setFeedID(mDummyFeed->id());
    mDummyPost->setFeedTitle(mDummyFeed->title());
    mDummyPost->setFeedLink(mDummyFeed->link());
    mDummyPost->setTitle("This is the title of a dummy post");
    mDummyPost->setLink("https://zapfeedreader.zappatic.net");
    mDummyPost->setContent("The purpose of this post is <b>for testing</b> your scripts!");
    mDummyPost->setAuthor("Test Author");
    mDummyPost->setCommentsURL("https://zapfeedreader.zappatic.net#comments");
    mDummyPost->setLogCallback([&](const std::string& message) { QMetaObject::invokeMethod(this, [=, this]() { appendToLog(QString::fromUtf8(message)); }); });

    mDummySource->setAssociatedDummyFeed(mDummyFeed.get());
    mDummySource->setAssociatedDummyPost(mDummyPost.get());
    mDummyFeed->setAssociatedDummyPost(mDummyPost.get());

    updateTestUI();
}

void ZapFR::Client::DialogEditScript::updateTestUI()
{
    if (mDummyPost != nullptr)
    {
        ui->lineEditTestPostTitle->setText(QString::fromUtf8(mDummyPost->title()));
        ui->lineEditTestPostLink->setText(QString::fromUtf8(mDummyPost->link()));
        ui->plainTextEditTestPostContent->setPlainText(QString::fromUtf8(mDummyPost->content()));
        ui->lineEditTestPostAuthor->setText(QString::fromUtf8(mDummyPost->author()));
        ui->lineEditTestPostCommentsURL->setText(QString::fromUtf8(mDummyPost->commentsURL()));
        ui->lineEditTestPostThumbnail->setText(QString::fromUtf8(mDummyPost->thumbnail()));
        ui->checkBoxIsRead->setChecked(mDummyPost->isRead());

        const auto& flagColors = mDummyPost->flagColors();
        ui->widgetFlagBlue->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Blue) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);
        ui->widgetFlagGreen->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Green) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);
        ui->widgetFlagYellow->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Yellow) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);
        ui->widgetFlagOrange->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Orange) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);
        ui->widgetFlagRed->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Red) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);
        ui->widgetFlagPurple->setFlagStyle(flagColors.contains(ZapFR::Engine::FlagColor::Purple) ? Utilities::FlagStyle::Filled : Utilities::FlagStyle::Unfilled);

        mTestEnclosuresModel->clear();
        for (const auto& enclosure : mDummyPost->enclosures())
        {
            auto url = QString::fromUtf8(enclosure.url);
            auto urlItem = new QStandardItem(url);
            urlItem->setData(url, Qt::ToolTipRole);

            auto size = QString("%1 bytes").arg(enclosure.size);
            auto sizeItem = new QStandardItem(size);
            sizeItem->setData(size, Qt::ToolTipRole);

            auto mimeType = QString::fromUtf8(enclosure.mimeType);
            auto mimeTypeItem = new QStandardItem(mimeType);
            mimeTypeItem->setData(mimeType, Qt::ToolTipRole);

            QList<QStandardItem*> rowData;
            rowData << urlItem << mimeTypeItem << sizeItem;
            mTestEnclosuresModel->appendRow(rowData);
        }
        mTestEnclosuresModel->setHorizontalHeaderItem(EnclosureColumn::URL, new QStandardItem(tr("URL")));
        mTestEnclosuresModel->setHorizontalHeaderItem(EnclosureColumn::MimeType, new QStandardItem(tr("Mimetype")));
        mTestEnclosuresModel->setHorizontalHeaderItem(EnclosureColumn::Size, new QStandardItem(tr("Size")));

        auto hh = ui->tableViewTestEnclosures->horizontalHeader();
        hh->setSectionResizeMode(EnclosureColumn::Size, QHeaderView::Interactive);
        hh->setSectionResizeMode(EnclosureColumn::MimeType, QHeaderView::Interactive);
        hh->setSectionResizeMode(EnclosureColumn::URL, QHeaderView::Stretch);
        hh->resizeSection(EnclosureColumn::MimeType, 150);
        hh->resizeSection(EnclosureColumn::Size, 150);
    }
}

void ZapFR::Client::DialogEditScript::runTestScript()
{
    mDummyPost->setTitle(ui->lineEditTestPostTitle->text().toStdString());
    mDummyPost->setLink(ui->lineEditTestPostLink->text().toStdString());
    mDummyPost->setContent(ui->plainTextEditTestPostContent->toPlainText().toStdString());
    mDummyPost->setAuthor(ui->lineEditTestPostAuthor->text().toStdString());
    mDummyPost->setCommentsURL(ui->lineEditTestPostCommentsURL->text().toStdString());
    mDummyPost->setThumbnail(ui->lineEditTestPostThumbnail->text().toStdString());
    mDummyPost->setIsRead(ui->checkBoxIsRead->isChecked());

    std::unordered_set<ZapFR::Engine::FlagColor> flagColors;
    if (ui->widgetFlagBlue->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Blue);
    }
    if (ui->widgetFlagGreen->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Green);
    }
    if (ui->widgetFlagYellow->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Yellow);
    }
    if (ui->widgetFlagOrange->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Orange);
    }
    if (ui->widgetFlagRed->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Red);
    }
    if (ui->widgetFlagPurple->flagStyle() == Utilities::FlagStyle::Filled)
    {
        flagColors.insert(ZapFR::Engine::FlagColor::Purple);
    }
    mDummyPost->setFlagColors(flagColors);

    auto script = ui->textEditScript->toPlainText().toStdString();
    try
    {
        ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, mDummySource.get(), mDummyFeed.get(), mDummyPost.get(),
                                                               [&](const std::string& message)
                                                               { QMetaObject::invokeMethod(this, [=, this]() { appendToLog(QString::fromUtf8(message)); }); });
    }
    catch (const std::exception& e)
    {
        static QRegularExpression errorRegex(R"(^\[.*?\]:([0-9]+):(.*?)$)");
        auto m = errorRegex.match(e.what());
        if (m.hasMatch())
        {
            appendToLog(tr("Error on line %1: %2").arg(m.captured(1)).arg(m.captured(2)));
        }
        else
        {
            appendToLog(QString::fromUtf8(e.what()));
        }
    }
    updateTestUI();
}

void ZapFR::Client::DialogEditScript::resetTestValues()
{
    initializeTestEnvironment();
}

void ZapFR::Client::DialogEditScript::clearLog()
{
    ui->plainTextEditLog->setPlainText("");
}

void ZapFR::Client::DialogEditScript::appendToLog(const QString& message)
{
    ui->plainTextEditLog->appendPlainText(message);
}

void ZapFR::Client::DialogEditScript::pasteTestPost()
{
    auto mimeData = QGuiApplication::clipboard()->mimeData();
    if (mimeData != nullptr && mimeData->hasFormat(MimeType::TestPost))
    {
        auto jsonData = mimeData->data(MimeType::TestPost);
        auto json = std::string(jsonData.constData(), static_cast<size_t>(jsonData.length()));
        Poco::JSON::Parser parser;
        try
        {
            auto root = parser.parse(json);
            auto postJSON = root.extract<Poco::JSON::Object::Ptr>();
            mDummyPost = ZapFR::Engine::PostDummy::createFromJSON(postJSON);

            mDummySource->setAssociatedDummyPost(mDummyPost.get());
            mDummyFeed->setAssociatedDummyPost(mDummyPost.get());
            updateTestUI();
        }
        catch (...)
        {
        }
    }
}

void ZapFR::Client::DialogEditScript::addEnclosure()
{
    mDialogEditEnclosure->reset(DialogTestScriptEditEnclosure::DisplayMode::Add, "", "", 0);
    mDialogEditEnclosure->open();
}

void ZapFR::Client::DialogEditScript::editEnclosure()
{
    auto index = ui->tableViewTestEnclosures->currentIndex();
    if (index.isValid())
    {
        const auto& enclosure = mDummyPost->enclosures().at(static_cast<size_t>(index.row()));
        mDialogEditEnclosure->reset(DialogTestScriptEditEnclosure::DisplayMode::Edit, QString::fromUtf8(enclosure.url), QString::fromUtf8(enclosure.mimeType), enclosure.size);
        mDialogEditEnclosure->open();
    }
}

void ZapFR::Client::DialogEditScript::removeEnclosure()
{
    auto index = ui->tableViewTestEnclosures->currentIndex();
    if (index.isValid())
    {
        mDummyPost->removeEnclosure(static_cast<uint64_t>(index.row()));
        updateTestUI();
    }
}
