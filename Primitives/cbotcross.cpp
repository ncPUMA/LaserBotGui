#include "cbotcross.h"

#include <AIS_Point.hxx>
#include <Geom_CartesianPoint.hxx>
#include <AIS_TextLabel.hxx>

class CBotCrossPrivate
{
    friend class CBotCross;

    CBotCrossPrivate() {
        gp_Pnt pnt(0., 0., 0.);
        Handle(Geom_CartesianPoint) geomPoint = new Geom_CartesianPoint(pnt);
        point = new AIS_Point(geomPoint);

        txtCoord = new AIS_TextLabel();
        txtCoord->SetText("Coord: 0. 0. 0.\n   Ang: 0. 0. 0.");
        txtCoord->SetPosition(gp_Pnt(pnt.X(), pnt.Y(), pnt.Z()));
    }

    Handle(AIS_Point) point;
    Handle(AIS_TextLabel) txtCoord;
};



CBotCross::CBotCross() :
    d_ptr(new CBotCrossPrivate())
{

}

CBotCross::~CBotCross()
{
    delete d_ptr;
}

NCollection_Vector<Handle (AIS_InteractiveObject)> CBotCross::objects() const
{
    NCollection_Vector<Handle (AIS_InteractiveObject)> result;
    result.Append(d_ptr->point);
    result.Append(d_ptr->txtCoord);
    return result;
}
