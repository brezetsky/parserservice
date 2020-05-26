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
    //qWarning("Parser ProductParser parse!!!");
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
    getProductItem = getProductItem.replace("{logistic_price_value}", QString::number(row->logistic_price));
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
                if(end_time.contains("EET"))
                {
                    utc_value = 1;
                    end_time = end_time.remove("EET");
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
                end_time = end_time.remove(QRegExp("^\\s+"));
                end_time = end_time.remove(QRegExp("\\s+$"));
                end_time = end_time.replace(QRegExp("\\s+"), " ");
                end_time = end_time.replace(QRegExp("\\s"), "-");
                row->date_format = row->date_format.replace(QRegExp("\\s"), "-");
                //qWarning(end_time.toLatin1().constData());
                //qWarning(row->date_format.toLatin1().constData());
                uint end_timedt = QDateTime::fromString(end_time, row->date_format).toTime_t();
                if(end_timedt > 100000000)
                {
                    if(utc_value != 0)
                    {
                        end_timedt = end_timedt + 3600;
                    }
                    end_timedt = end_timedt - 86400;
                    if(end_timedt > QDateTime::currentDateTime().toTime_t())
                    {
                        end_time = QString::number(end_timedt);
                        //qWarning(end_time.toLatin1().constData());
                        QString price = product_map["price"].toString();
                        QSqlQuery productUpdate(database);
                        productUpdate.prepare("UPDATE products SET end_time = :et, price = :pv WHERE id = :id;");
                        productUpdate.bindValue(":et", end_time);
                        productUpdate.bindValue(":pv", price);
                        productUpdate.bindValue(":id", id);
                        if(!productUpdate.exec())
                        {
                            QSqlQuery logInsert(database);
                            logInsert.prepare("INSERT INTO service_logs (code,message,link,create_dt) VALUES(:code_value,:message_value,:link_value,:create_dt_value);");
                            logInsert.bindValue(":code_value", 10);
                            logInsert.bindValue(":message_value", productUpdate.lastError().text());
                            logInsert.bindValue(":link_value", ident_name);
                            logInsert.bindValue(":create_dt_value", QDateTime::currentDateTime().toTime_t());
                            logInsert.exec();
                            emit parserEnd(p, "ProductParser");
                        }
                        else
                        {
                            emit parserEnd(p, "ProductParser");
                        }
                    }
                }
                else {
                    QSqlQuery logInsert(database);
                    logInsert.prepare("INSERT INTO service_logs (code,message,link,create_dt) VALUES(:code_value,:message_value,:link_value,:create_dt_value);");
                    logInsert.bindValue(":code_value", 11);
                    logInsert.bindValue(":message_value", "End date of auction not correct!");
                    logInsert.bindValue(":link_value", ident_name);
                    logInsert.bindValue(":create_dt_value", QDateTime::currentDateTime().toTime_t());
                    logInsert.exec();
                    emit parserEnd(p, "ProductParser");
                }
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
    //qWarning(product.toString().toUtf8());
    QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
    //qWarning("Parser ProductParser translate127!!!");
   //get the jsonObject
    QJsonObject jObject = doc.object();
    //qWarning("Parser ProductParser translate128!!!");
    QVariantMap product_map = jObject.toVariantMap();
    //qWarning("Parser ProductParser translate132!!!");
    foreach (QString abrv, languages) {
        //qWarning("Parser ProductParser translate134!!!");
        QUrl url = QUrl("https://translate.yandex.net/api/v1.5/tr.json/translate");
        QUrlQuery postData;
        postData.addQueryItem("key", settings->YandexTranslateKey);
        //qWarning("Parser ProductParser translate138!!!");
        postData.addQueryItem("text", product_map["title"].toString());
        postData.addQueryItem("text", product_map["description"].toString());
        postData.addQueryItem("text", product_map["location"].toString());
        postData.addQueryItem("format", "html");
        postData.addQueryItem("lang", abrv);
        //qWarning("Parser ProductParser translate144!!!");
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded");
        //qWarning("Parser ProductParser translate149!!!");
        pt = new QWebEnginePage();
        QWebEngineHttpRequest r(url, QWebEngineHttpRequest::Post);
        r.setHeader(QByteArray("Content-Type"), QByteArray("application/x-www-form-urlencoded"));
        r.setPostData(postData.toString(QUrl::FullyEncoded).toUtf8());
#if defined(QUSEPROXY)
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName("195.133.144.163");
        //qWarning("Parser ProductParser translate155!!!");
        proxy.setPort(3128);
        proxy.setUser("mixajlov");
        proxy.setPassword("evilkiller190813");
        mngr->setProxy(proxy);
#endif
        //qWarning("Parser ProductParser translate161!!!");
        connect(pt, &QWebEnginePage::loadFinished, this, &ProductParser::getTranslate);
        //qWarning("Parser ProductParser translate163!!!");
        pt->load(r);
        //qWarning("Parser ProductParser translate164!!!");
    }
    //emit sendLog("Link parse finished!");
}

void ProductParser::getTranslate()
{
    pt->toPlainText([this](const QString& result) mutable {
        productCreate(result);
    });
}

void ProductParser::productCreate(QString reply)
{
    //qWarning("Parser ProductParser productCreate181!!!");
    //qWarning(reply.toUtf8());
    //qWarning("Parser ProductParser productCreate183!!!");
    QJsonDocument doc = QJsonDocument::fromJson(reply.toUtf8());
    //qWarning("Parser ProductParser productCreate185!!!");
   //get the jsonObject
    QJsonObject jObject = doc.object();
    //qWarning("Parser ProductParser productCreate188!!!");
    QVariantMap translateMap = jObject.toVariantMap();
    if(translateMap["code"].toInt() == 200)
    {
        //qWarning("Parser ProductParser productCreate190!!!");
        QString lang = translateMap["lang"].toString().right(translateMap["lang"].toString().indexOf('-'));
        //qWarning("Parser ProductParser productCreate192!!!");
        QVariantList text = translateMap["text"].toList();
        //qWarning("Parser ProductParser productCreate194!!!");
        product_item.title["title_" + lang] = text.at(0).toString();
        //qWarning("Parser ProductParser productCreate196!!!");
        product_item.content["content_" + lang] = text.at(1).toString();
        //qWarning("Parser ProductParser productCreate198!!!");
        product_item.anons["anons_" + lang] = text.at(1).toString().remove(QRegExp("<[^>]*>")).left(254);
        //qWarning("Parser ProductParser productCreate200!!!");
        product_item.description["description_" + lang] = text.at(1).toString().remove(QRegExp("<[^>]*>")).left(254);
        //qWarning("Parser ProductParser productCreate202!!!");
        product_item.location["location_" + lang] = text.at(2).toString();
        //qWarning("Parser ProductParser productCreate204!!!");
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
            end_time = end_time.remove(QRegExp("^\\s+"));
            end_time = end_time.remove(QRegExp("\\s+$"));
            end_time = end_time.replace(QRegExp("\\s+"), " ");
            end_time = end_time.replace(QRegExp("\\s"), "-");
            row->date_format = row->date_format.replace(QRegExp("\\s"), "-");
            //qWarning(end_time.toLatin1().constData());
            //qWarning(row->date_format.toLatin1().constData());
            QDateTime dt = QDateTime::fromString(end_time, row->date_format);
            uint end_timedt = dt.toTime_t();
            //qWarning(QString::number(end_timedt).toLatin1().constData());
            if(end_timedt > 100000000)
            {
                if(utc_value != 0)
                {
                    end_timedt = end_timedt + 3600;
                }
                end_timedt = end_timedt - 86400;
                if(end_timedt > QDateTime::currentDateTime().toTime_t())
                {
                    //qWarning(QString::number(end_timedt).toLatin1().constData());
                    product_item.end_time = QString::number(end_timedt);
                    //qWarning("Parser ProductParser productCreate255!!!");
                    QSqlQuery productInsert(database);
                    QList<QString> fields_title = product_item.title.keys();
                    QString title_keys = fields_title.join(", ");
                    QList<QString> fields_title_values = product_item.title.values();
                    QString title_values = "";
                    foreach(QString fName, fields_title)
                    {
                        if(title_values == "")
                        {
                            title_values = ":" + fName;
                        }
                        else {
                            title_values = title_values + ", :" + fName;
                        }
                    }
                    title_values.replace("`", "");

                    QList<QString> fields_content = product_item.content.keys();
                    QString content_keys = fields_content.join(", ");
                    QList<QString> fields_content_values = product_item.content.values();
                    QString content_values = "";
                    foreach(QString fName, fields_content)
                    {
                        if(content_values == "")
                        {
                            content_values = ":" + fName;
                        }
                        else {
                            content_values = content_values + ", :" + fName;
                        }
                    }
                    content_values.replace("`", "");

                    QList<QString> fields_anons = product_item.anons.keys();
                    QString anons_keys = fields_anons.join(", ");
                    QList<QString> fields_anons_values = product_item.anons.values();
                    QString anons_values = "";
                    foreach(QString fName, fields_anons)
                    {
                        if(anons_values == "")
                        {
                            anons_values = ":" + fName;
                        }
                        else {
                            anons_values = anons_values + ", :" + fName;
                        }
                    }
                    anons_values.replace("`", "");

                    QList<QString> fields_description = product_item.description.keys();
                    QString description_keys = fields_description.join(", ");
                    QList<QString> fields_description_values = product_item.description.values();
                    QString description_values = "";
                    foreach(QString fName, fields_description)
                    {
                        if(description_values == "")
                        {
                            description_values = ":" + fName;
                        }
                        else {
                            description_values = description_values + ", :" + fName;
                        }
                    }
                    description_values.replace("`", "");

                    QList<QString> fields_location = product_item.location.keys();
                    QString location_keys = fields_location.join(", ");
                    QList<QString> fields_location_values = product_item.location.values();
                    QString location_values = "";
                    foreach(QString fName, fields_location)
                    {
                        if(location_values == "")
                        {
                            location_values = ":" + fName;
                        }
                        else {
                            location_values = location_values + ", :" + fName;
                        }
                    }
                    location_values.replace("`", "");
                    //qWarning(title_values.toLatin1().constData());

                    productInsert.prepare("INSERT INTO products ("+ title_keys + ", " + anons_keys + ", " + content_keys + ", " + description_keys + ", " + location_keys + ", `article`, `category_id`, `price`, `end_time`, `slug`, `ident_name`) "
                                          "VALUES(" + title_values + ", " + anons_values + ", " + content_values + ", " + description_values + ", " + location_values + ", :article_value, :category_id_value, :price_value, :end_time_value, :slug_value, :ident_name_value);");
                    foreach(QString key, product_item.title.keys())
                    {
                        //qWarning(key.toLatin1().constData());
                        QString fn = ":" + key;
                        fn = fn.replace("`", "");
                        //qWarning(key.toLatin1().constData());
                        //qWarning(fn.toLatin1().constData());
                        productInsert.bindValue(fn, product_item.title[key]);
                    }
                    foreach(QString key, product_item.content.keys())
                    {
                        QString fn = ":" + key;
                        fn = fn.replace("`", "");
                        productInsert.bindValue(fn, product_item.content[key]);
                    }
                    foreach(QString key, product_item.anons.keys())
                    {
                        QString fn = ":" + key;
                        fn = fn.replace("`", "");
                        productInsert.bindValue(fn, product_item.anons[key]);
                    }
                    foreach(QString key, product_item.description.keys())
                    {
                        QString fn = ":" + key;
                        fn = fn.replace("`", "");
                        productInsert.bindValue(fn, product_item.description[key]);
                    }
                    foreach(QString key, product_item.location.keys())
                    {
                        QString fn = ":" + key;
                        fn = fn.replace("`", "");
                        productInsert.bindValue(fn, product_item.location[key]);
                    }
                    productInsert.bindValue(":article_value", product_item.article);
                    productInsert.bindValue(":category_id_value", product_item.category_id);
                    productInsert.bindValue(":price_value", product_item.price);
                    productInsert.bindValue(":end_time_value", product_item.end_time);
                    productInsert.bindValue(":slug_value", product_item.slug);
                    productInsert.bindValue(":ident_name_value", product_item.ident_name);
                    if(!productInsert.exec())
                    {
                        QSqlQuery logInsert(database);
                        logInsert.prepare("INSERT INTO service_logs (code,message,link,create_dt) VALUES(:code_value,:message_value,:link_value,:create_dt_value);");
                        logInsert.bindValue(":code_value", 10);
                        logInsert.bindValue(":message_value", productInsert.lastError().text());
                        logInsert.bindValue(":link_value", product_item.ident_name);
                        logInsert.bindValue(":create_dt_value", QDateTime::currentDateTime().toTime_t());
                        logInsert.exec();
                        emit parserEnd(p, "ProductParser");
                    }
                    else
                    {
                        //qWarning(getLastExecutedQuery(productInsert).toLatin1().constData());
                        product_item.id = productInsert.lastInsertId().toInt();
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
                        //qWarning("Parser ProductParser productCreate314!!!");
                        folder_id.setPermissions(permissions);
                        QFile folder_main(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main");
                        folder_main.setPermissions(permissions);
                        QFile folder_additional(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional");
                        folder_additional.setPermissions(permissions);
                        //qWarning("Parser ProductParser productCreate320!!!");
                        uploadPhoto();
                    }
                }
            }
            else {
                QSqlQuery logInsert(database);
                logInsert.prepare("INSERT INTO service_logs (code,message,link,create_dt) VALUES(:code_value,:message_value,:link_value,:create_dt_value);");
                logInsert.bindValue(":code_value", 11);
                logInsert.bindValue(":message_value", "End date of auction not correct!");
                logInsert.bindValue(":link_value", product_item.ident_name);
                logInsert.bindValue(":create_dt_value", QDateTime::currentDateTime().toTime_t());
                logInsert.exec();
                emit parserEnd(p, "ProductParser");
            }
        }
    }
    else
    {
        QJsonDocument doc = QJsonDocument::fromJson(product.toString().toUtf8());
        QJsonObject jObject = doc.object();
        QVariantMap product_map = jObject.toVariantMap();
        QSqlQuery logInsert(database);
        logInsert.prepare("INSERT INTO service_logs (code,message,link,create_dt) VALUES(:code_value,:message_value,:link_value,:create_dt_value);");
        logInsert.bindValue(":code_value", translateMap["code"].toInt());
        logInsert.bindValue(":message_value", translateMap["message"].toString());
        logInsert.bindValue(":link_value", product_map["ident_name"].toString());
        logInsert.bindValue(":create_dt_value", QDateTime::currentDateTime().toTime_t());
        logInsert.exec();
        emit parserEnd(p, "ProductParser");
    }
}

void ProductParser::uploadPhoto()
{
    //qWarning("Parser ProductParser productCreate327!!!");
    //qWarning("Parser ProductParser uploadPhoto!!!");
    QString pkey = product_item.photos.firstKey();
    QString link = product_item.photos[pkey].toString();
    //qWarning("Parser ProductParser productCreate331!!!");
    product_item.photos.remove(pkey);
    //qWarning("Parser ProductParser productCreate333!!!");
    savePhotos(link);
}

void ProductParser::savePhotos(QString l)
{
    //qWarning("Parser ProductParser savePhotos!!!");
    if(main_image)
    {
        //qWarning("Parser ProductParser savePhotos346!!!");
        QPixmap image;
        //qWarning("Parser ProductParser savePhotos348!!!");
        QString link = l;
        //qWarning("Parser ProductParser savePhotos350!!!");
        QString fullfileName = link.right(link.size()-link.lastIndexOf("/")-1);
        //qWarning("Parser ProductParser savePhotos352!!!");
        QString path = settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fullfileName;
        download_image(link.toLocal8Bit().data(), path);
        //qWarning("Parser ProductParser savePhotos354!!!");
        image.load(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/main/" + fullfileName);
        QString fileName = fullfileName.left(fullfileName.indexOf("."));
        QString ext = fullfileName.right(fullfileName.size()-fullfileName.lastIndexOf("."));
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
        int image_id = imageInserter.lastInsertId().toInt();
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
        QString link = l;
        QString fullfileName = link.right(link.size()-link.lastIndexOf("/")-1);
        QString path = settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fullfileName;
        download_image(link.toLocal8Bit().data(), path);
        image.load(settings->AbsUploadPath + "/products/" + QString::number(product_item.id) + "/additional/" + fullfileName);
        QString fileName = fullfileName.left(fullfileName.indexOf("."));
        QString ext = fullfileName.right(fullfileName.size()-fullfileName.lastIndexOf("."));\
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

size_t ProductParser::callbackfunction(void *ptr, size_t size, size_t nmemb, void* userdata)
{
    FILE* stream = (FILE*)userdata;
    if (!stream)
    {
        printf("!!! No stream\n");
        return 0;
    }

    size_t written = fwrite((FILE*)ptr, size, nmemb, stream);
    return written;
}

bool ProductParser::download_image(char *url, QString path)
{
    FILE* fp = fopen(path.toLocal8Bit(), "wb");
    if (!fp)
    {
        printf("!!! Failed to create file on the disk\n");
        return false;
    }

    CURL* curlCtx = curl_easy_init();
    curl_easy_setopt(curlCtx, CURLOPT_URL, url);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curlCtx, CURLOPT_WRITEFUNCTION, callbackfunction);
    curl_easy_setopt(curlCtx, CURLOPT_FOLLOWLOCATION, 1);

    CURLcode rc = curl_easy_perform(curlCtx);
    if (rc)
    {
        printf("!!! Failed to download: %s\n", url);
        return false;
    }

    long res_code = 0;
    curl_easy_getinfo(curlCtx, CURLINFO_RESPONSE_CODE, &res_code);
    if (!((res_code == 200 || res_code == 201) && rc != CURLE_ABORTED_BY_CALLBACK))
    {
        printf("!!! Response code: %d\n", res_code);
        return false;
    }

    curl_easy_cleanup(curlCtx);

    fclose(fp);

    return true;
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
