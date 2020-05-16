#ifndef PRODUCTPARSER_H
#define PRODUCTPARSER_H

#include <QObject>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QSqlDatabase>
#include <QMap>
#include <QList>
#include <QDir>
#include <QMimeDatabase>
#include <QUrlQuery>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QList>
#include <QNetworkRequest>
#include <QWebEnginePage>
#include <curl/curl.h>
#include <stdio.h>
#include <QNetworkReply>
#if defined(QUSEPROXY)
#include <QNetworkProxy>
#endif
#include "webpage.h"
#include "parserrow.h"
#include "parsersettings.h"
#include "productitem.h"

class ProductParser : public QObject
{
    Q_OBJECT
public:
    explicit ProductParser(ParserRow *r, WebPage *wp, QSqlDatabase& db, ParserSettings *s, QObject *parent = nullptr);

signals:
    void parserEnd(WebPage *wp, QString sender_name);
    void sendLog(QString logMessage);

public slots:
    void stop();
    void parse();

private slots:
    void savePhotos(QString l);
    void getTranslate();

private:
    static size_t callbackfunction(void *ptr, size_t size, size_t nmemb, void* userdata);
    bool download_image(char* url, QString path);
    void productCreate(QString reply);
    WebPage *p;
    QWebEnginePage *pt;
    QWebEnginePage *pi;
    ParserRow *row;
    ParserSettings *settings;
    QVariant product;
    void translate();
    QSqlDatabase database;
    QMap<QString,QString> lang_fields;
    QList<QString> languages;
    ProductItem product_item;
    QString getLastExecutedQuery(const QSqlQuery& query);
    void uploadPhoto();
    bool main_image = true;

};

#endif // PRODUCTPARSER_H
