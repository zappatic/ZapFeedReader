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

#ifndef ZAPFR_ENGINE_AGENTSOURCEIMPORTOPML_H
#define ZAPFR_ENGINE_AGENTSOURCEIMPORTOPML_H

#include "ZapFR/AgentRunnable.h"
#include "ZapFR/Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class Feed;
        class Folder;
        class Source;

        class AgentSourceImportOPML : public AgentRunnable
        {
          public:
            explicit AgentSourceImportOPML(uint64_t sourceID, const std::string& opml, uint64_t parentFolderID, std::function<void()> opmlParsedCallback,
                                           std::function<void(uint64_t, Feed*)> feedRefreshedCallback);
            virtual ~AgentSourceImportOPML() = default;

            void payload(Source* source) override;
            Type type() const noexcept override { return Type::SourceImportOPML; }

          private:
            std::string mOPML{""};
            uint64_t mParentFolderID{0};
            std::function<void()> mOPMLParsedCallback{};
            std::function<void(uint64_t, Feed*)> mFeedRefreshedCallback{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_AGENTSOURCEIMPORTOPML_H