#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include <QObject>
#include <QWebEngineView>
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
    QWebEngineView *p;
    ParserRow *row;
    QString html;
    void runDestructor(const QVariant &v);
};

#endif // PAGEPARSER_H
