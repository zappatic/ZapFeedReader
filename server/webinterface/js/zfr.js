class ZapFeedReader {
  sidebar = null;
  posts = null;
  postsTable = null;
  postsNavigation = null;
  postsNavigationCurPage = null;
  postContents = null;
  currentPostsPage = 1;
  currentPostsPerPage = 100;
  currentAmountOfPosts = 0;
  currentPostSelectionID = null;
  currentSidebarSelectionID = null;
  currentSidebarSelectionType = null;
  currentSidebarScrollOffset = 0;

  constructor() {
    this.sidebar = document.getElementById("sidebar");
    this.posts = document.getElementById("posts");
    this.postsTable = document.getElementById("posts-table");
    this.postContents = document.getElementById("post-contents");
    this.postsNavigation = document.getElementById("posts-navigation");
    this.postsNavigationCurPage = document.getElementById("posts-curpage");
    this.initDraggers();
    this.initPostNavigationButtons();
    this.refreshFeeds();
  }

  initDraggers = () => {
    this.makeDragger({
      divider: document.getElementById("divider-v"),
      onDrag: (e) => {
        const newW = Math.max(80, Math.min(e.clientX, window.innerWidth - 120));
        this.sidebar.style.width = newW + "px";
      },
    });

    this.makeDragger({
      divider: document.getElementById("divider-h"),
      onDrag: (e) => {
        const main = document.getElementById("maindivider");
        const rect = main.getBoundingClientRect();
        const newH = Math.max(
          40,
          Math.min(e.clientY - rect.top, rect.height - 40),
        );
        this.posts.style.height = newH + "px";
      },
    });
  };

  makeDragger = ({ divider, onDrag }) => {
    let active = false;
    divider.addEventListener("mousedown", (e) => {
      active = true;
      divider.classList.add("active");
      e.preventDefault();
    });
    document.addEventListener("mousemove", (e) => {
      if (active) onDrag(e);
    });
    document.addEventListener("mouseup", () => {
      active = false;
      divider.classList.remove("active");
    });
  };

  getPostsPageCount = () => {
    return Math.ceil(this.currentAmountOfPosts / this.currentPostsPerPage);
  };

  initPostNavigationButtons = () => {
    document
      .getElementById("posts-btn-firstpage")
      .addEventListener("click", () => {
        this.getPosts(
          this.currentSidebarSelectionType,
          this.currentSidebarSelectionID,
          1,
        );
      });

    document
      .getElementById("posts-btn-prevpage")
      .addEventListener("click", () => {
        this.getPosts(
          this.currentSidebarSelectionType,
          this.currentSidebarSelectionID,
          Math.max(1, this.currentPostsPage - 1),
        );
      });

    document
      .getElementById("posts-btn-nextpage")
      .addEventListener("click", () => {
        const nextPage = Math.min(
          this.getPostsPageCount(),
          this.currentPostsPage + 1,
        );
        this.getPosts(
          this.currentSidebarSelectionType,
          this.currentSidebarSelectionID,
          nextPage,
        );
      });

    document
      .getElementById("posts-btn-lastpage")
      .addEventListener("click", () => {
        this.getPosts(
          this.currentSidebarSelectionType,
          this.currentSidebarSelectionID,
          this.getPostsPageCount(),
        );
      });
  };

  setCurrentSidebarSelection = (type, id) => {
    this.currentSidebarSelectionID = id;
    this.currentSidebarSelectionType = type;
    this.sidebar.childNodes.forEach((node) => {
      const cl = node.classList;
      cl.remove("selected");
      if (node.id === `src-${type}-${id}`) {
        cl.add("selected");
      }
    });
  };

  setCurrentPostSelection = (id) => {
    this.currentPostSelectionID = id;
    this.postsTable.childNodes.forEach((post) => {
      post.classList.remove("selected");
    });
    const post = document.getElementById(`post-${id}`);
    if (post) {
      post.classList.add("selected");
    }
  };

  getSubfolders = async (parentFolderID) => {
    const foldersResponse = await fetch(
      `/folders?parentFolderID=${parentFolderID}`,
    );
    return await foldersResponse.json();
  };

  refreshFeeds = async () => {
    this.currentSidebarScrollOffset = this.sidebar.scrollTop;

    const buffer = document.createElement("div");
    const originalSidebar = this.sidebar;
    this.sidebar = buffer;

    //    this.sidebar.innerHTML = "";
    const feedsResponse = await fetch("/feeds?getIcons=true", {
      cache: "no-store",
    });
    const feeds = await feedsResponse.json();

    const createFeedEntry = async (feed, depth) => {
      const feedDiv = document.createElement("div");
      feedDiv.id = `src-feed-${feed.id}`;
      feedDiv.classList.add("feed");
      feedDiv.classList.add("entry");
      feedDiv.style.setProperty("--depth", depth);
      feedDiv.addEventListener("click", () => {
        this.setCurrentSidebarSelection("feed", feed.id);
        this.getPosts("feed", feed.id, 1);
      });

      const icon = document.createElement("img");
      icon.classList.add("icon");
      if (feed.hasOwnProperty("icon")) {
        icon.src = `data:image/png;base64,${feed.icon}`;
      } else {
        icon.src = "/web/img/rss.png";
      }
      feedDiv.appendChild(icon);

      const caption = document.createElement("div");
      caption.classList.add("caption");
      caption.innerText = feed.title;
      caption.title = feed.title;
      feedDiv.appendChild(caption);

      const badge = document.createElement("div");
      badge.classList.add("badge");
      badge.innerText = feed.unreadCount;
      if (feed.unreadCount === 0) {
        badge.classList.add("hidden");
      }
      feedDiv.appendChild(badge);

      this.sidebar.appendChild(feedDiv);
    };

    const createFolderEntry = async (folder, depth) => {
      const folderDiv = document.createElement("div");
      folderDiv.id = `src-folder-${folder.id}`;
      folderDiv.classList.add("folder");
      folderDiv.classList.add("entry");
      folderDiv.innerText = folder.title;
      folderDiv.style.setProperty("--depth", depth);
      folderDiv.addEventListener("click", () => {
        this.setCurrentSidebarSelection("folder", folder.id);
        this.getPosts("folder", folder.id, 1);
      });
      this.sidebar.appendChild(folderDiv);

      await Promise.all(
        feeds
          .filter((feed) => feed.folder === folder.id)
          .sort((a, b) => a.sortOrder - b.sortOrder)
          .map(async (feed) => await createFeedEntry(feed, depth + 1)),
      );

      const subFolders = await this.getSubfolders(folder.id);
      await Promise.all(
        subFolders
          .sort((a, b) => a.sortOrder - b.sortOrder)
          .map(async (subfolder) => {
            await createFolderEntry(subfolder, depth + 1);
          }),
      );
    };

    const rootFolders = await this.getSubfolders(0);
    await Promise.all(
      rootFolders
        .sort((a, b) => a.sortOrder - b.sortOrder)
        .map(async (rootFolder) => {
          await createFolderEntry(rootFolder, 0);
        }),
    );
    originalSidebar.replaceChildren(...buffer.childNodes);
    this.sidebar = originalSidebar;
    this.sidebar.scrollTop = this.currentSidebarScrollOffset;

    this.setCurrentSidebarSelection(
      this.currentSidebarSelectionType,
      this.currentSidebarSelectionID,
    );
  };

  getPosts = async (parentType, parentID, page) => {
    this.postsTable.innerText = "";
    this.postContents.srcdoc = "";
    this.currentPostsPage = page;

    const postsResponse = await fetch(
      `/posts?parentType=${parentType}&parentID=${parentID}&perPage=${this.currentPostsPerPage}&page=${page}&showUnreadPostsAtTop=true`,
      {
        cache: "no-store",
      },
    );
    const posts = await postsResponse.json();
    this.currentAmountOfPosts = posts.count;
    this.postsNavigationCurPage.innerText = `Page ${page} / ${this.getPostsPageCount()}`;
    this.postsNavigation.style.display = "flex";

    posts.posts.map((post) => {
      const postEntry = document.createElement("div");
      postEntry.id = `post-${post.id}`;
      postEntry.classList.add("entry");
      if (!post.isRead) {
        postEntry.classList.add("row-unread");
      }
      postEntry.addEventListener("click", () => {
        this.postContents.srcdoc = this.getPostHTMLTemplate(post);
        this.setCurrentPostSelection(post.id);
        if (!post.isRead) {
          fetch("/set-posts-read-status", {
            method: "POST",
            headers: { "Content-Type": "application/x-www-form-urlencoded" },
            body: new URLSearchParams({
              markAsRead: true,
              feedsAndPostIDs: JSON.stringify([
                { feedID: post.feedID, postID: post.id },
              ]),
            }),
          }).then((res) => {
            if (res.ok) {
              document.getElementById(`post-unread-${post.id}`).innerHTML = "";
              postEntry.classList.remove("row-unread");
              this.refreshFeeds();
            }
          });
        }
      });

      const postUnread = document.createElement("div");
      postUnread.id = `post-unread-${post.id}`;
      postUnread.classList.add("unread");
      if (post.isRead) {
        postUnread.innerText = " ";
      } else {
        const postUnreadImg = document.createElement("img");
        postUnreadImg.src = "/web/img/unread.svg";
        postUnreadImg.width = 12;
        postUnreadImg.height = 12;
        postUnread.appendChild(postUnreadImg);
      }
      postEntry.appendChild(postUnread);

      const postTitle = document.createElement("div");
      postTitle.classList.add("title");
      postTitle.innerText = post.title;
      postEntry.appendChild(postTitle);

      const postDate = document.createElement("div");
      postDate.classList.add("date");
      postDate.innerText = new Date(post.datePublished).toLocaleString(
        undefined,
        { dateStyle: "short", timeStyle: "short" },
      );
      postEntry.appendChild(postDate);

      this.postsTable.appendChild(postEntry);
    });
  };

  getPostStyles = () => {
    // should be kept in sync with ZapFR::Client::WebEngineViewPost::postStyles() in WebEngineViewPost.cpp
    return `body { font-family: "Ubuntu", "Segoe UI", Tahoma, Geneva, Verdana, sans-serif; font-size: 1.2rem; background-color: #2a2a2a; color: #fff; margin: 2px 25px; max-width: 100vw; }
                        a { color: #fff; }
                        .zapfr_title { color: #fff; font-size: 1.4em; font-weight: bold; text-decoration: none; display: block; margin: 25px 0 10px 0; user-select:none; }
                        .zapfr_infoheader { font-size: 0.75em; display: flex; gap: 10px; margin-bottom: 5px; }
                        .zapfr_infoheader_separator { display: inline-block; margin-right: 10px; }
                        .zapfr_divider { margin-bottom: 30px; height: 1px; border: none; color: #fff; background-color: #fff; }
                        .zapfr_thumbnail_feedheader { margin: 25px 0 10px 0; padding-bottom: 6px; user-select:none; display: flex; flex-direction: row; gap: 15px; border-bottom: 1px solid #fff;
                        }
                        .zapfr_thumbnail_feedheader_content { color: #fff; font-size: 1.4em; font-weight: bold; text-decoration: none; display: inline-block; }
                        .zapfr_thumbnail_feedicon { max-width: 25px; max-height: 25px; }
                        .zapfr_thumbnail_grid { display: grid; grid-template-columns: repeat(6, 1fr); grid-column-gap: 10px; grid-row-gap: 20px; margin: 25px 0 25px 0; }
                        .zapfr_thumbnail_cell { display: flex; flex-direction: column; align-items:center; }
                        .zapfr_thumbnail_cell_img { max-width: 200px; max-height: 150px; border-radius: 15px; }
                        .zapfr_thumbnail_cell_title { font-size: 0.9em; }
                        .zapfr_thumbnail_cell_closebtn { display:none; position:absolute; right:0px; top:0px; width:25px; height:25px; }
                        .zapfr_thumbnail_cell:hover .zapfr_thumbnail_cell_closebtn { display:block; }
                        .zapfr_navigate_to_feed { font-size: 0.9em; text-decoration: none; border: 1px solid #006fe2; padding: 2px 6px; border-radius: 4px; cursor: pointer; }
                        
                        @media screen and (min-width:0px) and (max-width:850px) {
                           .zapfr_thumbnail_grid { grid-template-columns: repeat(2, 1fr); }
                        }
                        @media screen and (min-width:851px) and (max-width:1100px) {
                           .zapfr_thumbnail_grid { grid-template-columns: repeat(3, 1fr); }
                        }
                        @media screen and (min-width:1101px) and (max-width:1350px) {
                           .zapfr_thumbnail_grid { grid-template-columns: repeat(4, 1fr); }
                        }
                        @media screen and (min-width:1351px) and (max-width:1600px) {
                           .zapfr_thumbnail_grid { grid-template-columns: repeat(5, 1fr); }
                        }`;
  };

  getPostHTMLTemplate = (post) => {
    // should be kept in sync with ZapFR::Client::WebEngineViewPost::postHTMLTemplate in WebEngineViewPost.cpp
    let html = `<!DOCTYPE html>
            <html>
                <head>
                    <base href="${post.feedLink}">
                    <base target="_blank">
                    <style type="text/css">${this.getPostStyles()}</style>
                </head>
                <body>`;
    if (post.hasOwnProperty("link") && post.link.trim().length > 0) {
      html += `<a class="zapfr_title" href="${post.link}">${post.title}</a>`;
    } else {
      html += `<h1 class="zapfr_title">${post.title}</h1>`;
    }

    // todo: localize
    html += `<div class="zapfr_infoheader"><div>Published: ${post.datePublished}</div>`;
    if (post.hasOwnProperty("author") && post.author.trim().length > 0) {
      html += `<div><span class="zapfr_infoheader_separator">|</span>Author: ${post.author}</div>`;
    }
    if (
      post.hasOwnProperty("commentsURL") &&
      post.commentsURL.trim().length > 0
    ) {
      html += `<div><span class="zapfr_infoheader_separator">|</span><a href="${post.commentsURL}">View comments</a></div>`;
    }
    html += `</div>`;
    if (post.hasOwnProperty("categories") && post.categories.length > 0) {
      html += `<div class="zapfr_infoheader"><div>Categories: ${post.categories.map((cat) => cat.title).join(", ")}</div></div>`;
    }
    html += `<hr class="zapfr_divider">${post.content}`;
    html += `<script>window.addEventListener('load', () => {document.querySelectorAll("a").forEach(a => a.rel = "noopener noreferrer");} );</script>`;
    html += `<body></html>`;
    return html;
  };
}
