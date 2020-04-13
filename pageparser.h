#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include <QObject>
#include <QWebEnginePage>
#include <QFile>
#include "parserrow.h"

class PageParser : public QObject
{
    Q_OBJECT
public:
    explicit PageParser(ParserRow *r, QObject *parent = nullptr);

signals:
    void getedLink(QString link);
    void pageParseEnd(ParserRow *row);
    void parserEnd();

public slots:
    void stop();
    void load();
    void parse();

private:
    //QWebEnginePage *p;
    ParserRow *row;
    QString html;
};

#endif // PAGEPARSER_H
