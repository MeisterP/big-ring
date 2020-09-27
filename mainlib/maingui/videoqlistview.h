#ifndef VIDEOQLISTVIEW_H
#define VIDEOQLISTVIEW_H

#include <QListView>

class VideoQListView : public QListView
{
    Q_OBJECT
public:
    explicit VideoQListView(QWidget *parent = 0);

signals:
    void openVideoImportPreferences();

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void mousePressEvent(QMouseEvent *event);
};

#endif // VIDEOQLISTVIEW_H
