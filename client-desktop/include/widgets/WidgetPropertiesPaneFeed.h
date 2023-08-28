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

#ifndef ZAPFR_CLIENT_WIDGETPROPERTIESPANEFEED_H
#define ZAPFR_CLIENT_WIDGETPROPERTIESPANEFEED_H

#include "ClientGlobal.h"

namespace Ui
{
    class WidgetPropertiesPaneFeed;
}
namespace ZapFR
{
    namespace Client
    {
        class WidgetPropertiesPaneFeed : public QWidget
        {
            Q_OBJECT

          public:
            explicit WidgetPropertiesPaneFeed(QWidget* parent = nullptr);
            ~WidgetPropertiesPaneFeed();
            WidgetPropertiesPaneFeed(const WidgetPropertiesPaneFeed& e) = delete;
            WidgetPropertiesPaneFeed& operator=(const WidgetPropertiesPaneFeed&) = delete;
            WidgetPropertiesPaneFeed(WidgetPropertiesPaneFeed&&) = delete;
            WidgetPropertiesPaneFeed& operator=(WidgetPropertiesPaneFeed&&) = delete;

            void reset(const QMap<QString, QVariant>& props);

          private slots:
            void save();

          private:
            Ui::WidgetPropertiesPaneFeed* ui;
            uint64_t mSourceID{0};
            uint64_t mFeedID{0};
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_WIDGETPROPERTIESPANEFEED_H
