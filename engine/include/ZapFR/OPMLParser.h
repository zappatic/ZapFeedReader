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

#ifndef ZAPFR_ENGINE_OPMLPARSER_H
#define ZAPFR_ENGINE_OPMLPARSER_H

#include <Poco/SAX/ContentHandler.h>

namespace ZapFR
{
    namespace Engine
    {
        struct OPMLEntry
        {
            std::string title{""};
            std::string url{""};
            std::vector<std::string> folderHierarchy{};
        };

        class OPMLParser : public Poco::XML::ContentHandler
        {
          public:
            static std::vector<OPMLEntry> parse(const std::string& opmlXML);

            std::vector<OPMLEntry> entries() const;

          private:
            const Poco::XML::Locator* mLocator{nullptr};
            std::vector<OPMLEntry> mEntries{};
            std::vector<std::string> mCurrentFolderHierarchy{};
            bool mCurrentOutlineIsFeed{false};

            void setDocumentLocator(const Poco::XML::Locator* loc) override;
            void startDocument() override;
            void endDocument() override;
            void startElement(const Poco::XML::XMLString& uri, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& qname,
                              const Poco::XML::Attributes& attrList) override;
            void endElement(const Poco::XML::XMLString& uri, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& qname) override;
            void characters(const Poco::XML::XMLChar ch[], int start, int length) override;
            void ignorableWhitespace(const Poco::XML::XMLChar ch[], int start, int length) override;
            void processingInstruction(const Poco::XML::XMLString& target, const Poco::XML::XMLString& data) override;
            void startPrefixMapping(const Poco::XML::XMLString& prefix, const Poco::XML::XMLString& uri) override;
            void endPrefixMapping(const Poco::XML::XMLString& prefix) override;
            void skippedEntity(const Poco::XML::XMLString& name) override;
        };
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_OPMLPARSER_H