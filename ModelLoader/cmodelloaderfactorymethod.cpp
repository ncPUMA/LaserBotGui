#include "cmodelloaderfactorymethod.h"

#include <map>

#include <QStringList>

#include "cbreploader.h"
#include "csteploader.h"

class CEmptyModelLoader : public CAbstractModelLoader
{
public:
    CEmptyModelLoader() : CAbstractModelLoader() { }

protected:
    NCollection_Vector <Handle(AIS_InteractiveObject)> loadPrivate(const char *) final {
        return NCollection_Vector <Handle(AIS_InteractiveObject)> ();
    }
};



class CModelLoaderFactoryMethodPrivate
{
    friend class CModelLoaderFactoryMethod;

    CModelLoaderFactoryMethodPrivate() {
        mapLoaders["BREP (*.brep)"] = new CBrepLoader();
        mapLoaders["STEP (*.step)"] = new CStepLoader();
    }

    ~CModelLoaderFactoryMethodPrivate() {
        for(auto pair : mapLoaders)
            delete pair.second;
    }

    CAbstractModelLoader& loader(const QString &filter) {
        CAbstractModelLoader * result = &emptyLoader;
        auto it = mapLoaders.find(filter);
        if (it != mapLoaders.cend())
            result = it->second;
        return *result;
    }

    std::map <QString, CAbstractModelLoader *> mapLoaders;
    CEmptyModelLoader emptyLoader;
};



CModelLoaderFactoryMethod::CModelLoaderFactoryMethod() :
    d_ptr(new CModelLoaderFactoryMethodPrivate())
{

}

CModelLoaderFactoryMethod::~CModelLoaderFactoryMethod()
{
    delete d_ptr;
}

QString CModelLoaderFactoryMethod::supportedFilters() const
{
    QStringList ls;
    for(auto pair : d_ptr->mapLoaders)
        ls << pair.first;
    return ls.join(";;");
}

CAbstractModelLoader &CModelLoaderFactoryMethod::loader(const QString &filter)
{
    return d_ptr->loader(filter);
}
