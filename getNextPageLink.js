var result = false;
var phost = location.protocol+'//'+location.hostname;
if(qt.jQuery('{next_page_selector}').length > 0)
{
    if(qt.jQuery('{next_page_selector}')[0].tagName == "a")
    {
        result = qt.jQuery('{next_page_selector}').attr("href");
    }
    else
    {
        result = qt.jQuery('{next_page_selector}').closest("a").next("a").attr("href");
    }

    if(result.indexOf(location.hostname) == -1) {
        result = phost + result;
    }
}
result;
