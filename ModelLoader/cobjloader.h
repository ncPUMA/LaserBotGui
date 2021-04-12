#ifndef COBJLOADER_H
#define COBJLOADER_H

#include "cabstractmodelloader.h"

class CObjLoader : public CAbstractModelLoader
{
public:
    CObjLoader();

protected:
    NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *fName) final;
};

#endif // COBJLOADER_H
