#ifndef PRODUCTPARSER_H
#define PRODUCTPARSER_H

#include <QObject>
#include "webpage.h"
#include "parserrow.h"
#include "parsersettings.h"

class ProductParser : public QObject
{
    Q_OBJECT
public:
    explicit ProductParser(ParserRow *r, WebPage *wp, ParserSettings *s, QObject *parent = nullptr);

signals:
    void parserEnd(WebPage *wp, QString sender_name);

public slots:
    void stop();
    void parse();

private:
    WebPage *p;
    ParserRow *row;
    ParserSettings *settings;
};

#endif // PRODUCTPARSER_H
