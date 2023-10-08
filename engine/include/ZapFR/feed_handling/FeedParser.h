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

#ifndef ZAPFR_ENGINE_FEEDPARSER_H
#define ZAPFR_ENGINE_FEEDPARSER_H

#include <Poco/DOM/Document.h>
#include <Poco/URI.h>

#include "ZapFR/base/Feed.h"
#include "ZapFR/base/Post.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedParser
        {
          public:
            FeedParser(const std::string& url) : mURI(Poco::URI(url)) {}
            virtual ~FeedParser() = default;
            FeedParser(const FeedParser& e) = delete;
            FeedParser& operator=(const FeedParser&) = delete;
            FeedParser(FeedParser&&) = delete;
            FeedParser& operator=(FeedParser&&) = delete;

            virtual Feed::Type type() const noexcept = 0;

            virtual std::string guid() const = 0;
            virtual std::string title() const = 0;
            virtual std::string subtitle() const = 0;
            virtual std::string link() const = 0;
            virtual std::string description() const = 0;
            virtual std::string language() const = 0;
            virtual std::string copyright() const = 0;
            virtual std::string iconURL() const = 0;

            struct Item
            {
                std::string title{""};
                std::string link{""};
                std::string content{""};
                std::string author{""};
                std::string commentsURL{""};
                std::string guid{""};
                std::string datePublished{""};
                std::string thumbnail{""};
                std::vector<Post::Enclosure> enclosures{};
                std::vector<std::string> categories{};
            };

            virtual std::vector<Item> items() const = 0;

          protected:
            Poco::URI mURI{};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDPARSER_H