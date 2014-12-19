#ifndef VIDEOTILE_H
#define VIDEOTILE_H

#include <QObject>
#include <QtWidgets/QGraphicsLayoutItem>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsWidget>
#include "reallifevideo.h"

class Thumbnailer;
class VideoTile : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit VideoTile(const RealLifeVideo rlv, QGraphicsItem *parent = 0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
signals:
    void selected(const RealLifeVideo& video);
private slots:
    void thumbnailUpdated(QPixmap updatedPixmap);
private:
    QGraphicsItem *addFlag();
    QGraphicsPixmapItem *addThumbnail();
    const RealLifeVideo _rlv;
    QGraphicsPixmapItem* _thumbnailItem;
    Thumbnailer* _thumbnailer;
    bool _selected;
};

#endif // VIDEOTILE_H
