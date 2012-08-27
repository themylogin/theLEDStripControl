#include "Dialog.h"
#include "ui_Dialog.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QRadioButton>
#include <QtNetwork/QTcpSocket>

#include <algorithm>
using std::min;
using std::max;
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <fstream>
#include <sstream>

Dialog::Dialog(QString server, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    this->initialized = false;

    this->filters.append(QPair<QString, QString>("none", tr("Just shine")));
    this->filters.append(QPair<QString, QString>("music-vu", tr("Blink to music")));

    this->ui->setupUi(this);
    this->setFocus();

    this->server = server;

    this->ui->colorsLayout->addWidget(this->createColorWidget(QColor(255, 0, 0)));
    this->setCurrentColorWidget(0);

    this->setGlow(true);
    this->setGlowSpeed(5000);
    connect(this->ui->glowSpeedSlider, SIGNAL(valueChanged(int)), this, SLOT(setGlowSpeed(int)));
    connect(this->ui->glowSpeedSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setGlowSpeedSeconds(double)));

    for (int i = 0; i < this->filters.length(); i++)
    {
        QRadioButton* filterButton = new QRadioButton(this->filters[i].second, this);
        filterButton->setFocusPolicy(Qt::NoFocus);

        this->ui->filtersLayout->addWidget(filterButton);
    }
    this->setCurrentFilter(0);

    this->initialized = true;
}

Dialog::~Dialog()
{
    delete ui;
}

theLEDStripControlDescription Dialog::getControlDescription()
{
    theLEDStripControlDescription description;
    if (this->glow)
    {
        for (int i = 0; i < this->ui->colorsLayout->count(); i++)
        {
            QColor color = dynamic_cast<ColorWidget*>(this->ui->colorsLayout->itemAt(i)->widget())->getColor();
            description.colors.push_back({ (uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue() });
        }
        description.glowSpeed = this->glowSpeed;
    }
    else
    {
        QColor color = dynamic_cast<ColorWidget*>(this->ui->colorsLayout->itemAt(this->currentColorWidget)->widget())->getColor();
        description.colors.push_back({ (uint8_t)color.red(), (uint8_t)color.green(), (uint8_t)color.blue() });
        description.glowSpeed = 0;
    }
    description.filter = this->filters[this->currentFilter].first.toStdString();
    return description;
}

void Dialog::setControlDescription(const theLEDStripControlDescription description)
{
    this->initialized = false;

    for (int i = this->ui->colorsLayout->count() - 1; i >= 0; i--)
    {
        ColorWidget* colorWidget = dynamic_cast<ColorWidget*>(this->ui->colorsLayout->itemAt(i)->widget());
        this->ui->colorsLayout->removeWidget(colorWidget);
        delete colorWidget;
    }
    for (unsigned int i = 0; i < description.colors.size(); i++)
    {
        this->ui->colorsLayout->addWidget(this->createColorWidget(QColor(description.colors[i].r,
                                                                         description.colors[i].g,
                                                                         description.colors[i].b)));
    }
    this->setCurrentColorWidget(0);

    this->setGlow(true);
    this->setGlowSpeed(description.glowSpeed);

    this->currentFilter = 0;
    for (int i = 0; i < this->ui->filtersLayout->count(); i++)
    {
        if (this->filters[i].first.toStdString() == description.filter)
        {
            this->setCurrentFilter(i);
            break;
        }
    }

    this->initialized = true;
}

void Dialog::keyPressEvent(QKeyEvent* event)
{
    ColorWidget* currentColorWidget = dynamic_cast<ColorWidget*>(this->ui->colorsLayout->itemAt(this->currentColorWidget)->widget());

    int colorChange;
    QColor color;

    int glowSpeedChange;

    int filter;

    QString filename;

    switch (event->key())
    {
    case Qt::Key_Insert:
        this->ui->colorsLayout->insertWidget(this->currentColorWidget + 1, this->createColorWidget(QColor(255, 0, 0)));
        this->setCurrentColorWidget(this->currentColorWidget + 1);
        if (this->glow)
        {
            this->updated();
        }
        break;
    case Qt::Key_Delete:
        if (this->ui->colorsLayout->count() > 1)
        {
            this->ui->colorsLayout->removeWidget(currentColorWidget);
            delete currentColorWidget;

            this->setCurrentColorWidget((this->ui->colorsLayout->count() + this->currentColorWidget - 1) % this->ui->colorsLayout->count());
            if (this->glow)
            {
                this->updated();
            }
        }
        break;

    case Qt::Key_Left:
        this->setCurrentColorWidget((this->ui->colorsLayout->count() + this->currentColorWidget - 1) % this->ui->colorsLayout->count());
        break;
    case Qt::Key_Right:
        this->setCurrentColorWidget((this->currentColorWidget + 1) % this->ui->colorsLayout->count());
        break;

    case Qt::Key_E:
    case Qt::Key_R:
    case Qt::Key_F:
    case Qt::Key_G:
    case Qt::Key_V:
    case Qt::Key_B:
        color = currentColorWidget->getColor();

        colorChange = 5;
        if (event->modifiers() & Qt::ControlModifier)
        {
            colorChange = 1;
        }
        if (event->modifiers() & Qt::ShiftModifier)
        {
            colorChange = 15;
        }

        switch (event->key())
        {
        case Qt::Key_E:
            color.setRed(max(color.red() - colorChange, 0));
            break;
        case Qt::Key_R:
            color.setRed(min(color.red() + colorChange, 255));
            break;
        case Qt::Key_F:
            color.setGreen(max(color.green() - colorChange, 0));
            break;
        case Qt::Key_G:
            color.setGreen(min(color.green() + colorChange, 255));
            break;
        case Qt::Key_V:
            color.setBlue(max(color.blue() - colorChange, 0));
            break;
        case Qt::Key_B:
            color.setBlue(min(color.blue() + colorChange, 255));
            break;
        }

        currentColorWidget->setColor(color);
        break;

    case Qt::Key_Space:
        this->setGlow(!this->glow);
        break;

    case Qt::Key_Up:
    case Qt::Key_Down:
        if (this->glow)
        {
            glowSpeedChange = 1000;
            if (event->modifiers() & Qt::ControlModifier)
            {
                glowSpeedChange = 100;
            }
            if (event->modifiers() & Qt::ShiftModifier)
            {
                glowSpeedChange = 10000;
            }

            switch (event->key())
            {
            case Qt::Key_Up:
                this->setGlowSpeed(min(this->glowSpeed + glowSpeedChange, (int)(this->ui->glowSpeedSpinBox->maximum() * 1000)));
                break;
            case Qt::Key_Down:
                this->setGlowSpeed(max(this->glowSpeed - glowSpeedChange, (int)(this->ui->glowSpeedSpinBox->minimum() * 1000)));
                break;
            }
        }
        break;

    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        filter = event->key() - Qt::Key_1;
        if (filter < this->filters.length())
        {
            this->setCurrentFilter(filter);
        }
        break;

    case Qt::Key_O:
        filename = QFileDialog::getOpenFileName(this, tr("Open LED Strip Control Description"), "", tr("LED Strip Control Description (*.lsd)"));
        if (filename != "")
        {
            std::ifstream ifs(filename.toStdString());
            theLEDStripControlDescription description;
            boost::archive::text_iarchive(ifs) >> description;
            this->setControlDescription(description);
            this->updated();
        }
        break;
    case Qt::Key_S:
        filename = QFileDialog::getSaveFileName(this, tr("Save LED Strip Control Description"), "untitled.lsd", tr("LED Strip Control Description (*.lsd)"));
        if (filename != "")
        {
            std::ofstream ofs(filename.toStdString());
            theLEDStripControlDescription description = this->getControlDescription();
            boost::archive::text_oarchive(ofs) << description;
        }
        break;
    }
}

void Dialog::setCurrentColorWidget(int currentColorWidget)
{
    this->currentColorWidget = currentColorWidget;
    for (int i = 0; i < this->ui->colorsLayout->count(); i++)
    {
        ColorWidget* colorWidget = dynamic_cast<ColorWidget*>(this->ui->colorsLayout->itemAt(i)->widget());
        if (i == this->currentColorWidget)
        {
            colorWidget->setStyleSheet("QFrame { border: 1px solid #888a85; }");
        }
        else
        {
            colorWidget->setStyleSheet("QFrame { border: 1px solid transparent; }");
        }
    }

    if (!this->glow)
    {
        this->updated();
    }
}

ColorWidget* Dialog::createColorWidget(const QColor color)
{
    ColorWidget* colorWidget = new ColorWidget(color, this);
    connect(colorWidget, SIGNAL(colorChanged(QColor)), this, SLOT(updated()));
    return colorWidget;
}

void Dialog::setGlow(bool glow)
{
    this->glow = glow;

    this->ui->glowSpeedSlider->setEnabled(this->glow);
    this->ui->glowSpeedSpinBox->setEnabled(this->glow);

    this->updated();
}

void Dialog::setCurrentFilter(int currentFilter)
{
    this->currentFilter = currentFilter;
    for (int i = 0; i < this->ui->filtersLayout->count(); i++)
    {
        dynamic_cast<QRadioButton*>(this->ui->filtersLayout->itemAt(i)->widget())->setChecked(i == this->currentFilter);
    }

    this->updated();
}

void Dialog::setGlowSpeed(int glowSpeed)
{
    this->glowSpeed = glowSpeed;

    this->ui->glowSpeedSlider->blockSignals(true);
    this->ui->glowSpeedSlider->setValue(this->glowSpeed);
    this->ui->glowSpeedSlider->blockSignals(false);

    this->ui->glowSpeedSpinBox->blockSignals(true);
    this->ui->glowSpeedSpinBox->setValue(this->glowSpeed / 1000.0);
    this->ui->glowSpeedSpinBox->blockSignals(false);

    this->updated();
}

void Dialog::setGlowSpeedSeconds(double glowSpeedSeconds)
{
    this->setGlowSpeed(glowSpeedSeconds * 1000);
}

void Dialog::updated()
{
    if (this->initialized)
    {
        std::ostringstream oss;
        theLEDStripControlDescription description = this->getControlDescription();
        boost::archive::text_oarchive(oss) << description;

        QTcpSocket* socket = new QTcpSocket(this);
        socket->connectToHost(this->server, 4000);
        socket->waitForConnected(1000);
        if (socket->state() != QAbstractSocket::UnconnectedState)
        {
            socket->write(oss.str().data());
            if (socket->state() != QAbstractSocket::UnconnectedState)
            {
                socket->waitForBytesWritten(1000);
                socket->disconnectFromHost();
                if (socket->state() != QAbstractSocket::UnconnectedState)
                {
                    socket->waitForDisconnected(1000);
                }
            }
        }
        delete socket;
    }
}
