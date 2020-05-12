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
            QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
           //get the jsonObject
            QJsonObject jObject = doc.object();
            QVariantMap product_map = jObject.toVariantMap();
            QString ident_name = product_map["ident_name"].toString();
            QSqlQuery productCheck(database);
            productCheck.prepare("SELECT id FROM products WHERE ident_name = :in;");
            productCheck.bindValue(":in", ident_name);
            productCheck.exec();
            if(productCheck.size() > 0)
            {
                productCheck.next();
                int id = productCheck.value(0).toInt();
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
                end_time = QString::number(end_timedt);
                QString price = product_map["price"].toString();
                QSqlQuery productUpdate(database);
                productUpdate.prepare("UPDATE products SET end_time = :et, price = :pv WHERE id = :id;");
                productUpdate.bindValue(":et", end_time);
                productUpdate.bindValue(":pv", price);
                productUpdate.bindValue(":id", id);
                productUpdate.exec();
                emit parserEnd(p, "ProductParser");
            }
            else
            {
                translate();
            }
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
        QJsonObject jObject = doc.object();
        QVariantMap product_map = jObject.toVariantMap();
        product_item.article = product_map["article"].toString();
        product_item.category_id = product_map["category_id"].toString();
        product_item.price = product_map["price"].toString();
        product_item.ident_name = product_map["ident_name"].toString();
        product_item.slug = product_map["ident_name"].toString().section('/', -1);
        product_item.status = product_map["status"].toString();
        QJsonObject jPhotos = product_map["photos"].toJsonObject();
        product_item.photos = jPhotos.toVariantMap();
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
        QSqlQuery productGet(database);
        productGet.prepare("SELECT id FROM products WHERE ident_name = :in;");
        productGet.bindValue(":in", product_item.ident_name);
        productGet.exec();
        productGet.next();
        product_item.id = productGet.value(0).toInt();
        QDir().mkpath(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main");
        QDir().mkpath(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional");
        QFile folder_id(settings->AbsUploadPath + "/products/" + QString::number(product_item.id));
        QFile::Permissions permissions = folder_id.permissions();
        permissions.setFlag(QFile::ExeOwner);
        permissions.setFlag(QFile::ReadOwner);
        permissions.setFlag(QFile::WriteOwner);
        permissions.setFlag(QFile::ExeUser);
        permissions.setFlag(QFile::ReadUser);
        permissions.setFlag(QFile::WriteUser);
        permissions.setFlag(QFile::ExeGroup);
        permissions.setFlag(QFile::ReadGroup);
        permissions.setFlag(QFile::WriteGroup);
        permissions.setFlag(QFile::ExeOther);
        permissions.setFlag(QFile::ReadOther);
        permissions.setFlag(QFile::WriteOther);
        folder_id.setPermissions(permissions);
        QFile folder_main(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main");
        folder_main.setPermissions(permissions);
        QFile folder_additional(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional");
        folder_additional.setPermissions(permissions);
        uploadPhoto();
    }
}

void ProductParser::uploadPhoto()
{
    QString pkey = product_item.photos.firstKey();
    QString link = product_item.photos[pkey].toString();
    product_item.photos.remove(pkey);
    QNetworkAccessManager *image_loader = new QNetworkAccessManager();
    connect(image_loader, &QNetworkAccessManager::finished, this, &ProductParser::savePhotos);
    QUrl ilink = QUrl(link);
    QNetworkRequest r(ilink);
    image_loader->get(r);
}

void ProductParser::savePhotos(QNetworkReply *reply)
{
    if(main_image)
    {
        QPixmap image;
        image.loadFromData(reply->readAll());
        QString link = reply->url().toString();
        QString fullfileName = link.right(link.size()-link.lastIndexOf("/")-1);
        QString fileName = fullfileName.left(fullfileName.indexOf("."));
        QString ext = fullfileName.right(fullfileName.size()-fullfileName.lastIndexOf("."));
        image.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fullfileName);
        QFile image_file(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fullfileName);
        QFile::Permissions permissions;
        permissions.setFlag(QFile::ReadOwner);
        permissions.setFlag(QFile::WriteOwner);
        permissions.setFlag(QFile::ReadUser);
        permissions.setFlag(QFile::WriteUser);
        permissions.setFlag(QFile::ReadGroup);
        permissions.setFlag(QFile::WriteGroup);
        permissions.setFlag(QFile::ReadOther);
        permissions.setFlag(QFile::WriteOther);
        image_file.setPermissions(permissions);
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fullfileName, QMimeDatabase::MatchContent);
        QString mime_type = mime.name();
        if(image.width() >= 1000)
        {
            QPixmap image_2000 = image.scaled(1000, 1000, Qt::KeepAspectRatio);
            image_2000.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_2000w" + ext);
            QFile image_file_2000w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_2000w" + ext);
            image_file_2000w.setPermissions(permissions);
        }
        if(image.width() >= 550)
        {
            QPixmap image_1000 = image.scaled(550, 550, Qt::KeepAspectRatio);
            image_1000.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_1000w" + ext);
            QFile image_file_1000w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_1000w" + ext);
            image_file_1000w.setPermissions(permissions);
        }
        if(image.width() >= 400)
        {
            QPixmap image_767 = image.scaled(400, 400, Qt::KeepAspectRatio);
            image_767.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_767w" + ext);
            QFile image_file_767w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fileName + "_767w" + ext);
            image_file_767w.setPermissions(permissions);
        }
        QSqlQuery imageInserter(database);
        imageInserter.prepare("INSERT INTO files (table_name, record_id, filename, fullfilename, ext, fullextension, system_type) VALUES('products', :product_id, :filenamev, :fullfilenamev, :extv, :fullextensionv, 'main')");
        imageInserter.bindValue(":product_id", product_item.id);
        imageInserter.bindValue(":filenamev", fileName);
        imageInserter.bindValue(":fullfilenamev", fullfileName);
        imageInserter.bindValue(":extv", ext);
        imageInserter.bindValue(":fullextensionv", mime_type);
        imageInserter.exec();
        QSqlQuery imageIdGetter(database);
        imageIdGetter.prepare("SELECT id FROM files WHERE table_name = 'products' AND record_id = :product_id");
        imageIdGetter.bindValue(":product_id", product_item.id);
        imageIdGetter.exec();
        imageIdGetter.next();
        int image_id = imageIdGetter.value(0).toInt();
        QSqlQuery productUpdate(database);
        productUpdate.prepare("UPDATE products SET main_img_id = :mid WHERE id = :id;");
        productUpdate.bindValue(":mid", image_id);
        productUpdate.bindValue(":id", product_item.id);
        productUpdate.exec();
        main_image = false;
    }
    else
    {
        QPixmap image;
        image.loadFromData(reply->readAll());
        QString link = reply->url().toString();
        QString fullfileName = link.right(link.size()-link.lastIndexOf("/")-1);
        QString fileName = fullfileName.left(fullfileName.indexOf("."));
        QString ext = fullfileName.right(fullfileName.size()-fullfileName.lastIndexOf("."));
        image.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fullfileName);
        QFile image_file(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fullfileName);
        QFile::Permissions permissions;
        permissions.setFlag(QFile::ReadOwner);
        permissions.setFlag(QFile::WriteOwner);
        permissions.setFlag(QFile::ReadUser);
        permissions.setFlag(QFile::WriteUser);
        permissions.setFlag(QFile::ReadGroup);
        permissions.setFlag(QFile::WriteGroup);
        permissions.setFlag(QFile::ReadOther);
        permissions.setFlag(QFile::WriteOther);
        image_file.setPermissions(permissions);
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForFile(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fullfileName, QMimeDatabase::MatchContent);
        QString mime_type = mime.name();
        if(image.width() >= 1000)
        {
            QPixmap image_2000 = image.scaled(1000, 1000, Qt::KeepAspectRatio);
            image_2000.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_2000w" + ext);
            QFile image_file_2000w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_2000w" + ext);
            image_file_2000w.setPermissions(permissions);
        }
        if(image.width() >= 550)
        {
            QPixmap image_1000 = image.scaled(550, 550, Qt::KeepAspectRatio);
            image_1000.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_1000w" + ext);
            QFile image_file_1000w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_1000w" + ext);
            image_file_1000w.setPermissions(permissions);
        }
        if(image.width() >= 400)
        {
            QPixmap image_767 = image.scaled(400, 400, Qt::KeepAspectRatio);
            image_767.save(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_767w" + ext);
            QFile image_file_767w(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fileName + "_767w" + ext);
            image_file_767w.setPermissions(permissions);
        }
        QSqlQuery imageInserter(database);
        imageInserter.prepare("INSERT INTO files (table_name, record_id, filename, fullfilename, ext, fullextension, system_type) VALUES('products', :product_id, :filenamev, :fullfilenamev, :extv, :fullextensionv, 'additional')");
        imageInserter.bindValue(":product_id", product_item.id);
        imageInserter.bindValue(":filenamev", fileName);
        imageInserter.bindValue(":fullfilenamev", fullfileName);
        imageInserter.bindValue(":extv", ext);
        imageInserter.bindValue(":fullextensionv", mime_type);
        imageInserter.exec();
    }
    if(product_item.photos.size() > 0)
    {
        uploadPhoto();
    }
    else {
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
