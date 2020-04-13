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
#include <webpage.h>


class ParserMain : public QObject
{
    Q_OBJECT
public:
    explicit ParserMain(ParserSettings *s, QObject *parent = nullptr);
    void pause();
    void resume();

signals:
    void stopAllThread();

public slots:
    void load();
    void manage_links(QString link);
    void manage_category_page(ParserRow *row, WebPage *wp);
    void manage_parsers(WebPage *w);
    void threadFinished();
    void wpFinished(bool ok, WebPage *wp, ParserRow *r, QString a);

private:
    bool disabled = false;
    QTimer *parseTimer;
    QSqlDatabase sitedb;
    qint64 parserOffset = 0;
    ParserSettings *settings;
    qint64 workers_count;
    QList<ParserRow*> categoryPages;
    QList<QString> productLinks;
    void loadPage(QString action);
};

#endif // PARSERMAIN_H
