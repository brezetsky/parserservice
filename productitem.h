#ifndef PRODOCTITEM_H
#define PRODOCTITEM_H

#include <QObject>
#include <QMap>

struct ProductItem
{
    int id = 0;
    QString article;
    QString category_id;
    QMap<QString,QString> title;
    QMap<QString,QString> anons;
    QString main_img_id;
    QVariantMap photos;
    QMap<QString,QString> content;
    QString price;
    QMap<QString,QString> location;
    QString end_time;
    QMap<QString,QString> description;
    QString slug;
    QString ident_name;
    QString status;
};

#endif // PRODOCTITEM_H
