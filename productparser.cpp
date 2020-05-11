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
        p->runJavaScript("JSON.stringify(qt.product_item);",[this](const QVariant &tv) {
            //emit sendLog(tv.toString());
            product = tv;
            translate();
        });
    });
}

void ProductParser::translate()
{
    QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
   //get the jsonObject
    QJsonObject jObject = doc.object();
    QVariantMap product_map = jObject.toVariantMap();
    foreach (QString abrv, languages) {
        QUrl url = QUrl("https://translate.yandex.net/api/v1.5/tr.json/translate");
        QUrlQuery postData;
        postData.addQueryItem("key", settings->YandexTranslateKey);
        postData.addQueryItem("text", product_map["title"].toString());
        postData.addQueryItem("text", product_map["description"].toString());
        postData.addQueryItem("text", product_map["location"].toString());
        postData.addQueryItem("format", "html");
        postData.addQueryItem("lang", abrv);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded");
        QNetworkAccessManager *mngr = new QNetworkAccessManager();
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
        mngr->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
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
    product_item.title["`title_" + lang + "`"] = "'" + text.at(0).toString() + "'";
    product_item.content["`content_" + lang + "`"] = "'" + text.at(1).toString() + "'";
    product_item.anons["`anons_" + lang + "`"] = "'" + text.at(1).toString().remove(QRegExp("<[^>]*>")).left(254) + "'";
    product_item.description["`description_" + lang + "`"] = "'" + text.at(1).toString().remove(QRegExp("<[^>]*>")).left(254) + "'";
    product_item.location["`location_" + lang + "`"] = "'" + text.at(2).toString() + "'";
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
        QString end_time = product_map["end_time"].toString();
        qint64 utc_value = 0;
        if(end_time.contains("CEST"))
        {
            utc_value = 1;
            end_time = end_time.remove("CEST");
        }
        if(end_time.contains(" EET"))
        {
            utc_value = 1;
            end_time = end_time.remove(" EET");
        }
        if(row->date_format == "d MM hh:mm yyyy")
        {
            end_time = end_time + " " + QDateTime::currentDateTime().toString("yyyy");
        }
        if(row->date_format == "MM d h:mm AP yyyy")
        {
            end_time = end_time + QDateTime::currentDateTime().toString("yyyy");
        }
        if(row->date_format == "d MM hh:mm")
        {
            row->date_format = row->date_format + " yyyy";
            end_time = end_time + " " + QDateTime::currentDateTime().toString("yyyy");
        }
        if(row->date_format == "MM d h:mm AP")
        {
            row->date_format = row->date_format + " yyyy";
            end_time = end_time + QDateTime::currentDateTime().toString("yyyy");
        }
        end_time = end_time.replace(0x00A0, " ");
        int end_timedt = QDateTime::fromString(end_time, row->date_format).toTime_t();
        if(utc_value != 0)
        {
            end_timedt = end_timedt + 3600;
        }
        product_item.end_time = QString::number(end_timedt);
        QSqlQuery productInsert(database);
        QList<QString> fields_title = product_item.title.keys();
        QString title_keys = fields_title.join(", ");
        QList<QString> fields_title_values = product_item.title.values();
        QString title_values = fields_title_values.join(", ");

        QList<QString> fields_content = product_item.content.keys();
        QString content_keys = fields_content.join(", ");
        QList<QString> fields_content_values = product_item.content.values();
        QString content_values = fields_content_values.join(", ").replace("'", "\'");

        QList<QString> fields_anons = product_item.anons.keys();
        QString anons_keys = fields_anons.join(", ");
        QList<QString> fields_anons_values = product_item.anons.values();
        QString anons_values = fields_anons_values.join(", ").replace("'", "\'");

        QList<QString> fields_description = product_item.description.keys();
        QString description_keys = fields_description.join(", ");
        QList<QString> fields_description_values = product_item.description.values();
        QString description_values = fields_description_values.join(", ").replace("'", "\'");

        QList<QString> fields_location = product_item.location.keys();
        QString location_keys = fields_location.join(", ");
        QList<QString> fields_location_values = product_item.location.values();
        QString location_values = fields_location_values.join(", ").replace("'", "\'");

        productInsert.prepare("INSERT INTO products ("+ title_keys + ", " + anons_keys + ", " + content_keys + ", " + description_keys + ", " + location_keys + ", `article`, `category_id`, `price`, `end_time`, `slug`, `ident_name`) "
                              "VALUES(" + title_values + ", " + anons_values + ", " + content_values + ", " + description_values + ", " + location_values + ", :article_value, :category_id_value, :price_value, :end_time_value, :slug_value, :ident_name_value);");
        productInsert.bindValue(":article_value", product_item.article);
        productInsert.bindValue(":category_id_value", product_item.category_id);
        productInsert.bindValue(":price_value", product_item.price);
        productInsert.bindValue(":end_time_value", product_item.end_time);
        productInsert.bindValue(":slug_value", product_item.slug);
        productInsert.bindValue(":ident_name_value", product_item.ident_name);
        productInsert.exec();
        //emit sendLog(getLastExecutedQuery(productInsert));
        emit parserEnd(p, "ProductParser");
    }
}

QString ProductParser::getLastExecutedQuery(const QSqlQuery& query)
{
 QString str = query.lastQuery();
 QMapIterator<QString, QVariant> it(query.boundValues());
 while (it.hasNext())
 {
  it.next();
  str.replace(it.key(),it.value().toString());
 }
 return str;
}
