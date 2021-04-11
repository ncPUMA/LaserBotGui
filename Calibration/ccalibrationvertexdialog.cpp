#include "ccalibrationvertexdialog.h"
#include "ui_ccalibrationvertexdialog.h"

CCalibrationVertexDialog::CCalibrationVertexDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CCalibrationVertexDialog)
{
    ui->setupUi(this);
}

CCalibrationVertexDialog::~CCalibrationVertexDialog()
{
    delete ui;
}

void CCalibrationVertexDialog::setMdlVertex(const double coordX, const double coordY, const double coordZ)
{
    ui->dsbMdlX->setValue(coordX);
    ui->dsbMdlY->setValue(coordY);
    ui->dsbMdlZ->setValue(coordZ);
}

double CCalibrationVertexDialog::getBotVertexX() const
{
    return ui->dsbBotX->value();
}

double CCalibrationVertexDialog::getBotVertexY() const
{
    return ui->dsbBotY->value();
}

double CCalibrationVertexDialog::getBotVertexZ() const
{
    return ui->dsbBotZ->value();
}
