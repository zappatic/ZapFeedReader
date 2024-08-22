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

#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPen>
#include <QProcess>

#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Folder.h"
#include "ZapFR/base/ScriptFolder.h"
#include "delegates/ItemDelegatePost.h"
#include "widgets/MainWindow.h"
#include "widgets/PopupFlagChooser.h"
#include "widgets/TableViewPosts.h"
#include "widgets/TableViewScriptFolders.h"
#include "widgets/TreeViewSources.h"
#include "widgets/WebEnginePagePost.h"

namespace
{
    static auto gsAddToScriptFolderMenuProperty{"AddToScriptFolderMenuProperty"};
    static auto gsRemoveFromScriptFolderMenuProperty{"RemoveFromScriptFolderMenuProperty"};
} // namespace

ZapFR::Client::TableViewPosts::TableViewPosts(QWidget* parent) : TableViewPaletteCorrected(parent)
{
    setItemDelegate(new ItemDelegatePost(this));

    mItemModelPosts = std::make_unique<QStandardItemModel>(this);
    setModel(mItemModelPosts.get());

    mPopupFlagChooser = std::make_unique<PopupFlagChooser>(this);
    mPostWebEnginePage = std::make_unique<WebEnginePagePost>(this);

    mActionMarkAsRead = std::make_unique<QAction>(tr("Mark as read"), this);
    mActionMarkSelectionAsRead = std::make_unique<QAction>(tr("Mark as read"), this);
    mActionMarkSelectionAsUnread = std::make_unique<QAction>(tr("Mark as unread"), this);
    mActionOpenInExternalBrowser = std::make_unique<QAction>(tr("Open in external browser"), this);
    mActionCopyForTestScript = std::make_unique<QAction>(tr("Copy for test script"), this);

    viewport()->setAttribute(Qt::WA_Hover);
    setMouseTracking(true);
}

void ZapFR::Client::TableViewPosts::setMainWindow(MainWindow* mainWindow) noexcept
{
    mMainWindow = mainWindow;
    mPostWebEnginePage->setMainWindow(mMainWindow);

    auto ui = mMainWindow->getUI();
    ui->webViewPost->setPage(mPostWebEnginePage.get());
    ui->stackedWidgetPost->setCurrentIndex(PostPane::Post);
    connectStuff();
    createContextMenus();
}

void ZapFR::Client::TableViewPosts::reload()
{
    mIsReloading = true;
    auto ui = mMainWindow->getUI();
    ui->webViewPost->setBlankPostPage();
    clear();

    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Post* post, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(post->id()), Role::ID);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), Role::SourceID);
        item->setData(QVariant::fromValue<uint64_t>(post->feedID()), Role::FeedID);
        item->setData(QVariant::fromValue<bool>(post->isRead()), Role::IsRead);
        item->setData(QString::fromStdString(post->link()), Role::Link);
    };

    // lambda for the callback, retrieving the posts
    auto processPosts = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Post*>& posts, uint64_t pageNumber, uint64_t totalPostCount,
                            const std::vector<ZapFR::Engine::ThumbnailData>& thumbnailData)
    {
        QList<QList<QStandardItem*>> rows;
        for (const auto& post : posts)
        {
            auto unreadItem = new QStandardItem("");
            setItemData(unreadItem, post, sourceID);

            auto flagItem = new QStandardItem("");
            setItemData(flagItem, post, sourceID);
            QList<QVariant> flagColors;
            for (const auto& flag : post->flagColors())
            {
                flagColors << QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flag));
            }
            flagItem->setData(flagColors, Role::AppliedFlags);

            auto feedItem = new QStandardItem("");
            setItemData(feedItem, post, sourceID);
            feedItem->setData(QString::fromStdString(post->feedTitle()), Qt::ToolTipRole);

            auto titleItem = new QStandardItem(QString::fromStdString(post->title()));
            setItemData(titleItem, post, sourceID);

            auto datePublished = QString::fromStdString(post->datePublished());
            auto dateItem = new QStandardItem(Utilities::prettyDate(datePublished));
            dateItem->setData(datePublished, Role::ISODate);
            setItemData(dateItem, post, sourceID);

            QList<QStandardItem*> rowData;
            rowData << unreadItem << flagItem << feedItem << titleItem << dateItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, [=, this]() { populatePosts(rows, pageNumber, totalPostCount, thumbnailData); });
    };

    auto searchFilter = mMainWindow->searchQuery().toStdString();
    const auto& [categoryFilterID, categoryFilterName] = mMainWindow->categoryFilter();

    // preserve the current selection
    mPreviouslySelectedPostIDs.clear();
    auto sm = selectionModel();
    if (sm != nullptr)
    {
        auto selection = sm->selectedIndexes();
        for (const auto& selectedIndex : selection)
        {
            mPreviouslySelectedPostIDs.insert(selectedIndex.data(Role::ID).toULongLong());
        }
    }

    auto index = mMainWindow->getUI()->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(TableViewScriptFolders::Role::SourceID).toULongLong();
        auto scriptFolderID = index.data(TableViewScriptFolders::Role::ID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueGetScriptFolderPosts(sourceID, scriptFolderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                       mMainWindow->preferences()->showUnreadPostsAtTop, searchFilter, categoryFilterID, mFlagFilter,
                                                                       processPosts);
    }
    else
    {
        index = mMainWindow->treeViewSources()->currentIndex();
        if (index.isValid())
        {
            auto type = index.data(TreeViewSources::Role::Type).toULongLong();
            auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
            if (type == TreeViewSources::EntryType::Feed)
            {
                auto feedID = index.data(TreeViewSources::Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFeedPosts(sourceID, feedID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                       mMainWindow->preferences()->showUnreadPostsAtTop, searchFilter, categoryFilterID, mFlagFilter,
                                                                       processPosts);
            }
            else if (type == TreeViewSources::EntryType::Folder)
            {
                auto folderID = index.data(TreeViewSources::Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFolderPosts(sourceID, folderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                         mMainWindow->preferences()->showUnreadPostsAtTop, searchFilter, categoryFilterID, mFlagFilter,
                                                                         processPosts);
            }
            else if (type == TreeViewSources::EntryType::Source)
            {
                ZapFR::Engine::Agent::getInstance()->queueGetSourcePosts(sourceID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts,
                                                                         mMainWindow->preferences()->showUnreadPostsAtTop, searchFilter, categoryFilterID, mFlagFilter,
                                                                         processPosts);
            }
            else
            {
                populatePosts();
            }
        }
    }
    mMainWindow->updateToolbar();
}

void ZapFR::Client::TableViewPosts::populatePosts(const QList<QList<QStandardItem*>>& posts, uint64_t pageNumber, uint64_t totalPostCount,
                                                  const std::vector<ZapFR::Engine::ThumbnailData>& thumbnailData)
{
    mCurrentThumbnailData = thumbnailData;
    mMainWindow->setContentPane(ContentPane::Posts);

    clear();

    for (const auto& post : posts)
    {
        mItemModelPosts->appendRow(post);
    }
    handleSelectionChanged({});

    mCurrentPostCount = totalPostCount;
    mCurrentPostPage = pageNumber;
    mCurrentPostPageCount = 1;
    if (mCurrentPostCount > 0)
    {
        mCurrentPostPageCount = static_cast<uint64_t>(std::ceil(static_cast<float>(mCurrentPostCount) / static_cast<float>(msPostsPerPage)));
    }

    auto ui = mMainWindow->getUI();
    ui->pushButtonPostPageNumber->setText(QString("%1 %2 / %3").arg(tr("Page")).arg(mCurrentPostPage).arg(mCurrentPostPageCount));
    ui->labelTotalPostCount->setToolTip(tr("%n post(s)", "", static_cast<int32_t>(mCurrentPostCount)));
    QString totalPostCountCaption;
    if (mCurrentPostCount == 1)
    {
        totalPostCountCaption = tr("1 post");
    }
    else if (mCurrentPostCount > 1000)
    {
        totalPostCountCaption = tr("%1K posts").arg(QString::number(static_cast<float>(mCurrentPostCount) / 1000.0f, 'f', 1));
    }
    else
    {
        totalPostCountCaption = tr("%1 posts").arg(mCurrentPostCount);
    }
    ui->labelTotalPostCount->setText(totalPostCountCaption);

    // restore previous selection
    if (mPreviouslySelectedPostIDs.size() > 0)
    {
        auto newSelection = QItemSelection();
        for (auto i = 0; i < mItemModelPosts->rowCount(); ++i)
        {
            auto leftCell = mItemModelPosts->index(i, 0);
            if (mPreviouslySelectedPostIDs.contains(leftCell.data(Role::ID).toULongLong()))
            {
                auto rightCell = mItemModelPosts->index(i, mItemModelPosts->columnCount() - 1);
                newSelection.select(leftCell, rightCell);
            }
        }

        selectionModel()->select(newSelection, QItemSelectionModel::SelectCurrent);

        auto indexes = newSelection.indexes();
        if (mPreviouslySelectedPostIDs.size() == 1 && indexes.size() > 0)
        {
            scrollTo(indexes.at(0));
        }

        mPreviouslySelectedPostIDs.clear();
    }

    mIsReloading = false;
}

void ZapFR::Client::TableViewPosts::clear()
{
    int32_t columnWidthUnread = horizontalHeader()->sectionSize(Column::UnreadCol);
    int32_t columnWidthFlag = horizontalHeader()->sectionSize(Column::FlagCol);
    int32_t columnWidthFeed = horizontalHeader()->sectionSize(Column::FeedCol);
    int32_t columnWidthDate = horizontalHeader()->sectionSize(Column::DateCol);

    static bool firstRun{true};
    if (firstRun)
    {
        columnWidthUnread = 50;
        columnWidthFlag = 40;
        columnWidthFeed = 40;
        columnWidthDate = 180;
        firstRun = false;
    }

    mItemModelPosts->clear();

    mItemModelPosts->setHorizontalHeaderItem(Column::UnreadCol, new QStandardItem(tr("Unread")));
    mItemModelPosts->setHorizontalHeaderItem(Column::FlagCol, new QStandardItem(tr("Flag")));
    mItemModelPosts->setHorizontalHeaderItem(Column::FeedCol, new QStandardItem(tr("Feed")));
    mItemModelPosts->setHorizontalHeaderItem(Column::TitleCol, new QStandardItem(tr("Title")));
    mItemModelPosts->setHorizontalHeaderItem(Column::DateCol, new QStandardItem(tr("Date")));

    horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setSectionResizeMode(Column::TitleCol, QHeaderView::Stretch);
    horizontalHeader()->resizeSection(Column::UnreadCol, columnWidthUnread);
    horizontalHeader()->resizeSection(Column::FlagCol, columnWidthFlag);
    horizontalHeader()->resizeSection(Column::FeedCol, columnWidthFeed);
    horizontalHeader()->resizeSection(Column::DateCol, columnWidthDate);

    // in case we have just 1 feed selected, hide the feed column in the posts table
    // if we have a script folder selected, always show the feed column
    setColumnHidden(Column::FeedCol, false);
    auto treeViewSourcesIndex = mMainWindow->treeViewSources()->currentIndex();
    auto tableViewScriptFoldersIndex = mMainWindow->getUI()->tableViewScriptFolders->currentIndex();
    if (!tableViewScriptFoldersIndex.isValid() && treeViewSourcesIndex.isValid() &&
        treeViewSourcesIndex.data(TreeViewSources::Role::Type).toULongLong() == TreeViewSources::EntryType::Feed)
    {
        setColumnHidden(Column::FeedCol, true);
    }
}

void ZapFR::Client::TableViewPosts::handleSelectionChanged(const QModelIndexList& selected)
{
    if (mMainWindow == nullptr)
    {
        return; // this happens on startup, due to setModal in the c'tor
    }

    mCurrentPostID = 0;
    mCurrentPostSourceID = 0;
    mCurrentPostFeedID = 0;

    auto ui = mMainWindow->getUI();
    if (selected.count() == 1)
    {
        auto index = selected.at(0);
        if (index.isValid())
        {
            mCurrentPostID = index.data(Role::ID).toULongLong();
            mCurrentPostSourceID = index.data(Role::SourceID).toULongLong();
            mCurrentPostFeedID = index.data(Role::FeedID).toULongLong();

            auto isRead = index.data(Role::IsRead).toBool();
            if (!isRead)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostsRead(mCurrentPostSourceID, {{mCurrentPostFeedID, mCurrentPostID}},
                                                                        [&](uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs)
                                                                        { QMetaObject::invokeMethod(this, [=, this]() { postsMarkedRead(sourceID, feedAndPostIDs); }); });
            }
            reloadCurrentPost();
        }
    }
    else if (selected.count() == 0)
    {
        if (mCurrentThumbnailData.empty())
        {
            ui->webViewPost->setBlankPostPage();
            ui->widgetPostCaption->setCaption(tr("No post selected"), mMainWindow);
            ui->stackedWidgetPost->setCurrentIndex(PostPane::PostCaption);
        }
        else
        {
            auto sourceTreeIndex = ui->treeViewSources->currentIndex();
            if (sourceTreeIndex.isValid())
            {
                auto sourceID = sourceTreeIndex.data(TreeViewSources::Role::ParentSourceID).toULongLong();
                postReadyToBeShown(ui->webViewPost->getHTMLForThumbnailData(sourceID, mCurrentThumbnailData), {});
            }
            ui->stackedWidgetPost->setCurrentIndex(PostPane::Post);
        }
    }
    else
    {
        ui->webViewPost->setBlankPostPage();
        ui->widgetPostCaption->setCaption(tr("%1 posts selected").arg(selected.count()), mMainWindow);
        ui->stackedWidgetPost->setCurrentIndex(PostPane::PostCaption);
    }
}

void ZapFR::Client::TableViewPosts::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    QTableView::selectionChanged(selected, deselected);

    QModelIndexList list;
    for (const auto& index : selectedIndexes())
    {
        if (index.column() == 0)
        {
            list.append(index);
        }
    }
    handleSelectionChanged(list);
}

void ZapFR::Client::TableViewPosts::reloadCurrentPost()
{
    auto ui = mMainWindow->getUI();
    ui->tableViewPostEnclosures->clear();

    if (mCurrentPostSourceID > 0 && mCurrentPostFeedID > 0 && mCurrentPostID > 0)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetPost(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                          [&](std::unique_ptr<ZapFR::Engine::Post> post)
                                                          {
                                                              auto postHTML = mMainWindow->getUI()->webViewPost->getHTMLForPost(post.get());
                                                              const auto& enclosures = post->enclosures();

                                                              QMetaObject::invokeMethod(this, [=, this]() { postReadyToBeShown(postHTML, enclosures); });
                                                          });
    }
    else
    {
        ui->webViewPost->setBlankPostPage();
        ui->widgetPostCaption->setCaption(tr("No post selected"), mMainWindow);
        ui->stackedWidgetPost->setCurrentIndex(PostPane::PostCaption);
    }
}

void ZapFR::Client::TableViewPosts::postReadyToBeShown(const QString& html, const std::vector<ZapFR::Engine::Post::Enclosure>& enclosures)
{
    auto ui = mMainWindow->getUI();
    ui->webViewPost->setPostHTML(html);
    ui->tableViewPostEnclosures->loadEnclosures(enclosures);

    // hide/show the enclosures table at an appropriate size
    if (enclosures.empty())
    {
        ui->splitterPostAndEnclosures->setSizes({10000, 0});
    }
    else
    {
        int32_t heightEnclosuresTable = ui->tableViewPostEnclosures->horizontalHeader()->height() +
                                        (static_cast<int32_t>(enclosures.size()) * ui->tableViewPostEnclosures->verticalHeader()->defaultSectionSize());
        auto heightPostWebview = ui->splitterPostAndEnclosures->size().height() - heightEnclosuresTable;
        ui->splitterPostAndEnclosures->setSizes({heightPostWebview, heightEnclosuresTable});
    }
}

void ZapFR::Client::TableViewPosts::mouseMoveEvent(QMouseEvent* event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && index.column() == Column::FlagCol)
    {
        setCursor(Qt::PointingHandCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
    QTableView::mouseMoveEvent(event);
}

void ZapFR::Client::TableViewPosts::mouseReleaseEvent(QMouseEvent* event)
{
    auto index = indexAt(event->pos());
    if (index.isValid() && index.column() == Column::FlagCol)
    {
        auto clickLocation = mapToGlobal(event->pos());
        if ((QGuiApplication::keyboardModifiers() & Qt::ControlModifier) == Qt::ControlModifier)
        {
            auto sourceID = index.data(Role::SourceID).toULongLong();
            auto feedID = index.data(Role::FeedID).toULongLong();
            auto postID = index.data(Role::ID).toULongLong();

            ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(
                sourceID, {{feedID, postID}}, ZapFR::Engine::Flag::allFlagColors(),
                [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs,
                    const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColors)
                { QMetaObject::invokeMethod(this, [=, this]() { updatePostsFlags(false, affectedSourceID, affectedFeedAndPostIDs, affectedFlagColors); }); });

            return;
        }
        else
        {
            mPopupFlagChooser->setGeometry(clickLocation.x(), clickLocation.y(), 200, 75);
            mPopupFlagChooser->showWithSelectedColors(index.data(Role::AppliedFlags).toList());
        }
    }
    QTableView::mouseReleaseEvent(event);
}

void ZapFR::Client::TableViewPosts::keyPressEvent(QKeyEvent* event)
{
    auto k = event->key();
    if (k == Qt::Key_Return || k == Qt::Key_Enter || k == Qt::Key_Space)
    {
        openSelectedPostsInExternalBrowser([](const QString& url) { QDesktopServices::openUrl(url); });
    }
    QTableView::keyPressEvent(event);
}

void ZapFR::Client::TableViewPosts::processFlagToggle(ZapFR::Engine::FlagColor flagColor, Utilities::FlagStyle flagStyle)
{
    QModelIndex index;
    auto sm = selectionModel();
    if (sm != nullptr)
    {
        auto selectedIndexes = sm->selectedIndexes();
        for (const auto& selectedIndex : selectedIndexes)
        {
            if (selectedIndex.column() == Column::FlagCol)
            {
                index = selectedIndex;
                break;
            }
        }
    }

    if (!index.isValid())
    {
        return;
    }

    auto sourceID = index.data(Role::SourceID).toULongLong();
    auto feedID = index.data(Role::FeedID).toULongLong();
    auto postID = index.data(Role::ID).toULongLong();

    switch (flagStyle)
    {
        case Utilities::FlagStyle::Filled:
        {
            ZapFR::Engine::Agent::getInstance()->queueMarkPostsFlagged(
                sourceID, {{feedID, postID}}, {flagColor},
                [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs,
                    const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColors)
                { QMetaObject::invokeMethod(this, [=, this]() { updatePostsFlags(true, affectedSourceID, affectedFeedAndPostIDs, affectedFlagColors); }); });
            break;
        }
        case Utilities::FlagStyle::Unfilled:
        {
            ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(
                sourceID, {{feedID, postID}}, {flagColor},
                [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs,
                    const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColors)
                { QMetaObject::invokeMethod(this, [=, this]() { updatePostsFlags(false, affectedSourceID, affectedFeedAndPostIDs, affectedFlagColors); }); });
            break;
        }
    }
}

void ZapFR::Client::TableViewPosts::openSelectedPostsInExternalBrowser(const std::function<void(const QString&)>& openHandler)
{
    std::unordered_set<QString> urls;
    auto selection = selectionModel()->selectedIndexes();
    for (const auto& s : selection)
    {
        if (s.column() == Column::TitleCol)
        {
            auto link = s.data(Role::Link).toString();
            if (!link.isEmpty() && link.startsWith("http"))
            {
                urls.insert(link);
            }
        }
    }

    bool openURLs = urls.size() > 0;

    if (urls.size() > 5)
    {
        auto m = QMessageBox(this);
        m.setIcon(QMessageBox::Icon::Warning);
        m.addButton(QMessageBox::Cancel);
        auto openAnywayButton = m.addButton(QMessageBox::Yes);
        openAnywayButton->setText(tr("Open anyway"));
        m.setWindowTitle(tr("Multiple posts selected"));
        m.setText(tr("You have selected %1 posts. Are you sure you want to open that many browser tabs at once?").arg(QString::number(urls.size())));
        m.setDefaultButton(QMessageBox::Cancel);
        m.exec();
        auto cb = m.clickedButton();
        openURLs = (cb == openAnywayButton);
    }

    if (openURLs)
    {
        for (const auto& url : urls)
        {
            openHandler(url);
        }
    }
}

void ZapFR::Client::TableViewPosts::paintEvent(QPaintEvent* event)
{
    QTableView::paintEvent(event);

    if (model() != nullptr && model()->rowCount() == 0)
    {
        Theme theme{Theme::Light};
        if (mMainWindow != nullptr)
        {
            theme = mMainWindow->getCurrentColorTheme();
        }
        auto textPen = QPen(theme == Theme::Dark ? QColor(68, 68, 68) : QColor(170, 170, 170));
        auto vp = viewport();
        auto textRect = QRect(0, 50, vp->width(), 100);
        auto f = font();
        f.setPixelSize(32);
        f.setWeight(QFont::DemiBold);

        auto painter = QPainter(vp);
        painter.setPen(textPen);
        painter.setFont(f);

        auto caption = mIsReloading ? tr("Loading posts...") : tr("No posts found");
        painter.drawText(textRect, Qt::AlignTop | Qt::AlignHCenter, caption);
    }
}

void ZapFR::Client::TableViewPosts::clearPosts()
{
    populatePosts();
}

std::vector<std::tuple<uint64_t, uint64_t>> ZapFR::Client::TableViewPosts::selectedPostIDs() const
{
    std::vector<std::tuple<uint64_t, uint64_t>> feedAndPostIDs;
    auto selectionModel = this->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        for (const auto& index : selectedIndexes)
        {
            if (index.column() == Column::UnreadCol)
            {
                auto feedID = index.data(Role::FeedID).toULongLong();
                auto postID = index.data(Role::ID).toULongLong();
                feedAndPostIDs.emplace_back(std::make_tuple(feedID, postID));
            }
        }
    }
    return feedAndPostIDs;
}

void ZapFR::Client::TableViewPosts::postsMarkedRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs)
{
    std::unordered_set<uint64_t> uniqueFeedIDs{};
    std::unordered_set<uint64_t> uniquePostIDs{};
    for (const auto& [feedID, postID] : postIDs)
    {
        uniqueFeedIDs.insert(feedID);
        uniquePostIDs.insert(postID);
    }
    updatePostsReadStatus(true, sourceID, uniquePostIDs);

    for (const auto& feedID : uniqueFeedIDs)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(
            sourceID, feedID,
            [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
            {
                std::vector<uint64_t> feedIDs;
                feedIDs.emplace_back(affectedFeedID);
                QMetaObject::invokeMethod(this, [=, this]() { mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(affectedSourceID, feedIDs, false, unreadCount); });
            });
    }

    mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();

    // no statusbar update here, as it's called just for clicking/reading a post in the table, which would be distracting
}

void ZapFR::Client::TableViewPosts::postsMarkedUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs)
{
    std::unordered_set<uint64_t> uniqueFeedIDs{};
    std::unordered_set<uint64_t> uniquePostIDs{};
    for (const auto& [feedID, postID] : postIDs)
    {
        uniqueFeedIDs.insert(feedID);
        uniquePostIDs.insert(postID);
    }
    updatePostsReadStatus(false, sourceID, uniquePostIDs);

    for (const auto& feedID : uniqueFeedIDs)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(
            sourceID, feedID,
            [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
            {
                std::vector<uint64_t> feedIDs;
                feedIDs.emplace_back(affectedFeedID);
                QMetaObject::invokeMethod(this, [=, this]() { mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(affectedSourceID, feedIDs, false, unreadCount); });
            });
    }

    mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();

    mMainWindow->setStatusBarMessage(tr("Post(s) marked as unread"));
}

void ZapFR::Client::TableViewPosts::updatePostsReadStatus(bool markAsRead, uint64_t sourceID, const std::optional<std::unordered_set<uint64_t>>& postIDs)
{
    for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
    {
        auto index = mItemModelPosts->index(i, 0);
        if (index.data(Role::SourceID).toULongLong() != sourceID)
        {
            continue;
        }

        auto setUnreadStatus{false};
        if (!postIDs.has_value())
        {
            setUnreadStatus = true;
        }
        else
        {
            setUnreadStatus = postIDs.value().contains(index.data(Role::ID).toULongLong());
        }

        if (setUnreadStatus)
        {
            for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
            {
                auto item = mItemModelPosts->item(i, col);
                item->setData(QVariant::fromValue<bool>(markAsRead), Role::IsRead);
            }
        }
    }
}

void ZapFR::Client::TableViewPosts::updatePostsFlags(bool markFlagged, uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs,
                                                     const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors)
{
    mMainWindow->getUI()->frameFlagFilters->reload(true);

    auto [currentSourceID, currentFeedID] = mMainWindow->getUI()->treeViewSources->getCurrentlySelectedSourceAndFeedID();
    if (currentSourceID != sourceID)
    {
        return;
    }

    for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
    {
        auto index = mItemModelPosts->index(i, Column::FlagCol);

        auto found{false};
        for (const auto& [checkFeedID, checkPostID] : feedAndPostIDs)
        {
            if (checkFeedID == index.data(Role::FeedID).toULongLong() && checkPostID == index.data(Role::ID).toULongLong())
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            continue;
        }

        auto flags = index.data(Role::AppliedFlags).toList();
        if (markFlagged)
        {
            for (const auto& flagColor : flagColors)
            {
                auto flagVariant = QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flagColor));
                if (!flags.contains(flagVariant))
                {
                    flags << flagVariant;
                }
            }
            mItemModelPosts->itemFromIndex(index)->setData(flags, Role::AppliedFlags);
        }
        else
        {
            flags.removeIf([&](const QVariant& v) { return flagColors.contains(static_cast<ZapFR::Engine::FlagColor>(v.toInt())); });
            mItemModelPosts->itemFromIndex(index)->setData(flags, Role::AppliedFlags);
        }
    }

    mMainWindow->setStatusBarMessage(markFlagged ? tr("Post(s) marked as flagged") : tr("Post(s) marked as unflagged"));
}

void ZapFR::Client::TableViewPosts::markAsRead()
{
    // check if we currently have a scriptfolder selected first
    auto ui = mMainWindow->getUI();

    auto sfIndex = ui->tableViewScriptFolders->currentIndex();
    if (sfIndex.isValid())
    {
        auto sourceID = sfIndex.data(TableViewScriptFolders::Role::SourceID).toULongLong();
        auto maxPostID = ui->treeViewSources->highestPostID(sourceID);
        auto scriptFolderID = sfIndex.data(TableViewScriptFolders::Role::ID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkScriptFolderRead(sourceID, scriptFolderID, maxPostID,
                                                                       [&](uint64_t affectedSourceID, std::vector<uint64_t> affectedFeedIDs)
                                                                       {
                                                                           QMetaObject::invokeMethod(this,
                                                                                                     [=, this]()
                                                                                                     {
                                                                                                         mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(
                                                                                                             affectedSourceID, affectedFeedIDs, false, 0);
                                                                                                         mCurrentPostPage = 1;
                                                                                                         updatePostsReadStatus(true, affectedSourceID, {});
                                                                                                         mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();
                                                                                                         mMainWindow->setStatusBarMessage(tr("Script folder marked as read"));
                                                                                                     });
                                                                       });
        return;
    }

    auto index = mMainWindow->treeViewSources()->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(TreeViewSources::Role::ParentSourceID).toULongLong();
        auto maxPostID = ui->treeViewSources->highestPostID(sourceID);
        auto type = index.data(TreeViewSources::Role::Type).toULongLong();
        switch (type)
        {
            case TreeViewSources::EntryType::Feed:
            {
                auto feedID = index.data(TreeViewSources::Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFeedRead(sourceID, feedID, maxPostID,
                                                                       [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                                                                       {
                                                                           QMetaObject::invokeMethod(this,
                                                                                                     [=, this]()
                                                                                                     {
                                                                                                         mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(
                                                                                                             affectedSourceID, {affectedFeedID}, false, 0);
                                                                                                         mCurrentPostPage = 1;
                                                                                                         updatePostsReadStatus(true, affectedSourceID, {});
                                                                                                         mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();
                                                                                                         mMainWindow->setStatusBarMessage(tr("Feed marked as read"));
                                                                                                     });
                                                                       });
                break;
            }
            case TreeViewSources::EntryType::Folder:
            {
                auto folderID = index.data(TreeViewSources::Role::ID).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFolderRead(sourceID, folderID, maxPostID,
                                                                         [&](uint64_t affectedSourceID, std::vector<uint64_t> affectedFeedIDs)
                                                                         {
                                                                             QMetaObject::invokeMethod(this,
                                                                                                       [=, this]()
                                                                                                       {
                                                                                                           mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(
                                                                                                               affectedSourceID, affectedFeedIDs, false, 0);
                                                                                                           mCurrentPostPage = 1;
                                                                                                           updatePostsReadStatus(true, affectedSourceID, {});
                                                                                                           mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();
                                                                                                           mMainWindow->setStatusBarMessage(tr("Folder marked as read"));
                                                                                                       });
                                                                         });
                break;
            }
            case TreeViewSources::EntryType::Source:
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkSourceRead(sourceID, maxPostID,
                                                                         [&](uint64_t affectedSourceID)
                                                                         {
                                                                             QMetaObject::invokeMethod(this,
                                                                                                       [=, this]()
                                                                                                       {
                                                                                                           mMainWindow->treeViewSources()->updateFeedUnreadCountBadge(
                                                                                                               affectedSourceID, {}, true, 0);
                                                                                                           mCurrentPostPage = 1;
                                                                                                           updatePostsReadStatus(true, affectedSourceID, {});
                                                                                                           mMainWindow->getUI()->tableViewScriptFolders->refreshBadges();
                                                                                                           mMainWindow->setStatusBarMessage(tr("Source marked as read"));
                                                                                                       });
                                                                         });
                break;
            }
        }
    }
}

void ZapFR::Client::TableViewPosts::markPostSelectionAsRead()
{
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsRead(
            sourceID, feedAndPostIDs, [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs)
            { QMetaObject::invokeMethod(this, [=, this]() { postsMarkedRead(affectedSourceID, affectedFeedAndPostIDs); }); });
    }
}

void ZapFR::Client::TableViewPosts::markPostSelectionAsUnread()
{
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnread(
            sourceID, feedAndPostIDs, [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs)
            { QMetaObject::invokeMethod(this, [=, this]() { postsMarkedUnread(affectedSourceID, affectedFeedAndPostIDs); }); });
    }
}

void ZapFR::Client::TableViewPosts::markPostSelectionFlagged()
{
    auto action = qobject_cast<QAction*>(sender());
    auto flagColor = static_cast<ZapFR::Engine::FlagColor>(action->data().toULongLong());

    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsFlagged(
            sourceID, feedAndPostIDs, {flagColor},
            [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs,
                const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColors)
            { QMetaObject::invokeMethod(this, [=, this]() { updatePostsFlags(true, affectedSourceID, affectedFeedAndPostIDs, affectedFlagColors); }); });
    }
}

void ZapFR::Client::TableViewPosts::markPostSelectionUnflagged()
{
    auto action = qobject_cast<QAction*>(sender());
    auto flagColor = static_cast<ZapFR::Engine::FlagColor>(action->data().toULongLong());
    std::unordered_set<ZapFR::Engine::FlagColor> flagColors;
    if (flagColor == ZapFR::Engine::FlagColor::Gray)
    {
        flagColors = ZapFR::Engine::Flag::allFlagColors();
    }
    else
    {
        flagColors.insert(flagColor);
    }

    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(
            sourceID, feedAndPostIDs, flagColors,
            [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs,
                const std::unordered_set<ZapFR::Engine::FlagColor>& affectedFlagColors)
            { QMetaObject::invokeMethod(this, [=, this]() { updatePostsFlags(false, affectedSourceID, affectedFeedAndPostIDs, affectedFlagColors); }); });
    }
}

void ZapFR::Client::TableViewPosts::assignPostSelectionToScriptFolder()
{
    auto action = qobject_cast<QAction*>(sender());
    auto scriptFolderID = action->data().toULongLong();
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueAssignPostsToScriptFolder(
            sourceID, scriptFolderID, feedAndPostIDs, [&](uint64_t affectedSourceID, uint64_t affectedScriptFolderID)
            { QMetaObject::invokeMethod(this, [=, this]() { postsAssignedToScriptFolder(affectedSourceID, affectedScriptFolderID); }); });
    }
}

void ZapFR::Client::TableViewPosts::removePostSelectionFromScriptFolder()
{
    auto action = qobject_cast<QAction*>(sender());
    auto scriptFolderID = action->data().toULongLong();
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = mMainWindow->treeViewSources()->currentIndex().data(TreeViewSources::Role::ParentSourceID).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueRemovePostsFromScriptFolder(
            sourceID, scriptFolderID, feedAndPostIDs, [&](uint64_t affectedSourceID, uint64_t affectedScriptFolderID)
            { QMetaObject::invokeMethod(this, [=, this]() { postsRemovedFromScriptFolder(affectedSourceID, affectedScriptFolderID); }); });
    }
}

void ZapFR::Client::TableViewPosts::postsAssignedToScriptFolder(uint64_t /*sourceID*/, uint64_t /*scriptFolderID*/)
{
    mMainWindow->getUI()->tableViewScriptFolders->reload(true);
    mMainWindow->setStatusBarMessage(tr("Post(s) assigned to script folder"));
}

void ZapFR::Client::TableViewPosts::postsRemovedFromScriptFolder(uint64_t sourceID, uint64_t scriptFolderID)
{
    mMainWindow->getUI()->tableViewScriptFolders->reload(true);
    auto index = mMainWindow->getUI()->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto selectedSourceID = index.data(TableViewScriptFolders::Role::SourceID).toULongLong();
        if (sourceID != selectedSourceID)
        {
            return;
        }
        auto selectedScriptFolderID = index.data(TableViewScriptFolders::Role::ID).toULongLong();
        if (scriptFolderID != selectedScriptFolderID)
        {
            return;
        }
        reload();
    }
    mMainWindow->setStatusBarMessage(tr("Post(s) unassigned from script folder"));
}

void ZapFR::Client::TableViewPosts::updateActivePostFilter()
{
    auto ui = mMainWindow->getUI();

    auto searchQuery = mMainWindow->searchQuery();
    const auto& [categoryFilterID, categoryFilterName] = mMainWindow->categoryFilter();
    auto selectedScriptFolder = ui->tableViewScriptFolders->currentIndex();

    auto isScriptFolderFilterActive{selectedScriptFolder.isValid()};
    auto isFlagFilterActive{mFlagFilter != ZapFR::Engine::FlagColor::Gray};
    auto isOnlyUnreadFilterActive{mShowOnlyUnreadPosts};
    auto isTextSearchFilterActive{!searchQuery.isEmpty()};
    auto isCategoryFilterActive{categoryFilterID != 0};
    auto isOtherFilterActive{isScriptFolderFilterActive || isOnlyUnreadFilterActive || isTextSearchFilterActive || isCategoryFilterActive};

    ui->labelActiveFilter->setVisible(isFlagFilterActive || isOtherFilterActive);
    ui->labelActiveFilterFlag->setVisible(isFlagFilterActive);
    ui->labelActiveFilterOther->setVisible(isOtherFilterActive);

    if (isFlagFilterActive)
    {
        ui->labelActiveFilterFlag->setPixmap(Utilities::flag(mFlagFilter, Utilities::FlagStyle::Filled));
    }
    QStringList otherFilters;
    if (isScriptFolderFilterActive)
    {
        otherFilters << tr("Script folder '%1'").arg(selectedScriptFolder.data(Qt::DisplayRole).toString());
    }
    if (isOnlyUnreadFilterActive)
    {
        otherFilters << tr("Only unread");
    }
    if (isTextSearchFilterActive)
    {
        otherFilters << tr("Search '%1'").arg(searchQuery);
    }
    if (isCategoryFilterActive)
    {
        otherFilters << tr("Category '%1'").arg(categoryFilterName);
    }

    if (isOtherFilterActive)
    {
        ui->labelActiveFilterOther->setText(otherFilters.join(", "));
    }
}

void ZapFR::Client::TableViewPosts::connectStuff()
{
    auto ui = mMainWindow->getUI();

    connect(this, &QTableView::doubleClicked, [&]() { openSelectedPostsInExternalBrowser([](const QString& url) { QDesktopServices::openUrl(url); }); });
    connect(mPopupFlagChooser.get(), &PopupFlagChooser::flagToggled, this, &TableViewPosts::processFlagToggle);

    connect(mActionMarkAsRead.get(), &QAction::triggered, this, &TableViewPosts::markAsRead);
    connect(mActionMarkSelectionAsUnread.get(), &QAction::triggered, this, &TableViewPosts::markPostSelectionAsUnread);
    connect(mActionMarkSelectionAsRead.get(), &QAction::triggered, this, &TableViewPosts::markPostSelectionAsRead);
    connect(mActionOpenInExternalBrowser.get(), &QAction::triggered,
            [&]() { openSelectedPostsInExternalBrowser([](const QString& url) { QDesktopServices::openUrl(url); }); });

    connect(mActionCopyForTestScript.get(), &QAction::triggered,
            [this]()
            {
                auto index = currentIndex();
                if (index.isValid())
                {
                    auto postID = index.data(Role::ID).toULongLong();
                    auto feedID = index.data(Role::FeedID).toULongLong();
                    auto sourceID = index.data(Role::SourceID).toULongLong();
                    ZapFR::Engine::Agent::getInstance()->queueGetPost(sourceID, feedID, postID,
                                                                      [&](std::unique_ptr<ZapFR::Engine::Post> post)
                                                                      {
                                                                          auto jsonObj = post->toJSON();
                                                                          std::stringstream jsonStream;
                                                                          Poco::JSON::Stringifier::stringify(jsonObj, jsonStream);
                                                                          auto json = jsonStream.str();

                                                                          QMetaObject::invokeMethod(this,
                                                                                                    [=, this]()
                                                                                                    {
                                                                                                        auto jsonData =
                                                                                                            QByteArray(json.c_str(), static_cast<ssize_t>(json.length()));
                                                                                                        auto mimeData = new QMimeData();
                                                                                                        mimeData->setData(MimeType::TestPost, jsonData);
                                                                                                        QGuiApplication::clipboard()->setMimeData(mimeData);
                                                                                                        mMainWindow->setStatusBarMessage(tr("Post copied"));
                                                                                                    });
                                                                      });
                }
            });

    connect(this, &TableViewPosts::customContextMenuRequested,
            [&](const QPoint& p)
            {
                // gather the script folder title and IDs
                std::vector<std::tuple<QVariant, QString>> scriptFolderData;
                for (const auto& [scriptFolderID, scriptFolderTitle] : mMainWindow->getUI()->tableViewScriptFolders->getIDToTitleMapping())
                {
                    scriptFolderData.emplace_back(QVariant::fromValue<uint64_t>(scriptFolderID), scriptFolderTitle);
                }

                // dynamically add the script folder entries to the 'add to script folder' and 'remove from script folder' submenus
                for (const auto& action : mPostContextMenu->actions())
                {
                    auto subMenu = action->menu();
                    if (subMenu != nullptr)
                    {
                        if (subMenu->property(gsAddToScriptFolderMenuProperty).isValid())
                        {
                            subMenu->clear();
                            if (scriptFolderData.empty())
                            {
                                action->setVisible(false);
                            }
                            else
                            {
                                action->setVisible(true);
                                for (const auto& [scriptFolderID, scriptFolderTitle] : scriptFolderData)
                                {
                                    auto childAction = subMenu->addAction(scriptFolderTitle);
                                    childAction->setData(scriptFolderID);
                                    connect(childAction, &QAction::triggered, this, &TableViewPosts::assignPostSelectionToScriptFolder);
                                }
                            }
                        }
                        else if (subMenu->property(gsRemoveFromScriptFolderMenuProperty).isValid())
                        {
                            subMenu->clear();
                            if (scriptFolderData.empty())
                            {
                                action->setVisible(false);
                            }
                            else
                            {
                                action->setVisible(true);
                                for (const auto& [scriptFolderID, scriptFolderTitle] : scriptFolderData)
                                {
                                    auto childAction = subMenu->addAction(scriptFolderTitle);
                                    childAction->setData(scriptFolderID);
                                    connect(childAction, &QAction::triggered, this, &TableViewPosts::removePostSelectionFromScriptFolder);
                                }
                            }
                        }
                    }
                }

                // add the detected browsers
                if (mMainWindow->preferences()->detectBrowsers)
                {
                    auto browsers = WebEngineViewPost::detectBrowsers();
                    static bool browsersAdded{false};
                    if (!browsersAdded)
                    {
                        for (const auto& browser : browsers)
                        {
                            auto action = new QAction(tr("&Open in %1").arg(browser.title), mPostContextMenu.get());
                            connect(action, &QAction::triggered,
                                    [=, this]()
                                    {
                                        openSelectedPostsInExternalBrowser(
                                            [&](const QString& url)
                                            {
                                                QStringList args;
                                                for (auto arg : browser.args)
                                                {
                                                    args << arg.replace("{url}", url);
                                                }
                                                qint64 pid{0};
                                                QProcess::startDetached(browser.command, args, QString(), &pid);
                                            });
                                    });
                            mPostContextMenu->addAction(action);
                        }
                        browsersAdded = true;
                    }
                }

                mPostContextMenu->popup(mMainWindow->getUI()->tableViewPosts->viewport()->mapToGlobal(p));
            });

    connect(ui->pushButtonPostPreviousPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::max(1ul, mCurrentPostPage - 1);
                reload();
            });

    connect(ui->pushButtonPostNextPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::min(mCurrentPostPageCount, mCurrentPostPage + 1);
                reload();
            });

    connect(ui->pushButtonPostFirstPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = 1;
                reload();
            });

    connect(ui->pushButtonPostLastPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = mCurrentPostPageCount;
                reload();
            });

    connect(ui->pushButtonPostPageNumber, &QPushButton::clicked,
            [&]()
            {
                mMainWindow->showJumpToPageDialog(mCurrentPostPage, mCurrentPostPageCount,
                                                  [&](uint64_t page)
                                                  {
                                                      mCurrentPostPage = page;
                                                      reload();
                                                  });
            });

    connect(ui->pushButtonToggleShowUnread, &QPushButton::clicked,
            [&]()
            {
                mShowOnlyUnreadPosts = !mShowOnlyUnreadPosts;
                if (mShowOnlyUnreadPosts)
                {
                    mCurrentPostPage = 1;
                    mMainWindow->getUI()->pushButtonToggleShowUnread->setText(tr("Show all posts"));
                }
                else
                {
                    mMainWindow->getUI()->pushButtonToggleShowUnread->setText(tr("Show only unread posts"));
                }
                reload();
                updateActivePostFilter();
            });

    connect(mPostWebEnginePage.get(), &QWebEnginePage::linkHovered,
            [&](const QString& url)
            {
                if (mMainWindow->getUI()->stackedWidgetPost->currentIndex() == PostPane::Post)
                {
                    if (!url.isEmpty())
                    {
                        QUrl uri(url);
                        if (uri.scheme() != "zapfr" && !url.startsWith("javascript:markAsRead("))
                        {
                            mMainWindow->getUI()->statusbar->showMessage(url);
                        }
                    }
                    else
                    {
                        mMainWindow->getUI()->statusbar->clearMessage();
                    }
                }
            });
}

void ZapFR::Client::TableViewPosts::createContextMenus()
{
    mPostContextMenu = std::make_unique<QMenu>(nullptr);
    mPostContextMenu->addAction(mActionMarkSelectionAsRead.get());
    mPostContextMenu->addAction(mActionMarkSelectionAsUnread.get());
    mPostContextMenu->addSeparator();

    // flag submenu
    static auto nameForColor = [](ZapFR::Engine::FlagColor color) -> QString
    {
        switch (color)
        {
            case ZapFR::Engine::FlagColor::Blue:
            {
                return tr("Blue");
            }
            case ZapFR::Engine::FlagColor::Green:
            {
                return tr("Green");
            }
            case ZapFR::Engine::FlagColor::Yellow:
            {
                return tr("Yellow");
            }
            case ZapFR::Engine::FlagColor::Orange:
            {
                return tr("Orange");
            }
            case ZapFR::Engine::FlagColor::Red:
            {
                return tr("Red");
            }
            case ZapFR::Engine::FlagColor::Purple:
            {
                return tr("Purple");
            }
            default:
            {
                return QString("");
            }
        }
    };
    static std::vector<std::unique_ptr<QAction>> flagActions{};
    if (flagActions.empty())
    {
        for (const auto& flagColor : ZapFR::Engine::Flag::allFlagColors())
        {
            QString colorName = nameForColor(flagColor);
            if (!colorName.isEmpty())
            {
                auto action = std::make_unique<QAction>(colorName);
                action->setIcon(Utilities::flag(flagColor, Utilities::FlagStyle::Filled));
                action->setData(QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flagColor)));
                connect(action.get(), &QAction::triggered, this, &TableViewPosts::markPostSelectionFlagged);
                flagActions.emplace_back(std::move(action));
            }
        }
    }
    auto flagMenu = mPostContextMenu->addMenu(tr("Flag"));
    for (const auto& fa : flagActions)
    {
        flagMenu->addAction(fa.get());
    }

    // unflag submenu
    static std::vector<std::unique_ptr<QAction>> unflagActions{};
    if (unflagActions.empty())
    {
        auto allAction = std::make_unique<QAction>(tr("All"));
        allAction->setIcon(Utilities::flag(ZapFR::Engine::FlagColor::Gray, Utilities::FlagStyle::Unfilled));
        allAction->setData(QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(ZapFR::Engine::FlagColor::Gray)));
        connect(allAction.get(), &QAction::triggered, this, &TableViewPosts::markPostSelectionUnflagged);
        unflagActions.emplace_back(std::move(allAction));

        for (const auto& flagColor : ZapFR::Engine::Flag::allFlagColors())
        {
            QString colorName = nameForColor(flagColor);
            if (!colorName.isEmpty())
            {
                auto action = std::make_unique<QAction>(colorName);
                action->setIcon(Utilities::flag(flagColor, Utilities::FlagStyle::Unfilled));
                action->setData(QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flagColor)));
                connect(action.get(), &QAction::triggered, this, &TableViewPosts::markPostSelectionUnflagged);
                unflagActions.emplace_back(std::move(action));
            }
        }
    }
    auto unflagMenu = mPostContextMenu->addMenu(tr("Unflag"));
    for (const auto& ua : unflagActions)
    {
        unflagMenu->addAction(ua.get());
    }

    mPostContextMenu->addSeparator();

    auto addToScriptFolderMenu = mPostContextMenu->addMenu(tr("Add to script folder"));
    addToScriptFolderMenu->setProperty(gsAddToScriptFolderMenuProperty, true);

    auto removeFromScriptFolderMenu = mPostContextMenu->addMenu(tr("Remove from script folder"));
    removeFromScriptFolderMenu->setProperty(gsRemoveFromScriptFolderMenuProperty, true);

    mPostContextMenu->addAction(mActionCopyForTestScript.get());

    mPostContextMenu->addSeparator();
    mPostContextMenu->addAction(mActionOpenInExternalBrowser.get());
}
