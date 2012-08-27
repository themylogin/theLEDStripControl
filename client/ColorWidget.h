#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QFrame>

#include <QColor>

#include "../shared/theLEDStripControlDescription.hpp"

namespace Ui {
class ColorWidget;
}

class ColorWidget : public QFrame
{
    Q_OBJECT
    
public:
    explicit ColorWidget(const QColor color, QWidget* parent = 0);
    ~ColorWidget();

    QColor getColor() const;
    void setColor(const QColor color);
    
private:
    Ui::ColorWidget* ui;

    QColor color;

private slots:
    void setColorFromSliders();
    void pickColorWithDialog();

signals:
    void colorChanged(const QColor color);
};

#endif // COLORWIDGET_H
