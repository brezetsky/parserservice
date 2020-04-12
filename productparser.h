#ifndef PRODUCTPARSER_H
#define PRODUCTPARSER_H

#include <QObject>
#include <QtWebEngineWidgets>
#include "parserrow.h"

class ProductParser : public QObject
{
    Q_OBJECT
public:
    explicit ProductParser(ParserRow *row, QString link, QObject *parent = nullptr);

signals:

public slots:

private:
    QWebEnginePage *p;
};

#endif // PRODUCTPARSER_H
