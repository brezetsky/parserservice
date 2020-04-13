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
    void parse();
    void manage_links(QString link);
    void manage_category_page(ParserRow *row);
    void manage_parsers();
    void threadFinished();

private:
    bool disabled = false;
    QTimer *parseTimer;
    QSqlDatabase sitedb;
    qint64 parserOffset = 0;
    ParserSettings *settings;
    QList<QThread*> workers;
    QList<ParserRow*> categoryPages;
    QList<QString> productLinks;
};

#endif // PARSERMAIN_H
