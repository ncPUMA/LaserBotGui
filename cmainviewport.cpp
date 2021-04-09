#include "cmainviewport.h"

#include <AIS_ViewController.hxx>

#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>

#include "caspectwindow.h"

class CMainViewportPrivate : public AIS_ViewController
{
    friend class CMainViewport;

    CMainViewportPrivate(CMainViewport * const qptr) :
        q_ptr(qptr)
    { }

    void init(AIS_InteractiveContext &extContext) {
        context = &extContext;
        v3dView = context->CurrentViewer()->CreateView().get();

        SetAllowRotation(Standard_True);
        v3dView->SetBackgroundColor(Quantity_Color(0.3, 0.3, 0.3, Quantity_TOC_RGB));

        aspect = new CAspectWindow(*q_ptr);
        v3dView->SetWindow(aspect);
        if (!aspect->IsMapped())
        {
          aspect->Map();
        }

        v3dView->MustBeResized();

        v3dView->FitAll();
        v3dView->ZFitAll();
        v3dView->Redraw();
    }

    void paintEvent() {
        v3dView->InvalidateImmediate();
        FlushViewEvents(context, v3dView, Standard_True);
    }

    void resizeEvent() {
        v3dView->MustBeResized();
    }

    CMainViewport * const q_ptr;
    Handle(AIS_InteractiveContext) context;
    Handle(V3d_View)               v3dView;
    Handle(CAspectWindow)          aspect;
};



CMainViewport::CMainViewport(QWidget *parent) :
    QWidget(parent),
    d_ptr(new CMainViewportPrivate(this))
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    setBackgroundRole(QPalette::NoRole);
    setFocusPolicy(Qt::StrongFocus);
}

CMainViewport::~CMainViewport()
{
    delete d_ptr;
}

void CMainViewport::init(AIS_InteractiveContext &context)
{
    d_ptr->init(context);
}

QPaintEngine *CMainViewport::paintEngine() const
{
    return nullptr;
}

void CMainViewport::paintEvent(QPaintEvent *)
{
    d_ptr->paintEvent();
}

void CMainViewport::resizeEvent(QResizeEvent *)
{
    d_ptr->resizeEvent();
}
