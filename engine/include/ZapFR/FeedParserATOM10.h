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

#ifndef ZAPFR_ENGINE_FEEDPARSERATOM10_H
#define ZAPFR_ENGINE_FEEDPARSERATOM10_H

#include "FeedParser.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedParserATOM10 : public FeedParser
        {
          public:
            FeedParserATOM10(Poco::XML::Document* xmlDoc, const std::string& url);
            virtual ~FeedParserATOM10() = default;

            std::string guid() const override;
            std::string title() const override;
            std::string subtitle() const override;
            std::string link() const override;
            std::string description() const override;
            std::string language() const override;
            std::string copyright() const override;
            std::string iconURL() const override;

            std::vector<Item> items() const override;
            void postProcessItem(Item& item) const override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDPARSERATOM10_H