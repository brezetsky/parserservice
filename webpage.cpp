#include "webpage.h"

WebPage::WebPage(ParserRow *r, QString a, QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    //qWarning("wp init!");
    action = a;
    row = r;
    connect(this, &WebPage::loadFinished, this, &WebPage::wLoadFinished);
}

void WebPage::wLoadFinished(bool ok)
{
    //qWarning("Loaded from browser!");
    emit wLoadFinishedSignal(ok, this, row, action);
}

void WebPage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString &message, int lineNumber, const QString &sourceID)
{
    Q_UNUSED(level);
    Q_UNUSED(message);
    Q_UNUSED(lineNumber);
    Q_UNUSED(sourceID);
}
