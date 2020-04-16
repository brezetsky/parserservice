#include "productparser.h"
#include "productitem.h"

ProductParser::ProductParser(ParserRow *r, WebPage *wp, QSqlDatabase& db,  ParserSettings *s, QObject *parent) : QObject(parent)
{
    p = wp;
    row = r;
    settings = s;
    database = db;
    QSqlQuery getLanguagesQuery(database);
    getLanguagesQuery.exec("SELECT abrv FROM avaible_languages");
    while (getLanguagesQuery.next()) {
        languages.append(getLanguagesQuery.value(0).toString());
    }
}

void ProductParser::stop()
{
    emit parserEnd(p, "TotalStop");
}

void ProductParser::parse()
{
    QFile file;
    file.setFileName(":/jquery.min.js");
    file.open(QIODevice::ReadOnly);
    QString jQuery = file.readAll();
    jQuery.append("\nvar qt = { 'jQuery': jQuery.noConflict(true) };");
    file.close();
    //emit sendLog(jQuery);
    p->runJavaScript(jQuery);
    file.setFileName(":/getProductItem.js");
    file.open(QIODevice::ReadOnly);
    QString getProductItem = file.readAll();
    file.close();
    getProductItem = getProductItem.replace("{article_selector}", row->article_selector);
    getProductItem = getProductItem.replace("{title_selector}", row->title_selector);
    getProductItem = getProductItem.replace("{photo_selector}", row->photo_selector);
    getProductItem = getProductItem.replace("{price_selector}", row->price_selector);
    getProductItem = getProductItem.replace("{description_selector}", row->description_selector);
    getProductItem = getProductItem.replace("{location_selector}", row->location_selector);
    getProductItem = getProductItem.replace("{location_etalon}", row->location_etalon);
    getProductItem = getProductItem.replace("{location_full_selector}", row->location_full_selector);
    getProductItem = getProductItem.replace("{logistic_price}", QString::number(row->logistic_price));
    getProductItem = getProductItem.replace("{end_time_selector}", row->end_time_selector);
    getProductItem = getProductItem.replace("{additional_fields}", row->additional_fields);
    getProductItem = getProductItem.replace("{price_formula}", row->price_formula);
    getProductItem = getProductItem.replace("{category_id}", QString::number(row->category_id));
    getProductItem = getProductItem.replace("{status}", QString::number(row->publicate_status));
    //emit sendLog(getProductItem);
    p->runJavaScript(getProductItem,[this](const QVariant &v) {
        //emit sendLog(v.toString());
        product = v;
        translate();
    });
}

void ProductParser::translate()
{
    QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
   //get the jsonObject
    QJsonObject jObject = doc.object();
    QVariantMap product_map = jObject.toVariantMap();
    foreach (QString abrv, languages) {
        QUrl url = QUrl("https://translate.yandex.net/api/v1.5/tr.json/translate?key=" + settings->YandexTranslateKey +
                        "&text=" + product_map["title"].toString() +
                   "&text=" + product_map["description"].toString() +
   "&text=" + product_map["location"].toString() + "&lang=" + abrv);
        QNetworkRequest request(url);
        QNetworkAccessManager *mngr = new QNetworkAccessManager(this);
#if defined(QUSEPROXY)
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName("195.133.144.163");
        proxy.setPort(3128);
        proxy.setUser("mixajlov");
        proxy.setPassword("evilkiller190813");
        mngr->setProxy(proxy);
#endif
        connect(mngr, &QNetworkAccessManager::finished, this, &ProductParser::productCreate);
        mngr->get(request);
    }
    //emit sendLog("Link parse finished!");
}

void ProductParser::productCreate(QNetworkReply *reply)
{
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromUtf8(reply->readAll()).toUtf8());
   //get the jsonObject
    QJsonObject jObject = doc.object();
    QVariantMap translateMap = jObject.toVariantMap();
    QString lang = translateMap["lang"].toString().right(translateMap["lang"].toString().indexOf('-'));
    QVariantList text = translateMap["text"].toList();
    product_item.title["title_" + lang] = text.at(0).toString();
    product_item.content["content_" + lang] = text.at(1).toString().replace("\n", "<br>");
    product_item.anons["anons_" + lang] = text.at(1).toString().left(254).replace("\n", "<br>");
    product_item.description["description_" + lang] = text.at(1).toString().left(254).replace("\n", "<br>");
    product_item.location["location_" + lang] = text.at(2).toString();
    if(languages.size() == product_item.title.size())
    {
        QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
       //get the jsonObject
        QJsonObject jObject = doc.object();
        QVariantMap product_map = jObject.toVariantMap();
        product_item.article = product_map["article"].toString();
        product_item.category_id = product_map["category_id"].toString();
        product_item.price = product_map["price"].toString();
        product_item.ident_name = product_map["ident_name"].toString();
        product_item.slug = product_map["ident_name"].toString().section('/', -1);
        product_item.status = product_map["status"].toString();
        emit parserEnd(p, "ProductParser");
    }
}
