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

#include "APIRequest404Handler.h"

void ZapFR::Server::APIRequest404Handler::handleRequest(Poco::Net::HTTPServerRequest& /*request*/, Poco::Net::HTTPServerResponse& response)
{
    response.set("Access-Control-Allow-Origin", "*");

    response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
    response.setContentType("application/json");
    auto jsonErrObj = Poco::JSON::Object();
    jsonErrObj.set("success", false);
    jsonErrObj.set("error", "404 - not found");
    jsonErrObj.set("message", "404 - not found");
    Poco::JSON::Stringifier::stringify(jsonErrObj, response.send());
}