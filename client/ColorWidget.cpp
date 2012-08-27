#include "ColorWidget.h"
#include "ui_ColorWidget.h"

#include <QColorDialog>

ColorWidget::ColorWidget(const QColor color, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ColorWidget)
{
    this->ui->setupUi(this);

    connect(this->ui->rSlider, SIGNAL(valueChanged(int)), this, SLOT(setColorFromSliders()));
    connect(this->ui->gSlider, SIGNAL(valueChanged(int)), this, SLOT(setColorFromSliders()));
    connect(this->ui->bSlider, SIGNAL(valueChanged(int)), this, SLOT(setColorFromSliders()));

    connect(this->ui->button, SIGNAL(clicked()), this, SLOT(pickColorWithDialog()));

    this->setColor(color);
}

ColorWidget::~ColorWidget()
{
    delete ui;
}

QColor ColorWidget::getColor() const
{
    return this->color;
}

void ColorWidget::setColor(const QColor color)
{
    this->color = color;

    this->ui->rSlider->blockSignals(true);
    this->ui->rSlider->setValue(this->color.red());
    this->ui->rSlider->blockSignals(false);
    this->ui->gSlider->blockSignals(true);
    this->ui->gSlider->setValue(this->color.green());
    this->ui->gSlider->blockSignals(false);
    this->ui->bSlider->blockSignals(true);
    this->ui->bSlider->setValue(this->color.blue());
    this->ui->bSlider->blockSignals(false);

    this->ui->button->setStyleSheet("background: " + this->color.name() + "; border: 0;");

    emit this->colorChanged(this->color);
}

void ColorWidget::setColorFromSliders()
{
    this->setColor(QColor(this->ui->rSlider->value(),
                          this->ui->gSlider->value(),
                          this->ui->bSlider->value()));
}

void ColorWidget::pickColorWithDialog()
{
    QColor dialogColor = QColorDialog::getColor(this->color, this);
    if (dialogColor.isValid())
    {
        this->setColor(dialogColor);
    }
}
