#ifndef PRODUCTPARSER_H
#define PRODUCTPARSER_H

#include <QObject>
#include <QtWebEngineWidgets>

class ProductParser : public QObject
{
    Q_OBJECT
public:
    explicit ProductParser(QObject *parent = nullptr);

signals:

public slots:

private:
    QWebEnginePage *p;
};

#endif // PRODUCTPARSER_H
