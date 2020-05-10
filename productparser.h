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
#include <QUrlQuery>
#include <QDateTime>
#include <QSqlQuery>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
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
    void productCreate(QNetworkReply *reply);

private:
    WebPage *p;
    ParserRow *row;
    ParserSettings *settings;
    QVariant product;
    void translate();
    QSqlDatabase database;
    QMap<QString,QString> lang_fields;
    QList<QString> languages;
    ProductItem product_item;
};

#endif // PRODUCTPARSER_H
