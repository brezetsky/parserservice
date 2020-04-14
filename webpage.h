#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <QWebEnginePage>
#include <QWebEngineProfile>
#include "parserrow.h"

class WebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WebPage(ParserRow *r, QString a = "page", QWebEngineProfile *profile = QWebEngineProfile::defaultProfile(), QObject *parent = nullptr);
    void javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID);

private:
    ParserRow *row;
    QString action;

signals:
    void wLoadFinishedSignal(bool ok, WebPage *self, ParserRow *r, QString a);

private slots:
    void wLoadFinished(bool ok);

};

#endif // WEBPAGE_H
