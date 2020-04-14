#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include <QObject>
#include <QFile>
#include "parserrow.h"
#include <webpage.h>

class PageParser : public QObject
{
    Q_OBJECT
public:
    explicit PageParser(ParserRow *r, WebPage *p, QObject *parent = nullptr);

signals:
    void getedLink(QString link, ParserRow *r);
    void pageParseEnd(ParserRow *row, WebPage *wp);
    void parserEnd(WebPage *wp, QString sender_name);

public slots:
    void parse();
    void stop();

private:
    WebPage *page;
    ParserRow *row;
    QString html;
    void runDestructor(const QVariant &v);
};

#endif // PAGEPARSER_H
