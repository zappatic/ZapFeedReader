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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Marks all posts in the feed as read)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/feed/([0-9]+)/mark-as-read$)", R"(/feed/<feedID>/mark-as-read)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to mark as read)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_markasread);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Moves a feed to a new subfolder and/or position within the folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/feed/([0-9]+)/move$)", R"(/feed/<feedID>/move)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to move)"});
				entry->addBodyParameter({R"(sortOrder)", true, R"(The new sort order of the feed (the position within the new parent folder))"});
				entry->addBodyParameter({R"(parentFolderID)", true, R"(The (new) folder parent to put the feed in)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_move);
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Retrieves a folder (subfolders are not populated))");
				entry->setMethod("GET");
				entry->setPath(R"(^\/folder/([0-9]+)$)", R"(/folder/<folderID>)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to retrieve)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_get);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(FeFolderseds)", R"(Marks all posts in the folder as read, returns the affected feedIDs)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/folder/([0-9]+)/mark-as-read$)", R"(/folder/<folderID>/mark-as-read)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to mark as read)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_markasread);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Moves a folder to a new subfolder and/or position within the parent folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/folder/([0-9]+)/move$)", R"(/folder/<folderID>/move)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to move)"});
				entry->addBodyParameter({R"(sortOrder)", true, R"(The new sort order of the folder (the position within the new parent folder))"});
				entry->addBodyParameter({R"(parentFolderID)", true, R"(The (new) folder parent to put the folder in)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_move);
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

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Logs)", R"(Returns all the logs belonging to a feed, folder or source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/logs$)", R"(/logs)");
				entry->addBodyParameter({R"(parentType)", true, R"(The type (source, folder, feed) to retrieve posts for)"});
				entry->addBodyParameter({R"(parentID)", false, R"(The ID of the parent type (feedID or folderID); n/a in case of 'source')"});
				entry->addBodyParameter({R"(perPage)", true, R"(The amount of records per page to retrieve)"});
				entry->addBodyParameter({R"(page)", true, R"(The page number to retrieve)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_logs_list);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Posts)", R"(Returns all the posts belonging to a feed, folder or source with various filters applied)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/posts$)", R"(/posts)");
				entry->addBodyParameter({R"(parentType)", true, R"(The type (source, folder, feed) to retrieve posts for)"});
				entry->addBodyParameter({R"(parentID)", false, R"(The ID of the parent type (feedID or folderID); n/a in case of 'source')"});
				entry->addBodyParameter({R"(perPage)", true, R"(The amount of records per page to retrieve)"});
				entry->addBodyParameter({R"(page)", true, R"(The page number to retrieve)"});
				entry->addBodyParameter({R"(showOnlyUnread)", false, R"(Whether to only retrieve unread posts - 'true' or 'false' - optional (default: false))"});
				entry->addBodyParameter({R"(searchFilter)", false, R"(An optional search filter to apply)"});
				entry->addBodyParameter({R"(flagColor)", false, R"(The ID of a flag color to apply as a filter)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_posts_list);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Marks all posts in the source as read)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/mark-as-read$)", R"(/mark-as-read)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_markasread);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Sets the read status of posts in bulk)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/set-posts-read-status$)", R"(/set-posts-read-status)");
				entry->addBodyParameter({R"(markAsRead)", true, R"(Whether to mark the posts as read or unread ('true' or 'false'))"});
				entry->addBodyParameter({R"(feedsAndPostIDs)", true, R"(Stringified json array: [ {feedID: x, postID: x}, {...}, ...])"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_setpostsreadstatus);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Retrieves statistics of a source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/statistics$)", R"(/statistics)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_statistics);
				msAPIs.emplace_back(std::move(entry));
			}

        msAPIsLoaded = true;
        }
}

