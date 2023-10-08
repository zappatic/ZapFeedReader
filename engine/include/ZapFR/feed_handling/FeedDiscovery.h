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

#ifndef ZAPFR_ENGINE_FEEDDISCOVERY_H
#define ZAPFR_ENGINE_FEEDDISCOVERY_H

#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/ErrorHandler.h>
#include <Poco/SAX/SAXException.h>
#include <Poco/URI.h>

#include "ZapFR/base/Feed.h"

namespace ZapFR
{
    namespace Engine
    {
        struct DiscoveredFeed
        {
            DiscoveredFeed() = default;
            DiscoveredFeed(const std::string& feedTitle, const std::string& feedURL, Feed::Type feedType) : title(feedTitle), url(feedURL), type(feedType) {}
            std::string title{""};
            std::string url{""};
            Feed::Type type{Feed::Type::RSS};
        };

        class FeedDiscovery
        {
          public:
            FeedDiscovery(const std::string& url);
            FeedDiscovery(const std::string& url, const std::string& data);
            virtual ~FeedDiscovery() = default;

            void discover();
            const std::vector<DiscoveredFeed>& discoveredFeeds() const noexcept { return mDiscoveredFeeds; }

          private:
            Poco::URI mURI{""};
            std::string mData{""};
            std::vector<DiscoveredFeed> mDiscoveredFeeds{};

            bool interpretAsYoutubeSource();
            bool interpretAsDirectFeedLink();
            bool interpretAsHTMLWithRelAlternateLinks();

            void postProcessFeeds();
        };

        class HTMLRelAlternateFeedExtractorSaxParser : public Poco::XML::ContentHandler
        {
          public:
            void setOutputVector(std::vector<DiscoveredFeed>* outputVector) noexcept { mOutputVector = outputVector; }

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
            std::vector<DiscoveredFeed>* mOutputVector{nullptr};
        };

        class DocumentElementExtractorSaxParser : public Poco::XML::ContentHandler
        {
          public:
            const std::string& documentElementTitle() const noexcept { return mDocumentElementTitle; }

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
            std::string mDocumentElementTitle{""};
            bool mFirstElementParsed{false};
        };

        class FeedDiscoverySaxErrorHandler : public Poco::XML::ErrorHandler
        {
          public:
            void error(const Poco::XML::SAXException& /*exc*/) override {} // ignore recoverable errors
            void fatalError(const Poco::XML::SAXException& exc) override { throw(exc); }
            void warning(const Poco::XML::SAXException& /*exc*/) override {} // ignore warnings
        };

    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_FEEDDISCOVERY_H