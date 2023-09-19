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

#include "./ui_MainWindow.h"
#include "ZapFR/Agent.h"
#include "ZapFR/base/Post.h"
#include "delegates/ItemDelegatePost.h"
#include "widgets/LineEditSearch.h"
#include "widgets/MainWindow.h"
#include "widgets/WebEnginePagePost.h"

namespace
{
    static auto gsAddToScriptFolderMenuProperty{"AddToScriptFolderMenuProperty"};
    static auto gsRemoveFromScriptFolderMenuProperty{"RemoveFromScriptFolderMenuProperty"};
} // namespace

void ZapFR::Client::MainWindow::reloadPosts()
{
    // lambda to assign the correct role data to the table entries
    auto setItemData = [&](QStandardItem* item, ZapFR::Engine::Post* post, uint64_t sourceID)
    {
        item->setData(QVariant::fromValue<uint64_t>(post->id()), PostIDRole);
        item->setData(QVariant::fromValue<uint64_t>(sourceID), PostSourceIDRole);
        item->setData(QVariant::fromValue<uint64_t>(post->feedID()), PostFeedIDRole);
        item->setData(QVariant::fromValue<bool>(post->isRead()), PostIsReadRole);
        item->setData(QString::fromUtf8(post->link()), PostLinkRole);
    };

    // lambda for the callback, retrieving the posts
    auto processPosts = [&](uint64_t sourceID, const std::vector<ZapFR::Engine::Post*>& posts, uint64_t pageNumber, uint64_t totalPostCount)
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
            flagItem->setData(flagColors, PostAppliedFlagsRole);

            auto feedItem = new QStandardItem("");
            setItemData(feedItem, post, sourceID);
            feedItem->setData(QString::fromUtf8(post->feedTitle()), Qt::ToolTipRole);

            auto titleItem = new QStandardItem(QString::fromUtf8(post->title()));
            setItemData(titleItem, post, sourceID);

            auto datePublished = QString::fromUtf8(post->datePublished());
            auto dateItem = new QStandardItem(Utilities::prettyDate(datePublished));
            dateItem->setData(datePublished, PostISODateRole);
            setItemData(dateItem, post, sourceID);

            QList<QStandardItem*> rowData;
            rowData << unreadItem << flagItem << feedItem << titleItem << dateItem;
            rows << rowData;
        }

        QMetaObject::invokeMethod(this, [&]() { populatePosts(rows, pageNumber, totalPostCount); });
    };

    auto searchFilter = mLineEditSearch->text().toStdString();

    auto index = ui->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(ScriptFolderSourceIDRole).toULongLong();
        auto scriptFolderID = index.data(ScriptFolderIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueGetScriptFolderPosts(sourceID, scriptFolderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, searchFilter,
                                                                       mFlagFilter, processPosts);
    }
    else
    {
        index = ui->treeViewSources->currentIndex();
        if (index.isValid())
        {
            if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFeedPosts(sourceID, feedID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, searchFilter, mFlagFilter,
                                                                       processPosts);
            }
            else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FOLDER)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetFolderPosts(sourceID, folderID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, searchFilter, mFlagFilter,
                                                                         processPosts);
            }
            else if (index.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_SOURCE)
            {
                auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueGetSourcePosts(sourceID, msPostsPerPage, mCurrentPostPage, mShowOnlyUnreadPosts, searchFilter, mFlagFilter,
                                                                         processPosts);
            }
            else
            {
                populatePosts();
            }
            updateToolbar();
        }
    }
}

void ZapFR::Client::MainWindow::populatePosts(const QList<QList<QStandardItem*>>& posts, uint64_t pageNumber, uint64_t totalPostCount)
{
    ui->stackedWidgetContentPanes->setCurrentIndex(StackedPanePosts);

    int32_t columnWidthUnread = 50;
    int32_t columnWidthFlag = 40;
    int32_t columnWidthFeed = 40;
    int32_t columnWidthDate = 180;
    // restore the previous column widths
    if (mItemModelPosts != nullptr)
    {
        columnWidthUnread = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnUnread);
        columnWidthFlag = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnFlag);
        columnWidthFeed = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnFeed);
        columnWidthDate = ui->tableViewPosts->horizontalHeader()->sectionSize(PostColumnDate);
    }

    mItemModelPosts = std::make_unique<QStandardItemModel>(this);
    ui->tableViewPosts->setModel(mItemModelPosts.get());
    mItemModelPosts->setHorizontalHeaderItem(PostColumnUnread, new QStandardItem(tr("Unread")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnFlag, new QStandardItem(tr("Flag")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnFeed, new QStandardItem(tr("Feed")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnTitle, new QStandardItem(tr("Title")));
    mItemModelPosts->setHorizontalHeaderItem(PostColumnDate, new QStandardItem(tr("Date")));
    for (const auto& post : posts)
    {
        mItemModelPosts->appendRow(post);
    }
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableViewPosts->horizontalHeader()->setSectionResizeMode(PostColumnTitle, QHeaderView::Stretch);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnUnread, columnWidthUnread);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnFlag, columnWidthFlag);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnFeed, columnWidthFeed);
    ui->tableViewPosts->horizontalHeader()->resizeSection(PostColumnDate, columnWidthDate);
    postsTableViewSelectionChanged({});

    // in case we have just 1 feed selected, hide the feed column in the posts table
    // if we have a script folder selected, always show the feed column
    ui->tableViewPosts->setColumnHidden(PostColumnFeed, false);
    auto treeViewSourcesIndex = ui->treeViewSources->currentIndex();
    auto tableViewScriptFoldersIndex = ui->tableViewScriptFolders->currentIndex();
    if (!tableViewScriptFoldersIndex.isValid() && treeViewSourcesIndex.isValid() && treeViewSourcesIndex.data(SourceTreeEntryTypeRole) == SOURCETREE_ENTRY_TYPE_FEED)
    {
        ui->tableViewPosts->setColumnHidden(PostColumnFeed, true);
    }

    mCurrentPostCount = totalPostCount;
    mCurrentPostPage = pageNumber;
    mCurrentPostPageCount = 1;
    if (mCurrentPostCount > 0)
    {
        mCurrentPostPageCount = static_cast<uint64_t>(std::ceil(static_cast<float>(mCurrentPostCount) / static_cast<float>(msPostsPerPage)));
    }

    ui->pushButtonPostPageNumber->setText(QString("%1 %2 / %3").arg(tr("Page")).arg(mCurrentPostPage).arg(mCurrentPostPageCount));
    ui->labelTotalPostCount->setText(tr("%n post(s)", "", static_cast<int32_t>(mCurrentPostCount)));
}

std::vector<std::tuple<uint64_t, uint64_t>> ZapFR::Client::MainWindow::selectedPostIDs() const
{
    std::vector<std::tuple<uint64_t, uint64_t>> feedAndPostIDs;
    auto selectionModel = ui->tableViewPosts->selectionModel();
    if (selectionModel != nullptr)
    {
        auto selectedIndexes = selectionModel->selectedIndexes();
        for (const auto& index : selectedIndexes)
        {
            if (index.column() == PostColumnUnread)
            {
                auto feedID = index.data(PostFeedIDRole).toULongLong();
                auto postID = index.data(PostIDRole).toULongLong();
                feedAndPostIDs.emplace_back(std::make_tuple(feedID, postID));
            }
        }
    }
    return feedAndPostIDs;
}

void ZapFR::Client::MainWindow::markAsRead()
{
    auto index = ui->treeViewSources->currentIndex();
    if (index.isValid())
    {
        auto sourceID = index.data(SourceTreeEntryParentSourceIDRole).toULongLong();
        auto type = index.data(SourceTreeEntryTypeRole).toULongLong();
        switch (type)
        {
            case SOURCETREE_ENTRY_TYPE_FEED:
            {
                auto feedID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFeedRead(sourceID, feedID,
                                                                       [&](uint64_t affectedSourceID, uint64_t affectedFeedID)
                                                                       { QMetaObject::invokeMethod(this, [&]() { feedMarkedRead(affectedSourceID, affectedFeedID); }); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_FOLDER:
            {
                auto folderID = index.data(SourceTreeEntryIDRole).toULongLong();
                ZapFR::Engine::Agent::getInstance()->queueMarkFolderRead(sourceID, folderID,
                                                                         [&](uint64_t affectedSourceID, std::unordered_set<uint64_t> feedIDs)
                                                                         { QMetaObject::invokeMethod(this, [&]() { folderMarkedRead(affectedSourceID, feedIDs); }); });
                break;
            }
            case SOURCETREE_ENTRY_TYPE_SOURCE:
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkSourceRead(sourceID, [&](uint64_t affectedSourceID)
                                                                         { QMetaObject::invokeMethod(this, [&]() { sourceMarkedRead(affectedSourceID); }); });
                break;
            }
        }
    }
}

void ZapFR::Client::MainWindow::markPostSelectionAsRead()
{
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsRead(sourceID, feedAndPostIDs,
                                                                [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs)
                                                                { QMetaObject::invokeMethod(this, [&]() { postsMarkedRead(affectedSourceID, affectedFeedAndPostIDs); }); });
    }
}

void ZapFR::Client::MainWindow::markPostSelectionAsUnread()
{
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnread(sourceID, feedAndPostIDs,
                                                                  [&](uint64_t affectedSourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& affectedFeedAndPostIDs) {
                                                                      QMetaObject::invokeMethod(this, [&]() { postsMarkedUnread(affectedSourceID, affectedFeedAndPostIDs); });
                                                                  });
    }
}

void ZapFR::Client::MainWindow::markPostSelectionFlagged()
{
    auto action = qobject_cast<QAction*>(sender());
    auto flagColor = static_cast<ZapFR::Engine::FlagColor>(action->data().toULongLong());

    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsFlagged(sourceID, feedAndPostIDs, {flagColor},
                                                                   [&]() { QMetaObject::invokeMethod(this, [&]() { postsMarkedFlagged(true); }); });
    }
}

void ZapFR::Client::MainWindow::markPostSelectionUnflagged()
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
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(sourceID, feedAndPostIDs, flagColors,
                                                                     [&]() { QMetaObject::invokeMethod(this, [&]() { postsMarkedUnflagged(true); }); });
    }
}

void ZapFR::Client::MainWindow::assignPostSelectionToScriptFolder()
{
    auto action = qobject_cast<QAction*>(sender());
    auto scriptFolderID = action->data().toULongLong();
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueAssignPostsToScriptFolder(
            sourceID, scriptFolderID, feedAndPostIDs,
            [&](uint64_t affectedSourceID, uint64_t affectedScriptFolderID)
            { QMetaObject::invokeMethod(this, [&]() { postsAssignedToScriptFolder(affectedSourceID, affectedScriptFolderID); }); });
    }
}

void ZapFR::Client::MainWindow::removePostSelectionFromScriptFolder()
{
    auto action = qobject_cast<QAction*>(sender());
    auto scriptFolderID = action->data().toULongLong();
    auto feedAndPostIDs = selectedPostIDs();
    if (feedAndPostIDs.size() > 0)
    {
        auto sourceID = ui->treeViewSources->currentIndex().data(SourceTreeEntryParentSourceIDRole).toULongLong();
        ZapFR::Engine::Agent::getInstance()->queueRemovePostsFromScriptFolder(
            sourceID, scriptFolderID, feedAndPostIDs,
            [&](uint64_t affectedSourceID, uint64_t affectedScriptFolderID)
            { QMetaObject::invokeMethod(this, [&]() { postsRemovedFromScriptFolder(affectedSourceID, affectedScriptFolderID); }); });
    }
}

void ZapFR::Client::MainWindow::postsTableViewSelectionChanged(const QModelIndexList& selected)
{
    mCurrentPostID = 0;
    mCurrentPostSourceID = 0;
    mCurrentPostFeedID = 0;

    if (selected.count() == 1)
    {
        auto index = selected.at(0);
        if (index.isValid())
        {
            mCurrentPostID = index.data(PostIDRole).toULongLong();
            mCurrentPostSourceID = index.data(PostSourceIDRole).toULongLong();
            mCurrentPostFeedID = index.data(PostFeedIDRole).toULongLong();

            auto isRead = index.data(PostIsReadRole).toBool();
            if (!isRead)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostsRead(mCurrentPostSourceID, {{mCurrentPostFeedID, mCurrentPostID}},
                                                                        [&](uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& feedAndPostIDs)
                                                                        { QMetaObject::invokeMethod(this, [&]() { postsMarkedRead(sourceID, feedAndPostIDs); }); });
            }
            reloadCurrentPost();
        }
    }
    else if (selected.count() == 0)
    {
        setBlankPostPage();
        ui->widgetPostCaption->setCaption(tr("No post selected"), this);
        ui->stackedWidgetPost->setCurrentIndex(StackedPanePostCaption);
    }
    else
    {
        setBlankPostPage();
        ui->widgetPostCaption->setCaption(tr("%1 posts selected").arg(selected.count()), this);
        ui->stackedWidgetPost->setCurrentIndex(StackedPanePostCaption);
    }
}

void ZapFR::Client::MainWindow::reloadCurrentPost()
{
    if (mItemModelPostEnclosures != nullptr)
    {
        mItemModelPostEnclosures->clear();
        mItemModelPostEnclosures->setHorizontalHeaderItem(PostEnclosuresColumnIcon, new QStandardItem(""));
        mItemModelPostEnclosures->setHorizontalHeaderItem(PostEnclosuresColumnURL, new QStandardItem(tr("URL")));
        mItemModelPostEnclosures->setHorizontalHeaderItem(PostEnclosuresColumnMimetype, new QStandardItem(tr("Type")));
        mItemModelPostEnclosures->setHorizontalHeaderItem(PostEnclosuresColumnFilesize, new QStandardItem(tr("Size")));
        ui->tableViewPostEnclosures->horizontalHeader()->setMinimumSectionSize(25);
        ui->tableViewPostEnclosures->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableViewPostEnclosures->horizontalHeader()->setSectionResizeMode(PostEnclosuresColumnURL, QHeaderView::Stretch);
        ui->tableViewPostEnclosures->horizontalHeader()->resizeSection(PostEnclosuresColumnMimetype, 250);
        ui->tableViewPostEnclosures->horizontalHeader()->resizeSection(PostEnclosuresColumnIcon, 25);
    }

    if (mCurrentPostSourceID > 0 && mCurrentPostFeedID > 0 && mCurrentPostID > 0)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetPost(mCurrentPostSourceID, mCurrentPostFeedID, mCurrentPostID,
                                                          [&](std::unique_ptr<ZapFR::Engine::Post> post)
                                                          {
                                                              Poco::URI uri(post->feedLink());
                                                              uri.setPath("");

                                                              std::unordered_map<std::string, QString> replacers;
                                                              replacers["BASE"] = QString::fromUtf8(uri.toString());
                                                              replacers["STYLES"] = postStyles();
                                                              replacers["POST.TITLE"] = QString::fromUtf8(post->title());
                                                              replacers["POST.LINK"] = QString::fromUtf8(post->link());
                                                              replacers["POST.AUTHOR"] = QString::fromUtf8(post->author());
                                                              replacers["POST.CONTENT"] = QString::fromUtf8(post->content());
                                                              replacers["POST.DATE_PUBLISHED"] = Utilities::prettyDate(QString::fromUtf8(post->datePublished()));
                                                              replacers["POST.COMMENTS_URL"] = QString::fromUtf8(post->commentsURL());
                                                              replacers["I18N.PUBLISHED"] = tr("Published");
                                                              replacers["I18N.AUTHOR"] = tr("Author");
                                                              replacers["I18N.VIEWCOMMENTS"] = tr("View comments");

                                                              auto postHTML = postHTMLTemplate();
                                                              for (const auto& [key, value] : replacers)
                                                              {
                                                                  // handle {if}{else}{/if}
                                                                  auto pattern = QString::fromUtf8(fmt::format(R"(\[if {}\](.*?)\[else\](.*?)\[/if\])", key));
                                                                  postHTML.replace(QRegularExpression(pattern), value.isEmpty() ? R"(\2)" : R"(\1)");

                                                                  // handle [if][/if]
                                                                  pattern = QString::fromUtf8(fmt::format(R"(\[if {}\](.*?)\[/if\])", key));
                                                                  postHTML.replace(QRegularExpression(pattern), value.isEmpty() ? "" : R"(\1)");

                                                                  postHTML.replace(QString::fromUtf8(fmt::format("[{}]", key)), value);
                                                              }

                                                              QMetaObject::invokeMethod(this, [&]() { postReadyToBeShown(postHTML, post->enclosures()); });
                                                          });
    }
    else
    {
        setBlankPostPage();
        ui->widgetPostCaption->setCaption(tr("No post selected"), this);
        ui->stackedWidgetPost->setCurrentIndex(StackedPanePostCaption);
    }
}

void ZapFR::Client::MainWindow::postReadyToBeShown(const QString& html, const std::vector<ZapFR::Engine::Post::Enclosure>& enclosures)
{
    setPostHTML(html);
    QMimeDatabase mimeDB;

    if (mItemModelPostEnclosures != nullptr)
    {
        for (const auto& e : enclosures)
        {
            auto mimeType = mimeDB.mimeTypeForName(QString::fromUtf8(e.mimeType));
            auto url = QString::fromUtf8(e.url);

            auto icon = mimeType.iconName();
            if (icon.isEmpty())
            {
                icon = mimeType.genericIconName();
            }

            auto iconItem = new QStandardItem("");
            iconItem->setData(QIcon::fromTheme(icon), Qt::DecorationRole);
            iconItem->setData(url, PostEnclosureLinkRole);

            auto urlItem = new QStandardItem(url);
            urlItem->setData(url, Qt::ToolTipRole);
            urlItem->setData(url, PostEnclosureLinkRole);

            auto mimeTypeItem = new QStandardItem(mimeType.name());
            mimeTypeItem->setData(url, PostEnclosureLinkRole);

            auto sizeCaption = tr("Unknown");
            if (e.size > 0)
            {
                sizeCaption = locale().formattedDataSize(static_cast<int64_t>(e.size));
            }
            auto sizeItem = new QStandardItem(sizeCaption);
            sizeItem->setData(url, PostEnclosureLinkRole);

            QList<QStandardItem*> rowData;
            rowData << iconItem << urlItem << mimeTypeItem << sizeItem;

            mItemModelPostEnclosures->appendRow(rowData);
        }
    }

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

void ZapFR::Client::MainWindow::setPostHTML(const QString& html) const
{
    ui->webViewPost->setHtml(html);
    ui->stackedWidgetPost->setCurrentIndex(StackedPanePost);
}

QString ZapFR::Client::MainWindow::postStyles() const
{
    static QString cache{""};

    if (!mPostStylesCacheValid)
    {
        auto font = ui->treeViewSources->font();
        auto palette = QPalette(ui->treeViewSources->palette());

        QString overrideFilename;
        QString backgroundColor;
        QString textColor;
        QColor highlightColor = palette.color(QPalette::Active, QPalette::Highlight);
        QString bodyFontSize = QString::number(mPreferencePostFontSize);

        if (getCurrentColorTheme() == Theme::Dark)
        {
            overrideFilename = "posttheme.dark.css";
            backgroundColor = "#2a2a2a";
            textColor = "#fff";
        }
        else
        {
            overrideFilename = "posttheme.light.css";
            backgroundColor = "#fff";
            textColor = "#000";
        }

        auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + overrideFilename));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            auto styles = QString::fromUtf8(override.readAll());
            override.close();
            return styles;
        }

        cache = QString(R"(body { font-family: "%1", sans-serif; font-size: %5px; background-color: %2; color: %3; margin: 2px 25px; })"
                        "\n"
                        "a { color: %4; }\n"
                        ".zapfr_title { color: %3; font-size: 1.4em; font-weight: bold; text-decoration: none; display: block; margin: 25px 0 10px 0; user-select:none; }\n"
                        ".zapfr_infoheader { font-size: 0.75em; display: flex; gap: 10px; }\n"
                        ".zapfr_infoheader_separator { display: inline-block; margin-right: 10px; }\n"
                        ".zapfr_divider { margin-bottom: 30px; height: 1px; border: none; color: %3; background-color: %3; }\n")
                    .arg(font.family())
                    .arg(backgroundColor)
                    .arg(textColor)
                    .arg(highlightColor.name())
                    .arg(bodyFontSize);
        mPostStylesCacheValid = true;
    }

    return cache;
}

QString ZapFR::Client::MainWindow::postHTMLTemplate() const
{
    static std::optional<QString> cache{};
    if (!cache.has_value())
    {
        auto override = QFile(QDir::cleanPath(configDir() + QDir::separator() + "post.html"));
        if (override.exists())
        {
            override.open(QIODeviceBase::ReadOnly);
            cache = QString::fromUtf8(override.readAll());
            override.close();
        }
        else
        {
            cache = R"(<!DOCTYPE html>
<html>
    <head>
        <base href="[BASE]">
        <style type="text/css">[STYLES]</style>
    </head>
    <body>
        [if POST.LINK]<a class="zapfr_title" href="[POST.LINK]">[POST.TITLE]</a>[else]<h1 class="zapfr_title">[POST.TITLE]</h1>[/if]
        <div class="zapfr_infoheader">
            <div>[I18N.PUBLISHED]: [POST.DATE_PUBLISHED]</div>
            [if POST.AUTHOR]<div><span class="zapfr_infoheader_separator">|</span>[I18N.AUTHOR]: [POST.AUTHOR]</div>[/if]
            [if POST.COMMENTS_URL]<div><span class="zapfr_infoheader_separator">|</span><a href="[POST.COMMENTS_URL]">[I18N.VIEWCOMMENTS]</a></div>[/if]
        </div>
        <hr class="zapfr_divider">
        [POST.CONTENT]
    </body>
</html>)";
        }
    }
    return cache.value();
}

void ZapFR::Client::MainWindow::setBlankPostPage() const
{
    QString htmlStr;
    QTextStream html(&htmlStr, QIODeviceBase::ReadWrite);
    html << "<!DOCTYPE html>\n<html><head><style type='text/css'>\n" << postStyles() << "</style></head><body></body></html>";
    setPostHTML(htmlStr);
}

void ZapFR::Client::MainWindow::postsMarkedRead(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs)
{
    std::unordered_set<uint64_t> uniqueFeedIDs{};

    for (const auto& [feedID, postID] : postIDs)
    {
        uniqueFeedIDs.insert(feedID);
        for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
        {
            auto index = mItemModelPosts->index(i, 0);
            if (index.data(PostIDRole).toULongLong() == postID)
            {
                for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
                {
                    auto item = mItemModelPosts->item(i, col);
                    item->setData(QVariant::fromValue<bool>(true), PostIsReadRole);
                }
            }
        }
    }

    for (const auto& feedID : uniqueFeedIDs)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(sourceID, feedID,
                                                                     [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
                                                                     {
                                                                         std::unordered_set<uint64_t> feedIDs;
                                                                         feedIDs.insert(affectedFeedID);
                                                                         QMetaObject::invokeMethod(
                                                                             this, [&]() { updateFeedUnreadCountBadge(affectedSourceID, feedIDs, false, unreadCount); });
                                                                     });
    }

    reloadScriptFolders(true);
    // no statusbar update here, as it's called just for clicking/reading a post in the table, which would be distracting
}

void ZapFR::Client::MainWindow::postsMarkedUnread(uint64_t sourceID, const std::vector<std::tuple<uint64_t, uint64_t>>& postIDs)
{
    std::unordered_set<uint64_t> uniqueFeedIDs{};

    for (const auto& [feedID, postID] : postIDs)
    {
        uniqueFeedIDs.insert(feedID);
        for (int32_t i = 0; i < mItemModelPosts->rowCount(); ++i)
        {
            auto index = mItemModelPosts->index(i, 0);
            if (index.data(PostIDRole).toULongLong() == postID)
            {
                for (int32_t col = 0; col < mItemModelPosts->columnCount(); ++col)
                {
                    auto item = mItemModelPosts->item(i, col);
                    item->setData(QVariant::fromValue<bool>(false), PostIsReadRole);
                }
            }
        }
    }

    for (const auto& feedID : uniqueFeedIDs)
    {
        ZapFR::Engine::Agent::getInstance()->queueGetFeedUnreadCount(sourceID, feedID,
                                                                     [&](uint64_t affectedSourceID, uint64_t affectedFeedID, uint64_t unreadCount)
                                                                     {
                                                                         std::unordered_set<uint64_t> feedIDs;
                                                                         feedIDs.insert(affectedFeedID);
                                                                         QMetaObject::invokeMethod(
                                                                             this, [&]() { updateFeedUnreadCountBadge(affectedSourceID, feedIDs, false, unreadCount); });
                                                                     });
    }

    reloadScriptFolders(true);
    ui->statusbar->showMessage(tr("Post(s) marked as unread"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::postsMarkedFlagged(bool doReloadPosts)
{
    reloadUsedFlagColors(true);
    if (doReloadPosts)
    {
        reloadPosts();
    }
    ui->statusbar->showMessage(tr("Post(s) marked as flagged"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::postsMarkedUnflagged(bool doReloadPosts)
{
    reloadUsedFlagColors(true);
    if (doReloadPosts)
    {
        reloadPosts();
    }
    ui->statusbar->showMessage(tr("Post(s) marked as unflagged"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::postsAssignedToScriptFolder(uint64_t /*sourceID*/, uint64_t /*scriptFolderID*/)
{
    reloadScriptFolders(true);
    ui->statusbar->showMessage(tr("Post(s) assigned to script folder"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::postsRemovedFromScriptFolder(uint64_t sourceID, uint64_t scriptFolderID)
{
    reloadScriptFolders(true);
    auto index = ui->tableViewScriptFolders->currentIndex();
    if (index.isValid())
    {
        auto selectedSourceID = index.data(ScriptFolderSourceIDRole).toULongLong();
        if (sourceID != selectedSourceID)
        {
            return;
        }
        auto selectedScriptFolderID = index.data(ScriptFolderIDRole).toULongLong();
        if (scriptFolderID != selectedScriptFolderID)
        {
            return;
        }
        reloadPosts();
    }
    ui->statusbar->showMessage(tr("Post(s) unassigned from script folder"), StatusBarDefaultTimeout);
}

void ZapFR::Client::MainWindow::connectPostStuff()
{
    connect(ui->action_Mark_as_read, &QAction::triggered, this, &MainWindow::markAsRead);

    connect(ui->action_Mark_selection_as_unread, &QAction::triggered, this, &MainWindow::markPostSelectionAsUnread);

    connect(ui->action_Mark_selection_as_read, &QAction::triggered, this, &MainWindow::markPostSelectionAsRead);

    connect(ui->action_Back_to_posts, &QAction::triggered, [&]() { ui->stackedWidgetContentPanes->setCurrentIndex(StackedPanePosts); });

    connect(ui->tableViewPosts, &TableViewPosts::selectedPostsChanged, this, &MainWindow::postsTableViewSelectionChanged);

    connect(ui->tableViewPosts, &TableViewPosts::customContextMenuRequested,
            [&](const QPoint& p)
            {
                // gather the script folder title and IDs
                std::vector<std::tuple<QVariant, QString>> scriptFolderData;
                for (int32_t i = 0; i < mItemModelScriptFolders->rowCount(); ++i)
                {
                    auto child = mItemModelScriptFolders->index(i, 0);
                    auto scriptFolderTitle = child.data(Qt::DisplayRole).toString();
                    auto scriptFolderID = child.data(ScriptFolderIDRole);
                    scriptFolderData.emplace_back(scriptFolderID, scriptFolderTitle);
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
                                    connect(childAction, &QAction::triggered, this, &MainWindow::assignPostSelectionToScriptFolder);
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
                                    connect(childAction, &QAction::triggered, this, &MainWindow::removePostSelectionFromScriptFolder);
                                }
                            }
                        }
                    }
                }
                mPostContextMenu->popup(ui->tableViewPosts->viewport()->mapToGlobal(p));
            });

    connect(ui->pushButtonPostPreviousPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::max(1ul, mCurrentPostPage - 1);
                reloadPosts();
            });

    connect(ui->pushButtonPostNextPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = std::min(mCurrentPostPageCount, mCurrentPostPage + 1);
                reloadPosts();
            });

    connect(ui->pushButtonPostFirstPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = 1;
                reloadPosts();
            });

    connect(ui->pushButtonPostLastPage, &QPushButton::clicked,
            [&]()
            {
                mCurrentPostPage = mCurrentPostPageCount;
                reloadPosts();
            });

    connect(ui->pushButtonPostPageNumber, &QPushButton::clicked,
            [&]()
            {
                showJumpToPageDialog(mCurrentPostPage, mCurrentPostPageCount,
                                     [&](uint64_t page)
                                     {
                                         mCurrentPostPage = page;
                                         reloadPosts();
                                     });
            });

    connect(ui->pushButtonToggleShowUnread, &QPushButton::clicked,
            [&]()
            {
                mShowOnlyUnreadPosts = !mShowOnlyUnreadPosts;
                if (mShowOnlyUnreadPosts)
                {
                    ui->pushButtonToggleShowUnread->setText(tr("Show all posts"));
                }
                else
                {
                    ui->pushButtonToggleShowUnread->setText(tr("Show only unread posts"));
                }
                reloadPosts();
                updateActivePostFilter();
            });

    connect(mPostWebEnginePage.get(), &QWebEnginePage::linkHovered,
            [&](const QString& url)
            {
                if (ui->stackedWidgetPost->currentIndex() == StackedPanePost)
                {
                    if (!url.isEmpty())
                    {
                        ui->statusbar->showMessage(url);
                    }
                    else
                    {
                        ui->statusbar->clearMessage();
                    }
                }
            });

    connect(ui->tableViewPosts, &TableViewPosts::postMarkedFlagged,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostsFlagged(sourceID, {{feedID, postID}}, {flagColor},
                                                                           [&]() { QMetaObject::invokeMethod(this, [&]() { postsMarkedFlagged(false); }); });
            });

    connect(ui->tableViewPosts, &TableViewPosts::postMarkedUnflagged,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID, ZapFR::Engine::FlagColor flagColor)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(sourceID, {{feedID, postID}}, {flagColor},
                                                                             [&]() { QMetaObject::invokeMethod(this, [&]() { postsMarkedUnflagged(false); }); });
            });

    connect(ui->tableViewPosts, &TableViewPosts::clearAllFlagsRequested,
            [&](uint64_t sourceID, uint64_t feedID, uint64_t postID)
            {
                ZapFR::Engine::Agent::getInstance()->queueMarkPostsUnflagged(sourceID, {{feedID, postID}}, ZapFR::Engine::Flag::allFlagColors(),
                                                                             [&]() { QMetaObject::invokeMethod(this, [&]() { postsMarkedUnflagged(false); }); });
            });

    connect(mLineEditSearch, &LineEditSearch::searchRequested,
            [&]()
            {
                updateActivePostFilter();
                reloadPosts();
            });
}

void ZapFR::Client::MainWindow::createPostContextMenus()
{
    mPostContextMenu = std::make_unique<QMenu>(nullptr);
    mPostContextMenu->addAction(ui->action_Mark_selection_as_read);
    mPostContextMenu->addAction(ui->action_Mark_selection_as_unread);
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
                connect(action.get(), &QAction::triggered, this, &MainWindow::markPostSelectionFlagged);
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
        connect(allAction.get(), &QAction::triggered, this, &MainWindow::markPostSelectionUnflagged);
        unflagActions.emplace_back(std::move(allAction));

        for (const auto& flagColor : ZapFR::Engine::Flag::allFlagColors())
        {
            QString colorName = nameForColor(flagColor);
            if (!colorName.isEmpty())
            {
                auto action = std::make_unique<QAction>(colorName);
                action->setIcon(Utilities::flag(flagColor, Utilities::FlagStyle::Unfilled));
                action->setData(QVariant(static_cast<std::underlying_type_t<ZapFR::Engine::FlagColor>>(flagColor)));
                connect(action.get(), &QAction::triggered, this, &MainWindow::markPostSelectionUnflagged);
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
}

void ZapFR::Client::MainWindow::initializeUIPosts()
{
    mPostWebEnginePage = std::make_unique<WebEnginePagePost>(this);
    ui->webViewPost->setPage(mPostWebEnginePage.get());

    ui->tableViewPosts->setItemDelegate(new ItemDelegatePost(ui->tableViewPosts));

    ui->stackedWidgetPost->setCurrentIndex(StackedPanePost);

    mItemModelPostEnclosures = std::make_unique<QStandardItemModel>(this);
    ui->tableViewPostEnclosures->setModel(mItemModelPostEnclosures.get());

    ui->tableViewPosts->setMainWindow(this);
}
