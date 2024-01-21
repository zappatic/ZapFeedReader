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

#include <optional>

#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/ErrorHandler.h>
#include <Poco/SAX/SAXException.h>

namespace ZapFR
{
    namespace Engine
    {
        class FavIconParser
        {
          public:
            FavIconParser() = default;
            virtual ~FavIconParser() = default;

            void parseURL(const std::string& url, uint64_t associatedFeedID);
            void parseString(const std::string& html, const std::optional<std::string>& originalURL);
            std::string favIcon() const noexcept;

          private:
            std::string mURL{""};
            std::string mFavIcon{""};
        };

        class FavIconSaxParser : public Poco::XML::ContentHandler
        {
          public:
            std::string favIconURL() const noexcept { return mFavIconURL; }
            bool linkIconTagEncountered() const noexcept { return mLinkIconTagEncountered; }

            void setDocumentLocator(const Poco::XML::Locator* loc) override { mLocator = loc; };
            void startDocument() override{};
            void endDocument() override{};
            void startElement(const Poco::XML::XMLString& uri, const Poco::XML::XMLString& localName, const Poco::XML::XMLString& qname,
                              const Poco::XML::Attributes& attrList) override;
            void endElement(const Poco::XML::XMLString& /*uri*/, const Poco::XML::XMLString& /*localName*/, const Poco::XML::XMLString& /*qname*/) override {}
            void characters(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/) override {}
            // LCOV_EXCL_START
            void ignorableWhitespace(const Poco::XML::XMLChar /*ch*/[], int /*start*/, int /*length*/) override {}
            void processingInstruction(const Poco::XML::XMLString& /*target*/, const Poco::XML::XMLString& /*data*/) override {}
            void startPrefixMapping(const Poco::XML::XMLString& /*prefix*/, const Poco::XML::XMLString& /*uri*/) override {}
            void endPrefixMapping(const Poco::XML::XMLString& /*prefix*/) override {}
            void skippedEntity(const Poco::XML::XMLString& /*name*/) override {}
            // LCOV_EXCL_STOP

          private:
            const Poco::XML::Locator* mLocator{nullptr};
            std::string mFavIconURL{""};
            bool mLinkIconTagEncountered{false};
        };

        class FavIconSaxErrorHandler : public Poco::XML::ErrorHandler
        {
          public:
            void error(const Poco::XML::SAXException& /*exc*/) override {} // ignore recoverable errors
            // LCOV_EXCL_START
            void fatalError(const Poco::XML::SAXException& exc) override { throw(exc); }
            void warning(const Poco::XML::SAXException& /*exc*/) override {} // ignore warnings
            // LCOV_EXCL_STOP
        };

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FAVICONPARSER_H