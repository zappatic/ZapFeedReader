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

#ifndef ZAPFR_ENGINE_FEEDPARSERXML_H
#define ZAPFR_ENGINE_FEEDPARSERXML_H

#include "FeedParser.h"

namespace ZapFR
{
    namespace Engine
    {
        class FeedParserXML : public FeedParser
        {
          public:
            FeedParserXML(const std::string& url);
            virtual ~FeedParserXML() = default;
            FeedParserXML(const FeedParserXML& e) = delete;
            FeedParserXML& operator=(const FeedParserXML&) = delete;
            FeedParserXML(FeedParserXML&&) = delete;
            FeedParserXML& operator=(FeedParserXML&&) = delete;

            void setXMLDoc(Poco::AutoPtr<Poco::XML::Document> xmlDoc) { mXMLDoc = xmlDoc; }

          protected:
            std::string fetchNodeValue(const std::string& nodeName) const;
            std::string fetchNodeValue(Poco::XML::Node* parent, const std::string& nodeName) const;
            std::string fetchNodeValueNS(Poco::XML::Node* parent, const std::string& nodeName, const Poco::XML::Node::NSMap& nsMap) const;
            std::string fetchNodeValueInnerXML(Poco::XML::Node* parent, const std::string& nodeName) const;
            Poco::XML::Node* fetchNode(Poco::XML::Node* parent, const std::string& nodeName) const;

            Poco::AutoPtr<Poco::XML::Document> mXMLDoc{nullptr};
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDPARSERATOM10_H