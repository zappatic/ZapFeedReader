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

#include <Poco/File.h>
#include <Poco/FileStream.h>
#include <Poco/StreamCopier.h>

#include "API.h"
#include "APIHandlers.h"
#include "APIRequest.h"
#include "Daemon.h"

// ::API
//
//	Returns the /web/img/ folder of the webinterface
//	/web/img/* (GET)
//
//	Content-Type: image/*
//
// API::

namespace
{
    struct ExtensionInfo
    {
        std::string_view extension;
        std::string_view mimeType;
    };

    constexpr std::array validExtensions{ExtensionInfo{"jpg", "image/jpeg"}, ExtensionInfo{"jpeg", "image/jpeg"}, ExtensionInfo{"png", "image/png"},
                                         ExtensionInfo{"svg", "image/svg+xml"}};

    std::optional<std::string_view> getMimeType(const Poco::Path& path)
    {
        auto ext = path.getExtension();
        std::ranges::transform(ext, ext.begin(), ::tolower);

        const auto it = std::ranges::find(validExtensions, ext, &ExtensionInfo::extension);

        if (it == validExtensions.end())
            return std::nullopt;

        return it->mimeType;
    }

} // namespace

Poco::Net::HTTPResponse::HTTPStatus ZapFR::Server::APIHandler_webinterface_img(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response)
{

    auto imgFileName = apiRequest->pathComponentAt(1);
    Poco::trimInPlace(imgFileName);
    if (imgFileName.empty())
    {
        throw std::runtime_error("No image file specified");
    }

    auto daemon = apiRequest->api()->daemon();
    auto imgFile = daemon->webinterfaceRoot().pushDirectory("img").setFileName(imgFileName);

    if (const auto mime = getMimeType(imgFile))
    {
        response.setContentType(std::string{*mime});
    }
    else
    {
        throw std::runtime_error(fmt::format("Invalid image file requested ('{}')", imgFileName));
    }

    if (!Poco::File(imgFile.toString()).exists())
    {
        throw std::runtime_error(fmt::format("Requested image file '{}' does not exist", imgFileName));
    }

    Poco::FileInputStream fileStream(imgFile.toString());
    Poco::StreamCopier::copyStream(fileStream, response.send());

    return Poco::Net::HTTPResponse::HTTP_OK;
}