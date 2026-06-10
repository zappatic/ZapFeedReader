{% include "_header.tpl" %}


<div id="container">
  <aside id="sidebar"></aside>
  <div class="divider-v" id="divider-v"></div>
  <div id="maindivider">
    <div id="posts"></div>
    <div class="divider-h" id="divider-h"></div>
    <div id="post"></div>
  </div>
</div>


<script>
    window.addEventListener('load', () => {
        new ZapFeedReader();
    });
</script>

{% include "_footer.tpl" %}
