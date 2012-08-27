#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

#include <QColor>
#include <QList>
#include <QPair>

#include "ColorWidget.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog(QString server, QWidget* parent = 0);
    ~Dialog();

    theLEDStripControlDescription getControlDescription();
    void setControlDescription(const theLEDStripControlDescription description);
    
protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::Dialog* ui;

    QString server;

    bool initialized;

    int currentColorWidget;
    void setCurrentColorWidget(int currentColorWidget);
    ColorWidget* createColorWidget(const QColor color);

    bool glow;
    int glowSpeed;
    void setGlow(bool glow);

    QList<QPair<QString, QString>> filters;
    int currentFilter;
    void setCurrentFilter(int currentFilter);

    void sendToServer();

private slots:
    void setGlowSpeed(int glowSpeed);
    void setGlowSpeedSeconds(double glowSpeedMs);

    void updated();
};

#endif // DIALOG_H
