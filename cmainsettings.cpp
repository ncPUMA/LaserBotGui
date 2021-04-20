#include "cmainsettings.h"

#include <map>

#include <QSettings>

#include "cabstractguisettings.h"

static const char *GUI_PREFIX = "GUI";

enum EN_GuiKeys
{
    ENGK_FIRST = 0,

    ENGK_TR_X = ENGK_FIRST,
    ENGK_TR_Y,
    ENGK_TR_Z,
    ENGK_ROT_X,
    ENGK_ROT_Y,
    ENGK_ROT_Z,
    ENGK_SC_X,
    ENGK_SC_Y,
    ENGK_SC_Z,
    ENGK_ANCH_X,
    ENGK_ANCH_Y,
    ENGK_ANCH_Z,
    ENGK_LL,

    ENGK_LAST
};
typedef int TGuiKey;

static const std::map <TGuiKey, QString> guiKeyMap = {
    { ENGK_TR_X  , "tr_x"    },
    { ENGK_TR_Y  , "tr_y"    },
    { ENGK_TR_Z  , "tr_z"    },
    { ENGK_ROT_X , "rot_x"   },
    { ENGK_ROT_Y , "rot_y"   },
    { ENGK_ROT_Z , "rot_z"   },
    { ENGK_SC_X  , "sc_x"    },
    { ENGK_SC_Y  , "sc_y"    },
    { ENGK_SC_Z  , "sc_z"    },
    { ENGK_ANCH_X, "anch_x"  },
    { ENGK_ANCH_Y, "anch_y"  },
    { ENGK_ANCH_Z, "anch_z"  },
    { ENGK_LL    , "l_laser" }
};

class CGuiSettings;

class CMainSettingsPrivate : public CAbstractGuiSettings
{
    friend class CMainSettings;

public:
    CMainSettingsPrivate() :
        settings(new QSettings()) { }

    ~CMainSettingsPrivate() {
        delete settings;
    }

    double getTranslationX() const final { return readGuiValue(ENGK_TR_X); }
    double getTranslationY() const final { return readGuiValue(ENGK_TR_Y); }
    double getTranslationZ() const final { return readGuiValue(ENGK_TR_Z); }
    double getRotationX() const final { return readGuiValue(ENGK_ROT_X); }
    double getRotationY() const final { return readGuiValue(ENGK_ROT_Y); }
    double getRotationZ() const final { return readGuiValue(ENGK_ROT_Z); }
    double getScaleX() const final { return readGuiValue(ENGK_SC_X); }
    double getScaleY() const final { return readGuiValue(ENGK_SC_Y); }
    double getScaleZ() const final { return readGuiValue(ENGK_SC_Z); }
    double getAnchorX() const final { return readGuiValue(ENGK_ANCH_X); }
    double getAnchorY() const final { return readGuiValue(ENGK_ANCH_Y); }
    double getAnchorZ() const final { return readGuiValue(ENGK_ANCH_Z); }
    double getLaserLenght() const final { return readGuiValue(ENGK_LL); }

    void setTranslationX(const double value) final { writeGuiValue(ENGK_TR_X, value); }
    void setTranslationY(const double value) final { writeGuiValue(ENGK_TR_Y, value); }
    void setTranslationZ(const double value) final { writeGuiValue(ENGK_TR_Z, value); }
    void setRotationX(const double value) final { writeGuiValue(ENGK_ROT_X, value); }
    void setRotationY(const double value) final { writeGuiValue(ENGK_ROT_Y, value); }
    void setRotationZ(const double value) final { writeGuiValue(ENGK_ROT_Z, value); }
    void setScaleX(const double value) final { writeGuiValue(ENGK_SC_X, value); }
    void setScaleY(const double value) final { writeGuiValue(ENGK_SC_Y, value); }
    void setScaleZ(const double value) final { writeGuiValue(ENGK_SC_Z, value); }
    void setAnchorX(const double value) final { writeGuiValue(ENGK_ANCH_X, value); }
    void setAnchorY(const double value) final { writeGuiValue(ENGK_ANCH_Y, value); }
    void setAnchorZ(const double value) final { writeGuiValue(ENGK_ANCH_Z, value); }
    void setLaserLenght(const double value) final { writeGuiValue(ENGK_LL, value); }

private:
    double readGuiValue(const TGuiKey key) const {
        double result = 0;
        auto it = guiKeyMap.find(key);
        if (it != guiKeyMap.cend()) {
            settings->beginGroup(GUI_PREFIX);
            result = settings->value(it->second).toDouble();
            settings->endGroup();
        }
        return result;
    }

    void writeGuiValue(const TGuiKey key, const double value) {
        auto it = guiKeyMap.find(key);
        if (it != guiKeyMap.cend()) {
            settings->beginGroup(GUI_PREFIX);
            settings->setValue(it->second, QVariant(value));
            settings->endGroup();
        }
    }

private:
    QSettings *settings;
};



CMainSettings::CMainSettings() :
    d_ptr(new CMainSettingsPrivate())
{

}

CMainSettings::~CMainSettings()
{
    delete d_ptr;
}

void CMainSettings::setSettingsFName(const char *fname)
{
    delete d_ptr->settings;
    d_ptr->settings = new QSettings(QString::fromLocal8Bit(fname));
}

CAbstractGuiSettings &CMainSettings::guiSettings()
{
    return *d_ptr;
}