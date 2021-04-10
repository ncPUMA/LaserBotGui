#ifndef CMAINVIEWPORT_H
#define CMAINVIEWPORT_H

#include <QWidget>

class CMainViewportPrivate;
class AIS_InteractiveContext;

class CMainViewport : public QWidget
{
    Q_OBJECT
public:
    explicit CMainViewport(QWidget *parent = nullptr);
    ~CMainViewport();

    void init(AIS_InteractiveContext &context);

    void fitInView();

protected:
    QPaintEngine* paintEngine() const final;
    void paintEvent(QPaintEvent *) final;
    void resizeEvent(QResizeEvent*) final;

private:
    CMainViewportPrivate * const d_ptr;
};

#endif // CMAINVIEWPORT_H
