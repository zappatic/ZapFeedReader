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

#ifndef ZAPFR_CLIENT_FRAMEFLAGFILTERS_H
#define ZAPFR_CLIENT_FRAMEFLAGFILTERS_H

#include <QFrame>

#include "ZapFR/Flag.h"

namespace ZapFR
{
    namespace Client
    {
        class MainWindow;

        class FrameFlagFilters : public QFrame
        {
            Q_OBJECT

          public:
            explicit FrameFlagFilters(QWidget* parent = nullptr);
            ~FrameFlagFilters() = default;
            FrameFlagFilters(const FrameFlagFilters& e) = delete;
            FrameFlagFilters& operator=(const FrameFlagFilters&) = delete;
            FrameFlagFilters(FrameFlagFilters&&) = delete;
            FrameFlagFilters& operator=(FrameFlagFilters&&) = delete;

            void setMainWindow(MainWindow* mw) noexcept;

            void reload(bool forceReload = false);

          private:
            MainWindow* mMainWindow{nullptr};

            void populateUsedFlags(uint64_t sourceID, const std::unordered_set<ZapFR::Engine::FlagColor>& flagColors);
        };
    } // namespace Client
} // namespace ZapFR

#endif // ZAPFR_CLIENT_FRAMEFLAGFILTERS_H
