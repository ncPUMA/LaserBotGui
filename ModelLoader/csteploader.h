#ifndef CSTEPLOADER_H
#define CSTEPLOADER_H

#include "cabstractmodelloader.h"

class CStepLoader : public CAbstractModelLoader
{
public:
    CStepLoader();

protected:
    NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *fName) final;
};

#endif // CSTEPLOADER_H
