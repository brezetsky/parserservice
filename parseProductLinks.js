qt.links = {};
qt.num = 0;
qt.jQuery('{item_selector}').each( function () {
    var link = qt.jQuery(this).attr('href');
    var phost = location.protocol+'//'+location.hostname;
    if(link.indexOf(location.hostname) == -1) {
        link = phost + link;
    }
    qt.links[qt.num] = link;
    qt.num++;
});
JSON.stringify(qt.links);
