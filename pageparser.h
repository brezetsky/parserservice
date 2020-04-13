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
    void getedLink(QString link);
    void pageParseEnd(ParserRow *row, WebPage *wp);
    void parserEnd(WebPage *wp);

public slots:
    void stop();
    void parse();

private:
    WebPage *page;
    ParserRow *row;
    QString html;
    void runDestructor(const QVariant &v);
};

#endif // PAGEPARSER_H
