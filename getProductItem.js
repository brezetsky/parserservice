qt.article_selector = "{article_selector}";
qt.title_selector = "{title_selector}";
qt.photo_selector = "{photo_selector}";
qt.price_selector = "{price_selector}";
qt.description_selector = "{description_selector}";
qt.location_selector = "{location_selector}";
qt.location_etalon = "{location_etalon}";
qt.location_full_selector = "{location_full_selector}";
qt.logistic_price = "{logistic_price}";
qt.end_time_selector = "{end_time_selector}";
qt.additional_fields = JSON.parse("{additional_fields}");
qt.price_formula = "{price_formula}";

qt.product_item = {
    article: "",
    category_id: "{category_id}",
    title: "",
    photos: {},
    price: "",
    description: "",
    location: "",
    end_time: "",
    ident_name: "",
    status: "{status}",
};

qt.product_item.article = qt.jQuery(qt.article_selector).text();
qt.product_item.title = qt.jQuery(qt.title_selector).text();
qt.product_item.description = qt.jQuery(qt.description_selector).text();
qt.product_item.location = qt.jQuery(qt.location_selector).text();
qt.product_item.end_time = qt.jQuery(qt.end_time_selector).text();
qt.product_item.ident_name = location.toString();
qt.photoNum = 0;
qt.jQuery(qt.photo_selector).each(function() {
    qt.dataSrc = qt.jQuery(this).data("src");
    qt.src = "";
    if(typeof qt.dataSrc !== typeof undefined && qt.dataSrc !== false)
    {
        qt.src = qt.dataSrc;
    }
    else
    {
        qt.src = qt.jQuery(this).attr("src");
    }
    var phost = location.protocol+'//'+location.hostname;
    if(qt.src.indexOf(location.hostname) == -1) {
        qt.src = phost + qt.src;
    }
    qt.product_item.photos[qt.photoNum] = qt.src;
    qt.photoNum++;
});

qt.orig_price = qt.jQuery(qt.price_selector).text().replace(/[^\d;]/g, '');
qt.price_formula = qt.price_formula.replace(/{price}/g, qt.orig_price);

for(var key in qt.additional_fields)
{
    qt.additional_field = qt.jQuery(qt.additional_fields[key]).text().replace(/[^\d;]/g, '');
    qt.price_formula = qt.price_formula.replace(/{' + key + '}/g, qt.additional_field);
}

if(qt.jQuery(qt.location_selector).text() == qt.location_etalon)
{
    qt.logistic_price = "0";
}

qt.price_formula = qt.price_formula.replace(/{lprice}/g, qt.logistic_price);

qt.product_item.price = eval(qt.price_formula);

JSON.stringify(qt.product_item);

