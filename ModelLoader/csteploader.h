#ifndef CSTEPLOADER_H
#define CSTEPLOADER_H

#include "cabstractmodelloader.h"

class CStepLoader : public CAbstractModelLoader
{
public:
    CStepLoader();

protected:
    TopoDS_Shape loadPrivate(const char *fName) final;
};

#endif // CSTEPLOADER_H
