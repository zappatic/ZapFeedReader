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

#ifndef ZAPFR_CLIENT_DIALOGADDFOLDER_H
#define ZAPFR_CLIENT_DIALOGADDFOLDER_H

#include "ClientGlobal.h"
#include "Source.h"

namespace Ui
{
    class DialogAddFolder;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogAddFolder : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogAddFolder(QWidget* parent = nullptr);
            ~DialogAddFolder();

            QString title() const;
            uint64_t addUnderFolder() const;
            uint64_t sourceID() const;

            void reset(const std::vector<std::unique_ptr<ZapFR::Engine::Source>>& sources, uint64_t selectedSourceID, uint64_t selectedFolderID);

          private slots:
            void currentSourceChanged(int index);

          private:
            Ui::DialogAddFolder* ui;
            std::unique_ptr<QStandardItemModel> mSourcesModel;
            std::unique_ptr<QStandardItemModel> mFoldersModel;
            int64_t mFolderIDToPreselect{-1};

            static constexpr uint32_t SourceIDRole{Qt::ItemDataRole::UserRole + 1};
            static constexpr uint32_t FolderIDRole{Qt::ItemDataRole::UserRole + 2};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGADDFOLDER_H
