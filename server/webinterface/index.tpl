{% include "_header.tpl" %}


<div id="container">
  <aside id="sidebar"></aside>
  <div class="divider-v" id="divider-v"></div>
  <div id="maindivider">
    <div id="posts">
        <div id="posts-table"></div>
        <div id="posts-navigation"></div>
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
