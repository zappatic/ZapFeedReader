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

#include "APIRequestHandler.h"
#include "API.h"
#include "APIRequest.h"

ZapFR::Server::APIRequestHandler::APIRequestHandler(API* api) : mAPI(api)
{
}

void ZapFR::Server::APIRequestHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response)
{
    std::string exceptionMessage;
    auto httpStatus = Poco::Net::HTTPResponse::HTTP_OK;

    try
    {
        auto apiRequest = std::make_unique<APIRequest>(mAPI, request);
        response.setContentType(mAPI->contentType());
        response.setChunkedTransferEncoding(true);

        auto handler = mAPI->handler();
        httpStatus = handler(apiRequest.get(), response);
    }
    catch (const APIError& e)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_OK;
        auto jsonErrObj = Poco::JSON::Object();
        jsonErrObj.set("success", false);
        jsonErrObj.set("error", e.what());
        Poco::JSON::Stringifier::stringify(jsonErrObj, response.send());
        std::cerr << e.what() << "\n";
    }
    catch (const UnauthorizedError& e)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED;
        exceptionMessage = e.what();
    }
    catch (const FourOhFourError& e)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_NOT_FOUND;
        exceptionMessage = e.what();
    }
    catch (const Poco::Exception& e)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;
        exceptionMessage = e.displayText();
    }
    catch (const std::exception& e)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;
        exceptionMessage = e.what();
    }
    catch (...)
    {
        httpStatus = Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR;
    }

    response.setStatus(httpStatus);
    if (httpStatus != Poco::Net::HTTPResponse::HTTP_OK)
    {
        response.setContentType("application/json");
        auto jsonErrObj = Poco::JSON::Object();
        jsonErrObj.set("error", exceptionMessage);
        std::cerr << exceptionMessage << "\n";
        Poco::JSON::Stringifier::stringify(jsonErrObj, response.send());
    }
}