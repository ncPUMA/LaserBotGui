#include "cbreploader.h"

#include <TopoDS_Shape.hxx>
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <AIS_Shape.hxx>

CBrepLoader::CBrepLoader() :
    CAbstractModelLoader()
{

}

NCollection_Vector <Handle(AIS_InteractiveObject)> CBrepLoader::loadPrivate(const char *fName)
{
    TopoDS_Shape aTopoShape;
    BRep_Builder aBuilder;
    Standard_Boolean aResult = BRepTools::Read(aTopoShape, fName , aBuilder);
    NCollection_Vector <Handle(AIS_InteractiveObject)> result;
    if (aResult)
        result.Append(new AIS_Shape(aTopoShape));
    return result;
}
