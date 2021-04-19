#include "cmainviewport.h"

#include <QDebug>
#include <QMouseEvent>

#include <AIS_ViewController.hxx>

#include <V3d_Viewer.hxx>
#include <AIS_InteractiveContext.hxx>
#include <V3d_View.hxx>

#include <TopoDS_Shape.hxx>

//#include <SelectMgr_EntityOwner.hxx>
//#include <StdSelect_BRepOwner.hxx>
//#include <BRepBuilderAPI_Transform.hxx>

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

        myMouseGestureMap.Clear();
        myMouseGestureMap.Bind(Aspect_VKeyMouse_LeftButton, AIS_MouseGesture_Pan);
        myMouseGestureMap.Bind(Aspect_VKeyMouse_RightButton, AIS_MouseGesture_RotateOrbit);

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

    void fitInView() {
        v3dView->FitAll();
        v3dView->ZFitAll();
        v3dView->Redraw();
    }

//    void handleMoveTo(const Handle(AIS_InteractiveContext)& theCtx,
//                      const Handle(V3d_View)& theView) final {

//        TopoDS_Shape shape;
//        for (theCtx->InitDetected(); theCtx->MoreDetected(); theCtx->NextDetected()) {
//            Handle(SelectMgr_EntityOwner) owner = theCtx->DetectedOwner();
//            if (Handle(StdSelect_BRepOwner) brepOwner = Handle(StdSelect_BRepOwner)::DownCast (owner)) {
//                shape = brepOwner->Shape();
//            }
//        }
//        if (!shape.IsNull() && myMousePressed != 0)
//        {
//            qDebug() << "move";
//            gp_Trsf trsf = shape.Location().Transformation();
//            trsf.SetTranslation(gp_Pnt(0.0, 0.0, 0.0), gp_Pnt(1.0, 1.0, 1.0));
//            BRepBuilderAPI_Transform myTrsf(shape, trsf, Standard_False);
//            shape = myTrsf.Shape();
//        }
//        else
//            AIS_ViewController::handleMoveTo(theCtx, theView);
//    }

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

void CMainViewport::fitInView()
{
    d_ptr->fitInView();
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

//! Map Qt buttons bitmask to virtual keys.
inline static Aspect_VKeyMouse qtMouseButtons2VKeys(Qt::MouseButtons theButtons)
{
    Aspect_VKeyMouse aButtons = Aspect_VKeyMouse_NONE;
    if ((theButtons & Qt::LeftButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_LeftButton;
    }
    if ((theButtons & Qt::MiddleButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_MiddleButton;
    }
    if ((theButtons & Qt::RightButton) != 0)
    {
        aButtons |= Aspect_VKeyMouse_RightButton;
    }
    return aButtons;
}

//! Map Qt mouse modifiers bitmask to virtual keys.
inline static Aspect_VKeyFlags qtMouseModifiers2VKeys(Qt::KeyboardModifiers theModifiers)
{
    Aspect_VKeyFlags aFlags = Aspect_VKeyFlags_NONE;
    if ((theModifiers & Qt::ShiftModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_SHIFT;
    }
    if ((theModifiers & Qt::ControlModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_CTRL;
    }
    if ((theModifiers & Qt::AltModifier) != 0)
    {
        aFlags |= Aspect_VKeyFlags_ALT;
    }
    return aFlags;
}

void CMainViewport::mousePressEvent(QMouseEvent *event)
{
    const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false))
        update();
}

void CMainViewport::mouseReleaseEvent(QMouseEvent *event)
{
    const Graphic3d_Vec2i aPnt(event->pos().x(), event->pos().y());
    const Aspect_VKeyFlags aFlags = qtMouseModifiers2VKeys(event->modifiers());
    if (d_ptr->UpdateMouseButtons(aPnt, qtMouseButtons2VKeys(event->buttons()), aFlags, false))
        update();
}

void CMainViewport::mouseMoveEvent(QMouseEvent *event)
{
    const Graphic3d_Vec2i aNewPos(event->pos().x(), event->pos().y());
    if (d_ptr->UpdateMousePosition(aNewPos,
                                   qtMouseButtons2VKeys(event->buttons()),
                                   qtMouseModifiers2VKeys(event->modifiers()),
                                   false))
    {
        update();
    }
}

void CMainViewport::wheelEvent(QWheelEvent *event)
{
    const Graphic3d_Vec2i aPos(event->pos().x(), event->pos().y());
    if (d_ptr->UpdateZoom(Aspect_ScrollDelta(aPos, event->delta() / 8)))
        update();
}
