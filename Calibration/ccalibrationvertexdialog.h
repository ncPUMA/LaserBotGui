#ifndef CCALIBRATIONVERTEXDIALOG_H
#define CCALIBRATIONVERTEXDIALOG_H

#include <QDialog>

namespace Ui {
class CCalibrationVertexDialog;
}

class CCalibrationVertexDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CCalibrationVertexDialog(QWidget *parent = nullptr);
    ~CCalibrationVertexDialog();

    void setMdlVertex(const double coordX, const double coordY, const double coordZ);

    double getBotVertexX() const;
    double getBotVertexY() const;
    double getBotVertexZ() const;

private:
    Ui::CCalibrationVertexDialog *ui;
};

#endif // CCALIBRATIONVERTEXDIALOG_H
