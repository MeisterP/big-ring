#ifndef INFORMATIONBOXGRAPHICSITEM_H
#define INFORMATIONBOXGRAPHICSITEM_H

#include <QtCore/QFileInfo>
#include <QtCore/QObject>
#include <QtWidgets/QGraphicsItem>

#include "model/reallifevideo.h"

class InformationBoxGraphicsItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(qreal x READ x WRITE setX)
public:
    explicit InformationBoxGraphicsItem(QObject *parent = 0);

    virtual QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
public slots:
    void setInformationBox(const InformationBox &informationBox);
private:
    void setImageFileInfo(const QFileInfo &imageFileInfo);
    void setText(const QString &text);

    QGraphicsPixmapItem *_pixmapItem;
    QGraphicsTextItem *_textItem;

};

#endif // INFORMATIONBOXGRAPHICSITEM_H
