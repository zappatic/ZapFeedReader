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

#ifndef ZAPFR_CLIENT_WIDGETPROPERTIESPANEFOLDER_H
#define ZAPFR_CLIENT_WIDGETPROPERTIESPANEFOLDER_H

#include <QWidget>

namespace Ui
{
    class WidgetPropertiesPaneFolder;
}
namespace ZapFR
{
    namespace Client
    {
        class WidgetPropertiesPaneFolder : public QWidget
        {
            Q_OBJECT

          public:
            explicit WidgetPropertiesPaneFolder(QWidget* parent = nullptr);
            ~WidgetPropertiesPaneFolder();
            WidgetPropertiesPaneFolder(const WidgetPropertiesPaneFolder& e) = delete;
            WidgetPropertiesPaneFolder& operator=(const WidgetPropertiesPaneFolder&) = delete;
            WidgetPropertiesPaneFolder(WidgetPropertiesPaneFolder&&) = delete;
            WidgetPropertiesPaneFolder& operator=(WidgetPropertiesPaneFolder&&) = delete;

            void reset(const QMap<QString, QVariant>& props);

          private:
            Ui::WidgetPropertiesPaneFolder* ui;
            uint64_t mSourceID{0};
            uint64_t mFolderID{0};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_WIDGETPROPERTIESPANEFOLDER_H
