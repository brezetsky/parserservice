#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include <QObject>
#include <QWebEnginePage>
#include "parserrow.h"

class PageParser : public QObject
{
    Q_OBJECT
public:
    explicit PageParser(ParserRow *r, QObject *parent = nullptr);

signals:
    void geteedLink(QString link);
    void pageParseEnd(ParserRow *row);
    void parserEnd();

public slots:
    void stop();
    void load();

private:
    QWebEnginePage *p;
    ParserRow *row;
};

#endif // PAGEPARSER_H
