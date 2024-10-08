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

#ifndef ZAPFR_CLIENT_DIALOGADDFEED_H
#define ZAPFR_CLIENT_DIALOGADDFEED_H

#include <QAbstractButton>

#include "../ClientGlobal.h"
#include "DialogWithSourcesAndFolders.h"
#include "ZapFR/feed_handling/FeedDiscovery.h"

namespace Ui
{
    class DialogAddFeed;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogAddFeed : public DialogWithSourcesAndFolders
        {
            Q_OBJECT

          public:
            explicit DialogAddFeed(QWidget* parent = nullptr);
            ~DialogAddFeed();
            DialogAddFeed(const DialogAddFeed& e) = delete;
            DialogAddFeed& operator=(const DialogAddFeed&) = delete;
            DialogAddFeed(DialogAddFeed&&) = delete;
            DialogAddFeed& operator=(DialogAddFeed&&) = delete;

            void reset(uint64_t selectedSourceID, uint64_t selectedFolderID);
            std::vector<std::string> feedURLsToAdd() const;

            enum Column
            {
                CheckboxCol = 0,
                TypeCol = 1,
                TitleCol = 2,
                URLCol = 3,
            };

            enum Role
            {
                URL = Qt::ItemDataRole::UserRole + 1,
                IsChecked = Qt::ItemDataRole::UserRole + 2,
            };

          private slots:
            void discoverFeeds();

          private:
            Ui::DialogAddFeed* ui;
            QAbstractButton* mAddFeedButton{nullptr};
            std::unique_ptr<QStandardItemModel> mDiscoveredFeedsModel{nullptr};

            QString url() const;
            void addDiscoveredFeed(const ZapFR::Engine::DiscoveredFeed& discoveredFeed);
            void clearDiscoveredFeeds();
            void updateAddButtonState();
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGADDFEED_H
