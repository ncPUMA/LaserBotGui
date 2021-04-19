#ifndef CABSTRACTGUISETTINGS_H
#define CABSTRACTGUISETTINGS_H

class CAbstractGuiSettings
{
public:
    CAbstractGuiSettings() { }
    virtual ~CAbstractGuiSettings() { }

    virtual double getTranslationX() const = 0;
    virtual double getTranslationY() const = 0;
    virtual double getTranslationZ() const = 0;
    virtual double getRotationX() const = 0;
    virtual double getRotationY() const = 0;
    virtual double getRotationZ() const = 0;
    virtual double getScaleX() const = 0;
    virtual double getScaleY() const = 0;
    virtual double getScaleZ() const = 0;
    virtual double getAnchorX() const = 0;
    virtual double getAnchorY() const = 0;
    virtual double getAnchorZ() const = 0;
    virtual double getLaserLenght() const = 0;

    virtual void setTranslationX(const double value) = 0;
    virtual void setTranslationY(const double value) = 0;
    virtual void setTranslationZ(const double value) = 0;
    virtual void setRotationX(const double value) = 0;
    virtual void setRotationY(const double value) = 0;
    virtual void setRotationZ(const double value) = 0;
    virtual void setScaleX(const double value) = 0;
    virtual void setScaleY(const double value) = 0;
    virtual void setScaleZ(const double value) = 0;
    virtual void setAnchorX(const double value) = 0;
    virtual void setAnchorY(const double value) = 0;
    virtual void setAnchorZ(const double value) = 0;
    virtual void setLaserLenght(const double value) = 0;
};

#endif // CABSTRACTGUISETTINGS_H
