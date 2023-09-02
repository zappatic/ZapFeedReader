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
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_about);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Adds a feed)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/feed$)", R"(/feed)");
				entry->addBodyParameter({R"(url)", true, R"(The url of the feed to add)"});
				entry->addBodyParameter({R"(folder)", true, R"(The ID of the folder in which to add the new feed)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_add);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Retrieves a feed)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/feed/([0-9]+)$)", R"(/feed/<feedID>)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to retrieve)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_get);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Removes a feed)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/feed/([0-9]+)$)", R"(/feed/<feedID>)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to delete)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_remove);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Returns all the feeds within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/feeds$)", R"(/feeds)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_feeds_list);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Adds a folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/folder$)", R"(/folder)");
				entry->addBodyParameter({R"(title)", true, R"(The title of the folder to add)"});
				entry->addBodyParameter({R"(parentFolderID)", true, R"(The ID of the folder under which to add the new subfolder)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_add);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Removes a folder)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/folder/([0-9]+)$)", R"(/folder/<folderID>)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to delete)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_remove);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Returns all the folders within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/folders$)", R"(/folders)");
				entry->addBodyParameter({R"(parentFolderID)", false, R"(The ID of the folder for which to retrieve the subfolders; optional, defaults to root(0))"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_folders_list);
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

