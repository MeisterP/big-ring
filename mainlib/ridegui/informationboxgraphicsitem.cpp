#include "informationboxgraphicsitem.h"

#include <QtGui/QFont>
#include <QtGui/QPainter>

InformationBoxGraphicsItem::InformationBoxGraphicsItem(QObject *parent) :
    QObject(parent)
{
    QFont font = QFont("Liberation Mono");
    font.setBold(true);
    font.setPointSize(24);

    _textItem = new QGraphicsTextItem(this);
    _textItem->setFont(font);

    _textItem->setDefaultTextColor(Qt::white);

    _textItem->setPlainText("Empty");
    _textItem->setPos(10, 5);
    _textItem->setOpacity(0.65);

    _pixmapItem = new QGraphicsPixmapItem(this);
    _pixmapItem->setOpacity(0.65);
    _pixmapItem->setPos(10, 0);
    _pixmapItem->hide();
}

QRectF InformationBoxGraphicsItem::boundingRect() const
{
    QRectF internalRect;
    if (_textItem->boundingRect().width() > _pixmapItem->boundingRect().width()) {
        internalRect = _textItem->boundingRect();
    } else {
        internalRect = _pixmapItem->boundingRect();
    }

    return QRectF(0, 0, internalRect.width() + 20, internalRect.height() + 20);
}

void InformationBoxGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(Qt::green);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::black);
    painter->setOpacity(0.65 * opacity());
    painter->drawRoundedRect(0, -10, boundingRect().width(), boundingRect().height(), 3, 3);
}

void InformationBoxGraphicsItem::setInformationBox(const InformationBox &informationBox)
{
    if (informationBox.message().isEmpty()) {
        setImageFileInfo(informationBox.imageFileInfo());
    } else {
        setText(informationBox.message());
    }
}

void InformationBoxGraphicsItem::setImageFileInfo(const QFileInfo &imageFileInfo)
{
    QPixmap pixmap(imageFileInfo.canonicalFilePath());
    _pixmapItem->setPixmap(pixmap);
    _textItem->setHtml("");
    _pixmapItem->show();
    _textItem->hide();
}

void InformationBoxGraphicsItem::setText(const QString &text)
{
    _pixmapItem->setPixmap(QPixmap());
    _textItem->setHtml(text);
    _textItem->show();
    _pixmapItem->hide();
}
