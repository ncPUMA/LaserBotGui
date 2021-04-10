#include "cigesloader.h"

#include <IGESControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <AIS_Shape.hxx>

CIgesLoader::CIgesLoader() :
    CAbstractModelLoader()
{

}

NCollection_Vector<Handle (AIS_InteractiveObject)> CIgesLoader::loadPrivate(const char *fName)
{
    NCollection_Vector <Handle (AIS_InteractiveObject)> result;
    IGESControl_Reader aReader;
    int status = aReader.ReadFile (fName);
    if (status == IFSelect_RetDone)
    {
        aReader.TransferRoots();
        TopoDS_Shape aTopoShape = aReader.OneShape();
        Handle(AIS_Shape) shape = new AIS_Shape (aTopoShape);
        result.Append(shape);
    }
    return result;
}
