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

#ifndef ZAPFR_ENGINE_AUTOREFRESH_H
#define ZAPFR_ENGINE_AUTOREFRESH_H

#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;

        class AutoRefresh
        {
          public:
            AutoRefresh(const AutoRefresh&) = delete;
            AutoRefresh& operator=(const AutoRefresh&) = delete;
            virtual ~AutoRefresh() = default;

            static AutoRefresh* getInstance();

            void setEnabled(bool b) noexcept { mEnabled = b; }
            bool isEnabled() const noexcept { return mEnabled; }

            void setFeedRefreshInterval(uint64_t seconds) noexcept { mFeedRefreshIntervalInSeconds = seconds; }
            uint64_t feedRefreshInterval() const noexcept { return mFeedRefreshIntervalInSeconds; }

            void setFeedRefreshedCallback(const std::function<void(uint64_t, Feed*)>& callback) noexcept { mFeedRefreshedCallback = callback; }

          private:
            explicit AutoRefresh();

            bool mEnabled{true};
            uint64_t mFeedRefreshIntervalInSeconds{DefaultFeedAutoRefreshInterval};
            std::unique_ptr<Poco::Timer> mTimer{nullptr};
            void onTimer(Poco::Timer& timer);
            std::optional<std::function<void(uint64_t, Feed*)>> mFeedRefreshedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AUTOREFRESH_H