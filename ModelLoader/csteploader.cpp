#include "csteploader.h"

#include <TopTools_HSequenceOfShape.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>

CStepLoader::CStepLoader() :
    CAbstractModelLoader()
{

}

TopoDS_Shape CStepLoader::loadPrivate(const char *fName)
{
    TopoDS_Shape result;
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

        TopoDS_Compound comp;
        TopoDS_Builder builder;
        builder.MakeCompound(comp);
        int aShapesNumber = aReader.NbShapes();
        for (int i = 1; i <= aShapesNumber; i++)
        {
          TopoDS_Shape aTopoShape = aReader.Shape(i);
          builder.Add(comp, aTopoShape);
        }
        result = comp;
    }
    return result;
}
