#ifndef CABSTRACTMODELLOADER_H
#define CABSTRACTMODELLOADER_H

#include <QString>

#include <NCollection_Vector.hxx>

class AIS_InteractiveObject;

class CAbstractModelLoader
{
public:
    CAbstractModelLoader();
    virtual ~CAbstractModelLoader();

    NCollection_Vector <Handle(AIS_InteractiveObject)> load(const QString &fName);

protected:
    virtual NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *fName) = 0;

private:
    CAbstractModelLoader(const CAbstractModelLoader &) = delete;
    CAbstractModelLoader& operator=(const CAbstractModelLoader &) = delete;
};

#endif // CABSTRACTMODELLOADER_H
