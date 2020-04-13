var result = false;
var phost = location.protocol+'//'+location.hostname;
if(qt.jQuery('{next_page_selector}').length > 0)
{
    result = qt.jQuery('{next_page_selector}').attr("href");
    if(result.indexOf(location.hostname) == -1) {
        result = phost + result;
    }
}
result;
