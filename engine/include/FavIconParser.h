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

#ifndef ZAPFR_ENGINE_FAVICONPARSER_H
#define ZAPFR_ENGINE_FAVICONPARSER_H

#include "Global.h"

namespace ZapFR
{
    namespace Engine
    {
        class FavIconParser
        {
          public:
            FavIconParser(const std::string& url);
            virtual ~FavIconParser() = default;

            std::string favIcon() const noexcept;

          private:
            std::string mURL{""};
            std::string mFavIcon{""};
        };

        class FavIconSaxParser : public Poco::XML::ContentHandler
        {
          public:
            std::string favIconURL() const noexcept { return mFavIconURL; }

            void setDocumentLocator(const Poco::XML::Locator* loc) override { mLocator = loc; };
            void startDocument() override{};
            void endDocument() override{};
            void startElement(const Poco::XML::XMLString& uri, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& qname,
                              const Poco::XML::Attributes& attrList) override;
            void endElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& /*localName*/, const Poco::XML::XMLString& /*qname*/) override {}
            void characters(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/) override {}
            void ignorableWhitespace(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/) override {}
            void processingInstruction(const Poco::XML::XMLString& /*target*/, const Poco::XML::XMLString& /*data*/) override {}
            void startPrefixMapping(const Poco::XML::XMLString& /*prefix*/, const Poco::XML::XMLString& /*uri*/) override {}
            void endPrefixMapping(const Poco::XML::XMLString& /*prefix*/) override {}
            void skippedEntity(const Poco::XML::XMLString& /*name*/) override {}

          private:
            const Poco::XML::Locator* mLocator{nullptr};
            std::string mFavIconURL{""};
        };

        class FavIconSaxErrorHandler : public Poco::XML::ErrorHandler
        {
          public:
            void error(const Poco::XML::SAXException& /*exc*/) override {} // ignore recoverable errors
            void fatalError(const Poco::XML::SAXException& exc) override { throw(exc); }
            void warning(const Poco::XML::SAXException& /*exc*/) override {} // ignore warnings
        };

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FAVICONPARSER_H