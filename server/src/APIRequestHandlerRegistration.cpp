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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Dummy feed)", R"(Adds a dummy post to the dummy feed)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/dummy-feed/add-post$)", R"(/dummy-feed/add-post)");
				entry->setRequiresCredentials(false);
				entry->setContentType(R"(text/plain)");
				entry->setHandler(ZapFR::Server::APIHandler_dummyfeed_createpost);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Dummy feed)", R"(Retrieves the RSS 2.0 dummy feed)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/dummy-feed/rss20$)", R"(/dummy-feed/rss20)");
				entry->setRequiresCredentials(false);
				entry->setContentType(R"(application/rss+xml)");
				entry->setHandler(ZapFR::Server::APIHandler_dummyfeed_get_rss20);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Adds a feed)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/feed$)", R"(/feed)");
				entry->addBodyParameter({R"(url)", true, R"(The url of the feed to add)"});
				entry->addBodyParameter({R"(parentFolderID)", true, R"(The ID of the folder in which to add the new feed)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_add);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Removes all the logs for this feed)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/feed/([0-9]+)/logs$)", R"(/feed/<feedID>/logs)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to remove the logs for)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_deletelogs);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Retrieves a feed)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/feed/([0-9]+)$)", R"(/feed/<feedID>)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to retrieve)"});
				entry->addBodyParameter({R"(getData)", false, R"(Whether to fetch the full feed data from the database ('true' or 'false'; default false))"});
				entry->addBodyParameter({R"(getStatistics)", false, R"(Whether to fetch the statistics of the feed ('true' or 'false'; default false))"});
				entry->addBodyParameter({R"(getUnreadCount)", false, R"(Whether to fetch the unread count of the feed ('true' or 'false'; default false))"});
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
				entry->addBodyParameter({R"(maxPostID)", true, R"(The highest post ID to mark as read)"});
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
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_move);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Manually refreshes the feed)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/feed/([0-9]+)/refresh$)", R"(/feed/<feedID>/refresh)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to refresh)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_refresh);
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Updates the properties of a feed)");
				entry->setMethod("PATCH");
				entry->setPath(R"(^\/feed/([0-9]+)$)", R"(/feed/<feedID>)");
				entry->addURIParameter({R"(feedID)", R"(The id of the feed to update)"});
				entry->addBodyParameter({R"(url)", true, R"(The new url of the feed)"});
				entry->addBodyParameter({R"(refreshInterval)", false, R"(The new refresh interval of the feed in seconds (optional, uses global default if not specified))"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_feed_update);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Feeds)", R"(Returns all the feeds within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/feeds$)", R"(/feeds)");
				entry->addBodyParameter({R"(getIcons)", false, R"(Whether to include the base64 encoded icon or not ('true' or 'false') (optional; default=false))"});
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Removes all the logs for the feeds in this folder)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/folder/([0-9]+)/logs$)", R"(/folder/<folderID>/logs)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to remove the logs for)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_deletelogs);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Retrieves a folder (subfolders are not populated))");
				entry->setMethod("GET");
				entry->setPath(R"(^\/folder/([0-9]+)$)", R"(/folder/<folderID>)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to retrieve)"});
				entry->addBodyParameter({R"(getStatistics)", false, R"(Whether to fetch the statistics of the folder ('true' or 'false'; default false))"});
				entry->addBodyParameter({R"(getFeedIDs)", false, R"(Whether to fetch the feed IDs of the folder ('true' or 'false'; default false))"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_get);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Marks all posts in the folder as read, returns the affected feedIDs)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/folder/([0-9]+)/mark-as-read$)", R"(/folder/<folderID>/mark-as-read)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to mark as read)"});
				entry->addBodyParameter({R"(maxPostID)", true, R"(The highest post ID to mark as read)"});
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
				entry->setJSONOutput(R"(Array)");
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Sorts all the subfolders and feeds in a given folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/folder/([0-9]+)/sort$)", R"(/folder/<folderID>/sort)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to sort)"});
				entry->addBodyParameter({R"(sortMethod)", false, R"(The method of sorting. Currently supported: 'alphaAsc' (default))"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_sort);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Updates the properties of a folder)");
				entry->setMethod("PATCH");
				entry->setPath(R"(^\/folder/([0-9]+)$)", R"(/folder/<folderID>)");
				entry->addURIParameter({R"(folderID)", R"(The id of the folder to update)"});
				entry->addBodyParameter({R"(newTitle)", true, R"(The new title of the folder)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_folder_update);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Folders)", R"(Returns all the folders within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/folders$)", R"(/folders)");
				entry->addBodyParameter({R"(parentFolderID)", false, R"(The ID of the folder for which to retrieve the subfolders; optional, defaults to root(0))"});
				entry->addBodyParameter({R"(getSubfolders)", false, R"(Whether to recursively retrieve all of the subfolders ('true' or 'false'; default false))"});
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Posts)", R"(Retrieves a post from a specific feed)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/post/([0-9]+)$)", R"(/post/<postID>)");
				entry->addURIParameter({R"(postID)", R"(The id of the post to retrieve)"});
				entry->addBodyParameter({R"(feedID)", true, R"(The ID of the feed the post belongs to)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_post_get);
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
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scripts)", R"(Adds a script)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/script$)", R"(/script)");
				entry->addBodyParameter({R"(type)", true, R"(The type of the script ('lua'))"});
				entry->addBodyParameter({R"(title)", true, R"(The title of the script)"});
				entry->addBodyParameter({R"(isEnabled)", true, R"(Whether the script is enabled or not ('true' or 'false'))"});
				entry->addBodyParameter({R"(runOnEvents)", false, R"(A comma separated list of events the script should run on)"});
				entry->addBodyParameter({R"(runOnFeedIDs)", false, R"(A comma separated list of feedIDs the script should run for)"});
				entry->addBodyParameter({R"(script)", false, R"(The script contents)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_script_add);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scripts)", R"(Retrieves a script)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/script/([0-9]+)$)", R"(/script/<scriptID>)");
				entry->addURIParameter({R"(scriptID)", R"(The id of the script to retrieve)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_script_get);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scripts)", R"(Removes a script)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/script/([0-9]+)$)", R"(/script/<scriptID>)");
				entry->addURIParameter({R"(scriptID)", R"(The id of the script to delete)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_script_remove);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scripts)", R"(Updates the properties of a script)");
				entry->setMethod("PATCH");
				entry->setPath(R"(^\/script/([0-9]+)$)", R"(/script/<scriptID>)");
				entry->addURIParameter({R"(scriptID)", R"(The id of the script to update)"});
				entry->addBodyParameter({R"(type)", true, R"(The type of the script ('lua'))"});
				entry->addBodyParameter({R"(title)", true, R"(The title of the script)"});
				entry->addBodyParameter({R"(isEnabled)", true, R"(Whether the script is enabled or not ('true' or 'false'))"});
				entry->addBodyParameter({R"(runOnEvents)", false, R"(A comma separated list of events the script should run on)"});
				entry->addBodyParameter({R"(runOnFeedIDs)", false, R"(A comma separated list of feedIDs the script should run for)"});
				entry->addBodyParameter({R"(script)", false, R"(The script contents)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_script_update);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Adds a script folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/scriptfolder$)", R"(/scriptfolder)");
				entry->addBodyParameter({R"(title)", true, R"(The title of the script folder to add)"});
				entry->addBodyParameter({R"(showTotal)", true, R"(Whether to show the total number of posts)"});
				entry->addBodyParameter({R"(showUnread)", true, R"(Whether to show the unread number of posts)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_add);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Assigns posts to a script folder)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/scriptfolder/([0-9]+)/assign-posts$)", R"(/scriptfolder/<scriptFolderID>/assign-posts)");
				entry->addURIParameter({R"(scriptFolderID)", R"(The id of the script folder to assign the posts to)"});
				entry->addBodyParameter({R"(feedsAndPostIDs)", true, R"(Stringified json array: [ {feedID: x, postID: x}, {...}, ...])"});
				entry->addBodyParameter({R"(assign)", true, R"(Whether to assign or unassign the posts ('true' or 'false'))"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_assignposts);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Retrieves a script folder)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/scriptfolder/([0-9]+)$)", R"(/scriptfolder/<scriptFolderID>)");
				entry->addURIParameter({R"(scriptFolderID)", R"(The id of the script folder to retrieve)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_get);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Marks all posts in the script folder as read, returns the affected feedIDs)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/scriptfolder/([0-9]+)/mark-as-read$)", R"(/scriptfolder/<scriptFolderID>/mark-as-read)");
				entry->addURIParameter({R"(scriptFolderID)", R"(The id of the script folder to mark as read)"});
				entry->addBodyParameter({R"(maxPostID)", true, R"(The highest post ID to mark as read)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_markasread);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Removes a script folder)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/scriptfolder/([0-9]+)$)", R"(/scriptfolder/<scriptFolderID>)");
				entry->addURIParameter({R"(scriptFolderID)", R"(The id of the script folder to delete)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_remove);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Updates the properties of a script folder)");
				entry->setMethod("PATCH");
				entry->setPath(R"(^\/scriptfolder/([0-9]+)$)", R"(/scriptfolder/<scriptFolderID>)");
				entry->addURIParameter({R"(scriptFolderID)", R"(The id of the script folder to update)"});
				entry->addBodyParameter({R"(title)", true, R"(The new title of the script folder)"});
				entry->addBodyParameter({R"(showTotal)", true, R"(Whether to show the total number of posts)"});
				entry->addBodyParameter({R"(showUnread)", true, R"(Whether to show the unread number of posts)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolder_update);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scriptfolders)", R"(Returns all the script folders within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/scriptfolders$)", R"(/scriptfolders)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_scriptfolders_list);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Scripts)", R"(Returns all the scripts within the source)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/scripts$)", R"(/scripts)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_scripts_list);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Removes all the logs in the source)");
				entry->setMethod("DELETE");
				entry->setPath(R"(^\/logs$)", R"(/logs)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_deletelogs);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Retrieves the status of the server, containing data to properly synchronize the client)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/status$)", R"(/status)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_getstatus);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Import an OPML file)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/import-opml$)", R"(/import-opml)");
				entry->addBodyParameter({R"(opml)", true, R"(The OPML XML content to parse and extract feeds from)"});
				entry->addBodyParameter({R"(parentFolderID)", true, R"(The folder ID under which to import the feeds in the OPML)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_source_importopml);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Marks all posts in the source as read)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/mark-as-read$)", R"(/mark-as-read)");
				entry->addBodyParameter({R"(maxPostID)", true, R"(The highest post ID to mark as read)"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_markasread);
				msAPIs.emplace_back(std::move(entry));
			}

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Sets the flag status of posts in bulk)");
				entry->setMethod("POST");
				entry->setPath(R"(^\/set-posts-flag-status$)", R"(/set-posts-flag-status)");
				entry->addBodyParameter({R"(markFlagged)", true, R"(Whether to mark the posts as flagged or unflagged ('true' or 'false'))"});
				entry->addBodyParameter({R"(flagColors)", true, R"(Stringified json array of flag color names to apply)"});
				entry->addBodyParameter({R"(feedsAndPostIDs)", true, R"(Stringified json array: [ {feedID: x, postID: x}, {...}, ...])"});
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Object)");
				entry->setHandler(ZapFR::Server::APIHandler_source_setpostsflagstatus);
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

		{
				auto entry = std::make_unique<ZapFR::Server::API>(daemon, R"(Sources)", R"(Retrieves all the flag colors in use)");
				entry->setMethod("GET");
				entry->setPath(R"(^\/used-flag-colors$)", R"(/used-flag-colors)");
				entry->setRequiresCredentials(true);
				entry->setContentType(R"(application/json)");
				entry->setJSONOutput(R"(Array)");
				entry->setHandler(ZapFR::Server::APIHandler_source_usedflagcolors);
				msAPIs.emplace_back(std::move(entry));
			}

        msAPIsLoaded = true;
        }
}

