#include "caspectwindow.h"

#include <QWidget>

static const int RESIZE_PRECISION = 2;

class CAspectWindowPrivate
{
    friend class CAspectWindow;

    CAspectWindowPrivate(QWidget &view) :
        wdgt(&view),
        oldGeometry(view.rect()) { }

    QWidget * const wdgt;
    QRect oldGeometry;
};



CAspectWindow::CAspectWindow(QWidget &view) :
    Aspect_Window(),
    d_ptr(new CAspectWindowPrivate(view))
{
    SetBackground(Quantity_NOC_WHITE);
}

CAspectWindow::~CAspectWindow()
{
    delete d_ptr;
}

Aspect_Drawable CAspectWindow::NativeHandle() const
{
    return static_cast <Aspect_Drawable> (d_ptr->wdgt->winId());
}

Aspect_Drawable CAspectWindow::NativeParentHandle() const
{
    WId parentId = 0;
    const QWidget * const parent = d_ptr->wdgt->parentWidget();
    if (parent)
        parentId = parent->winId();
    return static_cast <Aspect_Drawable> (parentId);
}

Aspect_TypeOfResize CAspectWindow::DoResize()
{
    Aspect_TypeOfResize result = Aspect_TOR_UNKNOWN;
    if (!d_ptr->wdgt->isMinimized())
    {
        enum EN_ResizedSideFlags
        {
            ENRSF_NO     = 0x00,
            ENRSF_LEFT   = 0x01,
            ENRSF_RIGHT  = 0x02,
            ENRSF_TOP    = 0x04,
            ENRSF_BOTTOM = 0x08,

            ENRSF_TOP_LEFT     = ENRSF_LEFT  + ENRSF_TOP,
            ENRSF_TOP_RIGHT    = ENRSF_RIGHT + ENRSF_TOP,
            ENRSF_BOTTOM_LEFT  = ENRSF_LEFT  + ENRSF_BOTTOM,
            ENRSF_BOTTOM_RIGHT = ENRSF_RIGHT + ENRSF_BOTTOM
        };
        typedef int TResizedSideFlag;

        TResizedSideFlag sideFlag = ENRSF_NO;
        const QRect newGeometry = d_ptr->wdgt->geometry();
        if (std::abs(newGeometry.left() - d_ptr->oldGeometry.left()) > RESIZE_PRECISION)
            sideFlag += ENRSF_LEFT;
        if (std::abs(newGeometry.right() - d_ptr->oldGeometry.right()) > RESIZE_PRECISION)
            sideFlag += ENRSF_RIGHT;
        if (std::abs(newGeometry.top() - d_ptr->oldGeometry.top()) > RESIZE_PRECISION)
            sideFlag += ENRSF_TOP;
        if (std::abs(newGeometry.bottom() - d_ptr->oldGeometry.bottom()) > RESIZE_PRECISION)
            sideFlag += ENRSF_BOTTOM;

        d_ptr->oldGeometry = newGeometry;

        const std::map <TResizedSideFlag, Aspect_TypeOfResize> mapResizeTypes = {
            { ENRSF_NO          , Aspect_TOR_NO_BORDER               },
            { ENRSF_LEFT        , Aspect_TOR_LEFT_BORDER             },
            { ENRSF_RIGHT       , Aspect_TOR_RIGHT_BORDER            },
            { ENRSF_TOP         , Aspect_TOR_TOP_BORDER              },
            { ENRSF_BOTTOM      , Aspect_TOR_BOTTOM_BORDER           },
            { ENRSF_TOP_LEFT    , Aspect_TOR_LEFT_AND_TOP_BORDER     },
            { ENRSF_TOP_RIGHT   , Aspect_TOR_TOP_AND_RIGHT_BORDER    },
            { ENRSF_BOTTOM_LEFT , Aspect_TOR_BOTTOM_AND_LEFT_BORDER  },
            { ENRSF_BOTTOM_RIGHT, Aspect_TOR_RIGHT_AND_BOTTOM_BORDER }
        };
        auto it = mapResizeTypes.find(sideFlag);
        if (it != mapResizeTypes.cend())
            result = it->second;
    }
    return result;
}

Standard_Boolean CAspectWindow::IsMapped() const
{
    return !(d_ptr->wdgt->isMinimized() || d_ptr->wdgt->isHidden());
}

Standard_Boolean CAspectWindow::DoMapping() const
{
    return Standard_True;
}

void CAspectWindow::Map() const
{
    d_ptr->wdgt->show();
    d_ptr->wdgt->update();
}

void CAspectWindow::Unmap() const
{
    d_ptr->wdgt->hide();
    d_ptr->wdgt->update();
}

void CAspectWindow::Position(Standard_Integer &theX1, Standard_Integer &theY1,
                             Standard_Integer &theX2, Standard_Integer &theY2) const
{
    const QRect geometry = d_ptr->wdgt->geometry();
    theX1 = geometry.left();
    theX2 = geometry.right();
    theY1 = geometry.top();
    theY2 = geometry.bottom();
}

Standard_Real CAspectWindow::Ratio() const
{
    const QRectF geometry = d_ptr->wdgt->geometry();
    return static_cast <Standard_Real>
            ((geometry.right() - geometry.left()) /
             (geometry.bottom() - geometry.top()));
}

void CAspectWindow::Size(Standard_Integer &theWidth, Standard_Integer &theHeight) const
{
    const QRect geometry = d_ptr->wdgt->geometry();
    theWidth  = geometry.width();
    theHeight = geometry.height();
}

Aspect_FBConfig CAspectWindow::NativeFBConfig() const
{
    return nullptr;
}
