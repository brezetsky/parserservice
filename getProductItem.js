qt.article_selector = "{article_selector}";
qt.title_selector = "{title_selector}";
qt.photo_selector = "{photo_selector}";
qt.price_selector = "{price_selector}";
qt.description_selector = "{description_selector}";
qt.location_selector = "{location_selector}";
qt.location_etalon = "{location_etalon}";
qt.location_full_selector = "{location_full_selector}";
qt.logistic_price = "{logistic_price_value}";
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
qt.product_item.title = qt.jQuery(qt.title_selector).text().replace(qt.product_item.article, "");
qt.product_item.description = qt.jQuery(qt.description_selector).html();
qt.product_item.location = qt.jQuery(qt.location_selector).text();
qt.product_item.end_time = qt.jQuery(qt.end_time_selector).text().replace(" at", "");
qt.product_item.end_time = qt.product_item.end_time.replace("Jan", "01");
qt.product_item.end_time = qt.product_item.end_time.replace("JAN", "01");
qt.product_item.end_time = qt.product_item.end_time.replace("Feb", "02");
qt.product_item.end_time = qt.product_item.end_time.replace("FEB", "02");
qt.product_item.end_time = qt.product_item.end_time.replace("Mar", "03");
qt.product_item.end_time = qt.product_item.end_time.replace("MAR", "03");
qt.product_item.end_time = qt.product_item.end_time.replace("Apr", "04");
qt.product_item.end_time = qt.product_item.end_time.replace("APR", "04");
qt.product_item.end_time = qt.product_item.end_time.replace("May", "05");
qt.product_item.end_time = qt.product_item.end_time.replace("MAJ", "05");
qt.product_item.end_time = qt.product_item.end_time.replace("Jun", "06");
qt.product_item.end_time = qt.product_item.end_time.replace("JUN", "06");
qt.product_item.end_time = qt.product_item.end_time.replace("Jul", "07");
qt.product_item.end_time = qt.product_item.end_time.replace("JUL", "07");
qt.product_item.end_time = qt.product_item.end_time.replace("Aug", "08");
qt.product_item.end_time = qt.product_item.end_time.replace("AUG", "08");
qt.product_item.end_time = qt.product_item.end_time.replace("Sep", "09");
qt.product_item.end_time = qt.product_item.end_time.replace("SEP", "09");
qt.product_item.end_time = qt.product_item.end_time.replace("Oct", "10");
qt.product_item.end_time = qt.product_item.end_time.replace("OCT", "10");
qt.product_item.end_time = qt.product_item.end_time.replace("Nov", "11");
qt.product_item.end_time = qt.product_item.end_time.replace("NOV", "11");
qt.product_item.end_time = qt.product_item.end_time.replace("Dec", "12");
qt.product_item.end_time = qt.product_item.end_time.replace("DEC", "12");
qt.product_item.ident_name = location.toString();
qt.photoNum = 0;
qt.jQuery(qt.photo_selector).each(function() {
    qt.dataSrc = qt.jQuery(this).data("src");
    qt.srcSet = qt.jQuery(this).attr("srcset");
    qt.src = "";
    if(typeof qt.dataSrc !== typeof undefined && qt.dataSrc !== false)
    {
        qt.src = qt.dataSrc;
    }
    else if(typeof qt.srcSet !== typeof undefined && qt.srcSet !== false)
    {
        qt.src = qt.srcSet.split(' 920w,')[0];
    }

    else
    {
        qt.src = qt.jQuery(this).attr("src");
    }
    var phost = location.protocol+'//'+location.hostname;
    if(qt.src.indexOf("http") == -1 && qt.src.indexOf(location.hostname) == -1 && (typeof qt.srcSet === typeof undefined || qt.srcSet === false)) {
        qt.src = phost + qt.src;
    }
    if(qt.src.indexOf("http") == -1)
    {
        qt.src = "http:" + qt.src;
    }

    qt.product_item.photos[qt.photoNum] = qt.src;
    qt.photoNum++;
});

qt.printPrice = function() {
    qt.price_formula = qt.price_formula.replace(/{price}/g, qt.orig_price);

    for(var key in qt.additional_fields)
    {
        qt.additional_field = qt.jQuery(qt.additional_fields[key]).text().replace(/[^\d;]/g, '');
        qt.price_formula = qt.price_formula.replace(/{' + key + '}/g, qt.additional_field);
    }

    regex = new RegExp('^.*' + qt.location_etalon + '.*$','i');
    if(regex.test(qt.jQuery(qt.location_selector).text()))
    {
        qt.logistic_price = "0";
    }

    qt.price_formula = qt.price_formula.replace(/{logistic_price}/g, qt.logistic_price);

    //console.log(qt.logistic_price);
    qt.product_item.price = eval(qt.price_formula);
    qt.product_item.price = Math.ceil((qt.product_item.price)*100)/100;
    //console.log(qt.product_item.price);
}

qt.calcUah = function(response){
    qt.orig_price = qt.orig_price * response[0].rate;
    qt.jQuery.ajax({
                       url: "https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?valcode=EUR&json",
                       async: false,
                       dataType: "json",
                       success: qt.calcEur
                   });
}

qt.calcEur = function(response){
    qt.orig_price = qt.orig_price / response[0].rate;
    qt.printPrice();
}

qt.orig_price = qt.jQuery(qt.price_selector).text();

if(new RegExp("^.*SEK|kr.*$", "i").test(qt.orig_price))
{
    qt.orig_price = qt.orig_price.replace(/[^\d;]/g, '');
    qt.jQuery.ajax({
                       url: "https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?valcode=SEK&json",
                       async: false,
                       dataType: "json",
                       success: qt.calcUah
                   });
}
else {
    qt.orig_price = qt.orig_price.replace(/[^\d;]/g, '');
    qt.printPrice();
}

