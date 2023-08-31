/* **************************************************************************** */
/* **** THIS FILE IS AUTO GENERATED                                        **** */
/* **** DO NOT MODIFY THIS FILE, ANY CHANGES WILL BE UNDONE UPON RECOMPILE **** */
/* **** USE APIRequestHandlerRegistration.cpp.tpl INSTEAD                  **** */
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

#include "API.h"
#include "APIHandlers.h"

std::vector<std::unique_ptr<ZapFR::Server::API>> ZapFR::Server::API::msAPIs = std::vector<std::unique_ptr<API>>();

        bool ZapFR::Server::API::msAPIsLoaded = false;

        void ZapFR::Server::API::initializeAPIs(Daemon* daemon)
        {
        if (!msAPIsLoaded)
        {
        		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(UI)", R"(Returns general info about the server)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/about$)", R"(/about)");
				entry->setRequiresCredentials(false);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(object)");
				entry->setHandler(ZapFR::Server::APIHandler_about);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(UI)", R"(Returns the index page)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/$)", R"(/)");
				entry->setRequiresCredentials(false);
				entry->setContentType(R"(text/html)");
				entry->setHandler(ZapFR::Server::APIHandler_index);
				msAPIs.emplace_back(std::move(entry));
			}

        msAPIsLoaded = true;
        }
}

