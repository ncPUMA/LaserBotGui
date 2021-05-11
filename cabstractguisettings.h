#ifndef CABSTRACTGUISETTINGS_H
#define CABSTRACTGUISETTINGS_H

#include "gui_types.h"

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
    virtual double getLaserX() const = 0;
    virtual double getLaserY() const = 0;
    virtual double getLaserZ() const = 0;
    virtual double getScale() const = 0;
    virtual GUI_TYPES::TMSAA getMsaa() const = 0;
    //for The Grip
    virtual bool isGripVisible() const = 0;
    virtual double getGripTranslationX() const = 0;
    virtual double getGripTranslationY() const = 0;
    virtual double getGripTranslationZ() const = 0;
    virtual double getGripRotationX() const = 0;
    virtual double getGripRotationY() const = 0;
    virtual double getGripRotationZ() const = 0;
    virtual double getGripAnchorX() const = 0;
    virtual double getGripAnchorY() const = 0;
    virtual double getGripAnchorZ() const = 0;
    virtual double getGripScale() const = 0;

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
    virtual void setLaserX(const double value) = 0;
    virtual void setLaserY(const double value) = 0;
    virtual void setLaserZ(const double value) = 0;
    virtual void setScale(const double value) = 0;
    virtual void setMsaa(const GUI_TYPES::TMSAA value) = 0;
    //for The Grip
    virtual void setGripVisible(const bool value) = 0;
    virtual void setGripTranslationX(const double value) = 0;
    virtual void setGripTranslationY(const double value) = 0;
    virtual void setGripTranslationZ(const double value) = 0;
    virtual void setGripRotationX(const double value) = 0;
    virtual void setGripRotationY(const double value) = 0;
    virtual void setGripRotationZ(const double value) = 0;
    virtual void setGripAnchorX(const double value) = 0;
    virtual void setGripAnchorY(const double value) = 0;
    virtual void setGripAnchorZ(const double value) = 0;
    virtual void setGripScale(const double value) = 0;
};

#endif // CABSTRACTGUISETTINGS_H
