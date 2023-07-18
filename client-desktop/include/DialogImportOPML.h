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

#ifndef ZAPFR_CLIENT_DIALOGIMPORTOPML_H
#define ZAPFR_CLIENT_DIALOGIMPORTOPML_H

#include "ClientGlobal.h"
#include "OPMLParser.h"
#include "Source.h"

namespace Ui
{
    class DialogImportOPML;
}

namespace ZapFR
{
    namespace Client
    {
        class DialogImportOPML : public QDialog
        {
            Q_OBJECT

          public:
            explicit DialogImportOPML(QWidget* parent = nullptr);
            ~DialogImportOPML();

            void reset(const std::vector<std::unique_ptr<ZapFR::Engine::Source>>& sources);
            std::vector<ZapFR::Engine::OPMLEntry> importedFeeds() const noexcept;
            QString folderHierarchy() const;
            uint64_t sourceID() const;

          private slots:
            void chooseOPMLFile(bool checked);

          private:
            Ui::DialogImportOPML* ui;
            std::unique_ptr<QStandardItemModel> mSourcesModel;
            std::vector<ZapFR::Engine::OPMLEntry> mImportedFeeds;

            static constexpr uint32_t SourceIDRole{Qt::ItemDataRole::UserRole + 1};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_DIALOGIMPORTOPML_H
