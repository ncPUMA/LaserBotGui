#ifndef CBREPLOADER_H
#define CBREPLOADER_H

#include "cabstractmodelloader.h"

class CBrepLoader : public CAbstractModelLoader
{
public:
    CBrepLoader();

protected:
    NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *fName) final;
};

#endif // CBREPLOADER_H
