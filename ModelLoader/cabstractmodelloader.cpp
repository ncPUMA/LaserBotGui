#include "cabstractmodelloader.h"

CAbstractModelLoader::CAbstractModelLoader()
{

}

CAbstractModelLoader::~CAbstractModelLoader()
{

}

NCollection_Vector<Handle (AIS_InteractiveObject)> CAbstractModelLoader::load(const QString &fName)
{
    return loadPrivate(fName.toLocal8Bit().constData());
}
