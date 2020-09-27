#include "videoqlistview.h"
#include <QtWidgets/QApplication>
#include <QtGui/QPainter>
VideoQListView::VideoQListView(QWidget *parent) :
    QListView(parent)
{
}

void VideoQListView::paintEvent(QPaintEvent *event)
{
    QListView::paintEvent(event);
    if (model() && model()->rowCount() > 0) {
        return;
    }
    QPainter p(this->viewport());
    p.drawText(viewport()->rect(), Qt::AlignCenter, "No Videos in List");
}

void VideoQListView::mousePressEvent(QMouseEvent *event)
{
    if (model() && model()->rowCount() > 0) {
        QListView::mousePressEvent(event);
    }
    emit openVideoImportPreferences();
}
