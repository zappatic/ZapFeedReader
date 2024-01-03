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

#define FMT_HEADER_ONLY
#include <fmt/core.h>

#include "ZapFR/Helpers.h"
#include "ZapFR/feed_handling/FeedParserJSON11.h"

ZapFR::Engine::FeedParserJSON11::FeedParserJSON11(const std::string& url) : FeedParser(url)
{
}

std::string ZapFR::Engine::FeedParserJSON11::guid() const
{
    if (mRootObj->has("feed_url"))
    {
        return mRootObj->getValue<std::string>("feed_url");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::title() const
{
    return mRootObj->getValue<std::string>("title");
}

std::string ZapFR::Engine::FeedParserJSON11::subtitle() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::link() const
{
    if (mRootObj->has("home_page_url"))
    {
        return mRootObj->getValue<std::string>("home_page_url");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::description() const
{
    if (mRootObj->has("description"))
    {
        return mRootObj->getValue<std::string>("description");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::language() const
{
    if (mRootObj->has("language"))
    {
        return mRootObj->getValue<std::string>("language");
    }
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::copyright() const
{
    return "";
}

std::string ZapFR::Engine::FeedParserJSON11::iconURL() const
{
    if (mRootObj->has("icon"))
    {
        return mRootObj->getValue<std::string>("icon");
    }
    else if (mRootObj->has("favicon"))
    {
        return mRootObj->getValue<std::string>("favicon");
    }
    return "";
}

std::vector<ZapFR::Engine::FeedParser::Item> ZapFR::Engine::FeedParserJSON11::items() const
{
    std::string topLevelAuthors;
    if (mRootObj->has("authors"))
    {
        topLevelAuthors = getAuthors(mRootObj->getArray("authors"));
    }
    if (topLevelAuthors.empty() && mRootObj->has("author"))
    {
        auto authorObj = mRootObj->getObject("author");
        if (authorObj->has("name"))
        {
            topLevelAuthors = authorObj->getValue<std::string>("name");
        }
    }

    std::vector<Item> items;

    auto itemList = mRootObj->getArray("items");
    for (size_t i = 0; i < itemList->size(); ++i)
    {
        auto itemObj = itemList->getObject(static_cast<uint32_t>(i));
        if (!itemObj->has("id"))
        {
            continue;
        }

        Item item;

        item.guid = itemObj->getValue<std::string>("id");

        if (itemObj->has("url"))
        {
            item.link = itemObj->getValue<std::string>("url");
        }

        if (itemObj->has("title"))
        {
            item.title = itemObj->getValue<std::string>("title");
        }

        if (itemObj->has("content_html"))
        {
            item.content = itemObj->getValue<std::string>("content_html");
        }
        else
        {
            auto text = itemObj->getValue<std::string>("content_text");

            // the spec is very clear about only allowing html in the content_html field, yet some people put html in there
            // seeing as this screws up the layout, force it to show the html as plain text by replacing < and >
            Poco::replaceInPlace(text, "<", "&lt;");
            Poco::replaceInPlace(text, ">", "&gt;");

            Poco::replaceInPlace(text, "\n", "<br />");
            item.content = fmt::format(R"(<pre style="white-space:pre-wrap;">{}</pre>)", text);
        }

        if (itemObj->has("image"))
        {
            item.thumbnail = itemObj->getValue<std::string>("image");
        }

        std::string providedDate;
        if (itemObj->has("date_modified"))
        {
            providedDate = itemObj->getValue<std::string>("date_modified");
        }
        else if (itemObj->has("date_published"))
        {
            providedDate = itemObj->getValue<std::string>("date_published");
        }
        if (!providedDate.empty())
        {
            Poco::DateTime parsedDate;
            int tzDiff;
            if (Poco::DateTimeParser::tryParse(Poco::DateTimeFormat::ISO8601_FORMAT, providedDate, parsedDate, tzDiff))
            {
                parsedDate.makeUTC(tzDiff);
                item.datePublished = Poco::DateTimeFormatter::format(parsedDate, Poco::DateTimeFormat::ISO8601_FORMAT);
            }
        }

        if (itemObj->has("authors"))
        {
            item.author = getAuthors(itemObj->getArray("authors"));
        }
        else if (itemObj->has("author"))
        {
            auto authorObj = itemObj->getObject("author");
            if (authorObj->has("name"))
            {
                item.author = authorObj->getValue<std::string>("name");
            }
        }
        else
        {
            item.author = topLevelAuthors;
        }

        if (itemObj->has("tags"))
        {
            auto tagArr = itemObj->getArray("tags");
            for (size_t j = 0; j < tagArr->size(); ++j)
            {
                item.categories.emplace_back(tagArr->getElement<std::string>(static_cast<uint32_t>(j)));
            }
        }

        items.emplace_back(item);
    }

    return items;
}

std::string ZapFR::Engine::FeedParserJSON11::getAuthors(Poco::JSON::Array::Ptr authorsArr) const
{
    std::vector<std::string> authors;
    for (size_t i = 0; i < authorsArr->size(); ++i)
    {
        auto author = authorsArr->getObject(static_cast<uint32_t>(i));
        if (author->has("name"))
        {
            authors.emplace_back(author->getValue<std::string>("name"));
        }
    }
    if (authors.size() > 0)
    {
        return Helpers::joinString(authors, ", ");
    }
    return "";
}
