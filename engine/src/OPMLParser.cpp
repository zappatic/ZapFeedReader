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

#include "ZapFR/OPMLParser.h"
#include "ZapFR/Helpers.h"

std::vector<ZapFR::Engine::OPMLEntry> ZapFR::Engine::OPMLParser::parse(const std::string& opmlXML)
{

    OPMLParser handler;

    Poco::XML::SAXParser parser;
    parser.setContentHandler(&handler);
    parser.parseString(opmlXML);

    return handler.entries();
}

void ZapFR::Engine::OPMLParser::setDocumentLocator(const Poco::XML::Locator* loc)
{
    mLocator = loc;
}

void ZapFR::Engine::OPMLParser::startDocument()
{
    mEntries.clear();
    mCurrentFolderHierarchy.clear();
}

void ZapFR::Engine::OPMLParser::endDocument()
{
}

void ZapFR::Engine::OPMLParser::startElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& /*qname*/,
                                             const Poco::XML::Attributes& attrList)
{
    if (localName == "outline")
    {
        bool isRSS{false};
        std::string text = "Untitled feed";

        auto textIndex = attrList.getIndex("", "text");
        if (textIndex > -1)
        {
            text = attrList.getValue(textIndex);
        }

        // Even though the spec says to include type="rss", some OPML files don't have this
        // The rule to determine whether an outline is an rss feed is the fact that it has
        // a xmlUrl attribute that starts with 'http'

        auto xmlUrlIndex = attrList.getIndex("", "xmlUrl");
        if (xmlUrlIndex > -1)
        {
            auto xmlUrl = attrList.getValue(xmlUrlIndex);
            if (xmlUrl.starts_with("http"))
            {
                isRSS = true;
                mCurrentOutlineIsFeed = true;

                OPMLEntry entry;
                entry.title = text;
                entry.url = xmlUrl;
                entry.folderHierarchy = mCurrentFolderHierarchy;
                mEntries.emplace_back(entry);
            }
        }

        if (!isRSS)
        {
            // treat it as a subfolder
            mCurrentOutlineIsFeed = false;
            mCurrentFolderHierarchy.push_back(text);
        }
    }
}

void ZapFR::Engine::OPMLParser::endElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& /*qname*/)
{
    if (localName == "outline")
    {
        if (!mCurrentOutlineIsFeed && mCurrentFolderHierarchy.size() > 0)
        {
            mCurrentFolderHierarchy.pop_back();
        }
        else
        {
            mCurrentOutlineIsFeed = false;
        }
    }
}

void ZapFR::Engine::OPMLParser::characters(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/)
{
}

void ZapFR::Engine::OPMLParser::ignorableWhitespace(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/)
{
}

void ZapFR::Engine::OPMLParser::processingInstruction(const Poco::XML::XMLString& /*target*/, const Poco::XML::XMLString& /*data*/)
{
}

void ZapFR::Engine::OPMLParser::startPrefixMapping(const Poco::XML::XMLString& /*prefix*/, const Poco::XML::XMLString& /*uri*/)
{
}

void ZapFR::Engine::OPMLParser::endPrefixMapping(const Poco::XML::XMLString& /*prefix*/)
{
}

void ZapFR::Engine::OPMLParser::skippedEntity(const Poco::XML::XMLString& /*name*/)
{
}

std::vector<ZapFR::Engine::OPMLEntry> ZapFR::Engine::OPMLParser::entries() const
{
    return mEntries;
}
