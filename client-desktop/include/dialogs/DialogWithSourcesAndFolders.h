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

#ifndef ZAPFR_CLIENT_DIALOGWITHSOURCESANDFOLDERS_H
#define ZAPFR_CLIENT_DIALOGWITHSOURCESANDFOLDERS_H

#include "../ClientGlobal.h"
#include "ZapFR/Source.h"

namespace ZapFR
{
    namespace Client
    {
        class DialogWithSourcesAndFolders : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogWithSourcesAndFolders(QWidget* parent = nullptr);
            ~DialogWithSourcesAndFolders() = default;
            DialogWithSourcesAndFolders(const DialogWithSourcesAndFolders& e) = delete;
            DialogWithSourcesAndFolders& operator=(const DialogWithSourcesAndFolders&) = delete;
            DialogWithSourcesAndFolders(DialogWithSourcesAndFolders&&) = delete;
            DialogWithSourcesAndFolders& operator=(DialogWithSourcesAndFolders&&) = delete;

            uint64_t selectedFolderID() const;
            uint64_t selectedSourceID() const;

            void setComboBoxSources(QComboBox* cb);
            void setComboBoxFolders(QComboBox* cb);
            void setPreselectedSourceAndFolderIDs(uint64_t selectedSourceID, uint64_t selectedFolderID);

          private slots:
            void currentSourceChanged(int index);

          private:
            QComboBox* mComboBoxSources{nullptr};
            QComboBox* mComboBoxFolders{nullptr};
            std::unique_ptr<QStandardItemModel> mSourcesModel{nullptr};
            std::unique_ptr<QStandardItemModel> mFoldersModel{nullptr};
            int64_t mFolderIDToPreselect{-1};

            static constexpr uint32_t SourceIDRole{Qt::ItemDataRole::UserRole + 1};
            static constexpr uint32_t FolderIDRole{Qt::ItemDataRole::UserRole + 2};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGWITHSOURCESANDFOLDERS_H
