{% include "_header.tpl" %}


<div id="container">
  <aside id="sidebar"></aside>
  <div class="divider-v" id="divider-v"></div>
  <div id="maindivider">
    <div id="posts">
        <div id="posts-table"></div>
        <div id="posts-navigation" style="display: none;">
            <button id="posts-btn-firstpage" class="themedsvgbtn" title="First page">
                <svg width="20px" height="20px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M4 4V20M8 12H20M8 12L12 8M8 12L12 16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" />
                </svg>
            </button>
            <button id="posts-btn-prevpage" class="themedsvgbtn" title="Previous page">
                <svg width="20px" height="20px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M4 12H20M4 12L8 8M4 12L8 16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" />
                </svg>
            </button>
            <div id="posts-curpage">Page </div>
            <button id="posts-btn-nextpage" class="themedsvgbtn" title="Next page">
                <svg width="20px" height="20px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M4 12H20M20 12L16 8M20 12L16 16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" />
                </svg>
            </button>
            <button id="posts-btn-lastpage" class="themedsvgbtn" title="Last page">
                <svg width="20px" height="20px" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
                    <path d="M20 4V20M4 12H16M16 12L12 8M16 12L12 16" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" />
                </svg>
            </button>
        </div>
    </div>
    <div class="divider-h" id="divider-h"></div>
    <div id="post">
        <iframe id="post-contents" sandbox="allow-scripts allow-popups" csp="default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline';"></iframe>
    </div>

  </div>
</div>


<script>
    window.addEventListener('load', () => {
        new ZapFeedReader();
    });
</script>

{% include "_footer.tpl" %}
