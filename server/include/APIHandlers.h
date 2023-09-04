/* **************************************************************************** */
/* **** THIS FILE IS AUTO GENERATED                                        **** */
/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */
/* **** USE APIHandlers.h.tpl INSTEAD                                      **** */
/* **************************************************************************** */



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

#ifndef ZAPFR_SERVER_APIHANDLERS_H
#define ZAPFR_SERVER_APIHANDLERS_H

#include "ServerGlobal.h"

namespace ZapFR
{
    namespace Server
    {
        class APIRequest;

		Poco::Net::HTTPResponse::HTTPStatus APIHandler_about(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feed_add(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feed_get(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feed_markasread(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feed_move(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feed_remove(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_feeds_list(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folder_add(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folder_get(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folder_markasread(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folder_move(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folder_remove(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_folders_list(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_index(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_logs_list(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_posts_list(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_source_markasread(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_source_setpostsflagstatus(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_source_setpostsreadstatus(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_source_statistics(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);
		Poco::Net::HTTPResponse::HTTPStatus APIHandler_source_usedflagcolors(APIRequest* apiRequest, Poco::Net::HTTPServerResponse& response);

    } // namespace Server
} // namespace ZapFR

#endif // ZAPFR_SERVER_APIHANDLERS_H


