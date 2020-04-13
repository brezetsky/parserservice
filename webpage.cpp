#include "webpage.h"

WebPage::WebPage(ParserRow *r, QString a, QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    qWarning("wp init!");
    action = a;
    row = r;
    connect(this, &WebPage::loadFinished, this, &WebPage::wLoadFinished);
}

void WebPage::wLoadFinished(bool ok)
{
    qWarning("Loaded from browser!");
    emit wLoadFinishedSignal(ok, this, row, action);
}
