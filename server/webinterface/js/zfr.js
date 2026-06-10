class ZapFeedReader {
  sidebar = null;
  posts = null;

  constructor() {
    this.sidebar = document.getElementById("sidebar");
    this.posts = document.getElementById("posts");
    this.initDraggers();
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

  getSubfolders = async (parentFolderID) => {
    const foldersResponse = await fetch(
      `/folders?parentFolderID=${parentFolderID}`,
    );
    return await foldersResponse.json();
  };

  refreshFeeds = async () => {
    const feedsResponse = await fetch("/feeds?getIcons=true", {
      cache: "no-store",
    });
    const feeds = await feedsResponse.json();

    const createFeedEntry = async (feed, depth) => {
      const feedDiv = document.createElement("div");
      feedDiv.classList.add("feed");
      feedDiv.classList.add("entry");
      feedDiv.style.setProperty("--depth", depth);

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
      caption.addEventListener("click", () => {
        console.log("loading feed", feed);
      });

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
      folderDiv.classList.add("folder");
      folderDiv.classList.add("entry");
      folderDiv.innerText = folder.title;
      folderDiv.style.setProperty("--depth", depth);
      this.sidebar.appendChild(folderDiv);

      feeds
        .filter((feed) => feed.folder === folder.id)
        .sort((a, b) => a.sortOrder - b.sortOrder)
        .map((feed) => createFeedEntry(feed, depth + 1));

      const subFolders = await this.getSubfolders(folder.id);
      subFolders
        .sort((a, b) => a.sortOrder - b.sortOrder)
        .map((subfolder) => {
          createFolderEntry(subfolder, depth + 1);
        });
    };

    const rootFolders = await this.getSubfolders(0);
    rootFolders
      .sort((a, b) => a.sortOrder - b.sortOrder)
      .map((rootFolder) => {
        createFolderEntry(rootFolder, 0);
      });
  };
}
