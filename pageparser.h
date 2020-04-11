#ifndef PAGEPARSER_H
#define PAGEPARSER_H

#include <QObject>
#include <QWebEngineView>

class PageParser : public QObject
{
    Q_OBJECT
public:
    explicit PageParser(QObject *parent = nullptr);

signals:

public slots:

private:
    QWebEnginePage *p;
};

#endif // PAGEPARSER_H
