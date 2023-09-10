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

#ifndef ZAPFR_ENGINE_GLOBAL_H
#define ZAPFR_ENGINE_GLOBAL_H

#include <Poco/Base64Decoder.h>
#include <Poco/Base64Encoder.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/DOMWriter.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/NodeList.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Data/Session.h>
#include <Poco/DateTime.h>
#include <Poco/DateTimeParser.h>
#include <Poco/DigestStream.h>
#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/MD5Engine.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPCredentials.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Nullable.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <Poco/Runnable.h>
#include <Poco/SAX/Attributes.h>
#include <Poco/SAX/ContentHandler.h>
#include <Poco/SAX/ErrorHandler.h>
#include <Poco/SAX/SAXException.h>
#include <Poco/SAX/SAXParser.h>
#include <Poco/StreamCopier.h>
#include <Poco/StringTokenizer.h>
#include <Poco/ThreadPool.h>
#include <Poco/Timer.h>
#include <Poco/URI.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/SystemConfiguration.h>
#include <Poco/XML/XMLWriter.h>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#define FMT_HEADER_ONLY
#include <fmt/core.h>

namespace ZapFR
{
    namespace Engine
    {
        enum class ApplicationType
        {
            Server,
            Client
        };

        static const uint64_t APIVersion{1};
        static const uint16_t DefaultServerPort{16016};
        [[maybe_unused]] static const char* IdentifierLocalServer{"zapfeedreader.local"};
        [[maybe_unused]] static const char* IdentifierRemoteServer{"zapfeedreader.remote"};
    } // namespace Engine
} // namespace ZapFR

#endif // ZAPFR_ENGINE_GLOBAL_H