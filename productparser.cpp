#include "productparser.h"

ProductParser::ProductParser(QObject *parent) : QObject(parent)
{
    p = new QWebEnginePage();
}
