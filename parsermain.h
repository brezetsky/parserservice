#ifndef PARSERMAIN_H
#define PARSERMAIN_H

#include <QObject>
#include <QTimer>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QThread>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include "parsersettings.h"
#include "parserrow.h"
#include "productparser.h"
#include "pageparser.h"
#include "webpage.h"
#include "linkobject.h"


class ParserMain : public QObject
{
    Q_OBJECT
public:
    explicit ParserMain(ParserSettings *s, QObject *parent = nullptr);
    ~ParserMain();
    void pause();
    void resume();

signals:
    void stopAllThread();

public slots:
    void load();
    void manage_links(QString link, ParserRow *r);
    void manage_category_page(ParserRow *row, WebPage *wp);
    void manage_parsers(WebPage *w, QString sender_name);
    void threadFinished();
    void wpFinished(bool ok, WebPage *wp, ParserRow *r, QString a);

private:
    bool disabled = false;
    QTimer *parseTimer;
    QSqlDatabase sitedb;
    qint64 parserOffset = 0;
    ParserSettings *settings;
    qint64 workers_count = 0;
    QList<ParserRow*> categoryPages;
    QList<LinkObject*> productLinks;
    void loadPage(QString action);
    qint64 pageActiveLoaderCount = 0;
    void loadDbData();
};

#endif // PARSERMAIN_H
