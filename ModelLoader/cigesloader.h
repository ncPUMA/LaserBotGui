#ifndef CIGESLOADER_H
#define CIGESLOADER_H

#include "cabstractmodelloader.h"

class CIgesLoader : public CAbstractModelLoader
{
public:
    CIgesLoader();

protected:
    NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *fName) final;
};

#endif // CIGESLOADER_H
