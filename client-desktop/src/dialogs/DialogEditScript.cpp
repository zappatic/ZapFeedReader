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

#include "dialogs/DialogEditScript.h"
#include "SyntaxHighlighterLua.h"
#include "ZapFR/lua/ScriptLua.h"
#include "delegates/ItemDelegateEditScriptDialogSource.h"
#include "models/StandardItemModelSources.h"
#include "ui_DialogEditScript.h"
#include "widgets/MainWindow.h"
#include "widgets/TreeViewSources.h"

ZapFR::Client::DialogEditScript::DialogEditScript(QWidget* parent) : QDialog(parent), ui(new Ui::DialogEditScript)
{
    ui->setupUi(this);
    mFeedsModel = std::make_unique<QStandardItemModel>(this);
    mSyntaxHighlighterLua = std::make_unique<SyntaxHighlighterLua>(ui->textEditScript->document());
    ui->treeViewRunOnFeedIDs->setModel(mFeedsModel.get());
    ui->treeViewRunOnFeedIDs->setItemDelegate(new ItemDelegateEditScriptDialogSource(ui->treeViewRunOnFeedIDs));

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

    mDummyPost = std::make_unique<ZapFR::Engine::PostDummy>(1);
    mDummyPost->setTitle("This is the title of a dummy post");
    mDummyPost->setLink("https://zapfeedreader.zappatic.net");
    mDummyPost->setContent("The purpose of this post is <b>for testing</b> your scripts!");
    mDummyPost->setAuthor("Test Author");
    mDummyPost->setCommentsURL("https://zapfeedreader.zappatic.net#comments");

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
    }
}

void ZapFR::Client::DialogEditScript::runTestScript()
{
    auto script = ui->textEditScript->toPlainText().toStdString();
    ZapFR::Engine::ScriptLua::getInstance()->runPostScript(script, mDummySource.get(), mDummyFeed.get(), mDummyPost.get());
    updateTestUI();
}

void ZapFR::Client::DialogEditScript::resetTestValues()
{
    initializeTestEnvironment();
}
