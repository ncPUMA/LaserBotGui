#include "csteploader.h"

#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <AIS_Shape.hxx>

CStepLoader::CStepLoader() :
    CAbstractModelLoader()
{

}

NCollection_Vector<Handle(AIS_InteractiveObject)> CStepLoader::loadPrivate(const char *fName)
{
    NCollection_Vector <Handle(AIS_InteractiveObject)> result;
//    Handle(TopTools_HSequenceOfShape) aSequence = new TopTools_HSequenceOfShape();
    STEPControl_Reader aReader;
    const IFSelect_ReturnStatus aStatus = aReader.ReadFile(fName);
    if (aStatus == IFSelect_RetDone)
    {
        bool anIsFailsOnly = false;
        aReader.PrintCheckLoad(anIsFailsOnly, IFSelect_ItemsByEntity);

        int aRootsNumber = aReader.NbRootsForTransfer();
        aReader.PrintCheckTransfer(anIsFailsOnly, IFSelect_ItemsByEntity);
        for (Standard_Integer i = 1; i <= aRootsNumber; i++)
        {
          aReader.TransferRoot(i);
        }

        int aShapesNumber = aReader.NbShapes();
        for (int i = 1; i <= aShapesNumber; i++)
        {
          TopoDS_Shape aTopoShape = aReader.Shape(i);
          Handle(AIS_Shape) shape = new AIS_Shape(aTopoShape);
          result.Append(shape);
        }
    }
    return result;
}
