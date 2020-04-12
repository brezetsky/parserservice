#ifndef PARSERROW_H
#define PARSERROW_H

#include <QObject>
#include <QVariantMap>

struct ParserRow
{
    qint64 id;
    qint64 category_id;
    QString category_url;
    QString item_selector;
    QString next_page_selector;
    QString article_selector;
    QString title_selector;
    QString photo_selector;
    QString price_selector;
    QString description_selector;
    QString location_selector;
    QString location_etalon;
    QString location_full_selector;
    float logistic_price;
    QString end_time_selector;
    qint8 publicate_status;
    QVariantMap additional_fields;
    QString price_formula;
    qint64 date_create;
    qint64 date_last_parse;
};

#endif // PARSERROW_H
