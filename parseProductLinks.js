qt.links = [];
qt.jQuery('{item_selector}').each( function () {
    var link = qt.jQuery(this).attr('href');
    var phost = location.protocol+'//'+location.hostname;
    if(link.indexOf(location.hostname) == -1) {
        link = phost + link;
    }
    qt.links.push(link);
});
JSON.stringify(qt.links);
