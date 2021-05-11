#include "cmainsettings.h"

#include <map>

#include <QSettings>
#include <QHostAddress>

#include "cabstractguisettings.h"
#include "BotSocket/cabstractbotsocketsettings.h"

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
    ENGK_LX,
    ENGK_LY,
    ENGK_LZ,
    ENGK_SC,
    ENGK_MSAA,
    ENGK_GRIP_VIS,
    ENGK_GRIP_TR_X,
    ENGK_GRIP_TR_Y,
    ENGK_GRIP_TR_Z,
    ENGK_GRIP_ROT_X,
    ENGK_GRIP_ROT_Y,
    ENGK_GRIP_ROT_Z,
    ENGK_GRIP_SC,

    ENGK_LAST
};
typedef int TGuiKey;

static const std::map <TGuiKey, QString> guiKeyMap = {
    { ENGK_TR_X      , "tr_x"         },
    { ENGK_TR_Y      , "tr_y"         },
    { ENGK_TR_Z      , "tr_z"         },
    { ENGK_ROT_X     , "rot_x"        },
    { ENGK_ROT_Y     , "rot_y"        },
    { ENGK_ROT_Z     , "rot_z"        },
    { ENGK_SC_X      , "sc_x"         },
    { ENGK_SC_Y      , "sc_y"         },
    { ENGK_SC_Z      , "sc_z"         },
    { ENGK_ANCH_X    , "anch_x"       },
    { ENGK_ANCH_Y    , "anch_y"       },
    { ENGK_ANCH_Z    , "anch_z"       },
    { ENGK_LX        , "laser_x"      },
    { ENGK_LY        , "laser_y"      },
    { ENGK_LZ        , "laser_z"      },
    { ENGK_SC        , "sc"           },
    { ENGK_MSAA      , "msaa"         },
    { ENGK_GRIP_VIS  , "grip_vis"     },
    { ENGK_GRIP_TR_X , "grip_tr_x"    },
    { ENGK_GRIP_TR_Y , "grip_tr_y"    },
    { ENGK_GRIP_TR_Z , "grip_tr_z"    },
    { ENGK_GRIP_ROT_X, "grip_rot_x"   },
    { ENGK_GRIP_ROT_Y, "grip_rot_y"   },
    { ENGK_GRIP_ROT_Z, "grip_rot_z"   },
    { ENGK_GRIP_SC   , "grip_sc"      }
};

static const char *BOT_PREFIX = "BOT";
static const char *BOT_KEY_LCL_IP = "lcl_ip";
static const char *BOT_KEY_LCL_PORT = "lcl_port";
static const char *BOT_KEY_REM_IP = "rem_ip";
static const char *BOT_KEY_REM_PORT = "rem_port";

static const char *BOT_DEF_LCL_IP = "192.168.0.1";
static const uint16_t BOT_DEF_LCL_PORT = 1122;
static const char *BOT_DEF_REM_IP = "192.168.0.2";
static const uint16_t BOT_DEF_REM_PORT = 2211;

class CMainSettingsPrivate : public CAbstractGuiSettings, public CAbstractBotSocketSettings
{
    friend class CMainSettings;

public:
    CMainSettingsPrivate() :
        CAbstractGuiSettings(),
        CAbstractBotSocketSettings(),
        settings(new QSettings()) {

    }

    ~CMainSettingsPrivate() {
        delete settings;
    }

    //CAbstractGuiSettings
    double getTranslationX() const final { return readGuiValue <double> (ENGK_TR_X); }
    double getTranslationY() const final { return readGuiValue <double> (ENGK_TR_Y); }
    double getTranslationZ() const final { return readGuiValue <double> (ENGK_TR_Z); }
    double getRotationX() const final { return readGuiValue <double> (ENGK_ROT_X); }
    double getRotationY() const final { return readGuiValue <double> (ENGK_ROT_Y); }
    double getRotationZ() const final { return readGuiValue <double> (ENGK_ROT_Z); }
    double getScaleX() const final { return readGuiValue <double> (ENGK_SC_X); }
    double getScaleY() const final { return readGuiValue <double> (ENGK_SC_Y); }
    double getScaleZ() const final { return readGuiValue <double> (ENGK_SC_Z); }
    double getAnchorX() const final { return readGuiValue <double> (ENGK_ANCH_X); }
    double getAnchorY() const final { return readGuiValue <double> (ENGK_ANCH_Y); }
    double getAnchorZ() const final { return readGuiValue <double> (ENGK_ANCH_Z); }
    double getLaserX() const final { return readGuiValue <double> (ENGK_LX); }
    double getLaserY() const final { return readGuiValue <double> (ENGK_LY); }
    double getLaserZ() const final { return readGuiValue <double> (ENGK_LZ); }
    double getScale() const final { return readGuiValue <double> (ENGK_SC); }
    GUI_TYPES::TMSAA getMsaa() const final { return readGuiValue <GUI_TYPES::TMSAA> (ENGK_MSAA); }
    //for The Grip
    bool isGripVisible() const final { return readGuiValue <bool> (ENGK_GRIP_VIS); }
    double getGripTranslationX() const final { return readGuiValue <double> (ENGK_GRIP_TR_X); }
    double getGripTranslationY() const final { return readGuiValue <double> (ENGK_GRIP_TR_Y); }
    double getGripTranslationZ() const final { return readGuiValue <double> (ENGK_GRIP_TR_Z); }
    double getGripRotationX() const final { return readGuiValue <double> (ENGK_GRIP_ROT_X); }
    double getGripRotationY() const final { return readGuiValue <double> (ENGK_GRIP_ROT_Y); }
    double getGripRotationZ() const final { return readGuiValue <double> (ENGK_GRIP_ROT_Z); }
    double getGripScale() const final { return readGuiValue <double> (ENGK_GRIP_SC); }

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
    void setLaserX(const double value) final { writeGuiValue(ENGK_LX, value); }
    void setLaserY(const double value) final { writeGuiValue(ENGK_LY, value); }
    void setLaserZ(const double value) final { writeGuiValue(ENGK_LZ, value); }
    void setScale(const double value) final { writeGuiValue(ENGK_SC, value); }
    void setMsaa(const GUI_TYPES::TMSAA value) final { writeGuiValue(ENGK_MSAA, value); }
    //for The Grip
    void setGripVisible(const bool value) final { writeGuiValue(ENGK_GRIP_VIS, value); }
    void setGripTranslationX(const double value) final { writeGuiValue(ENGK_GRIP_TR_X, value); }
    void setGripTranslationY(const double value) final { writeGuiValue(ENGK_GRIP_TR_Y, value); }
    void setGripTranslationZ(const double value) final { writeGuiValue(ENGK_GRIP_TR_Z, value); }
    void setGripRotationX(const double value) final { writeGuiValue(ENGK_GRIP_ROT_X, value); }
    void setGripRotationY(const double value) final { writeGuiValue(ENGK_GRIP_ROT_Y, value); }
    void setGripRotationZ(const double value) final { writeGuiValue(ENGK_GRIP_ROT_Z, value); }
    void setGripScale(const double value) final { writeGuiValue(ENGK_GRIP_SC, value); }

    //CAbstractBotSocketSettings
    uint32_t getLocalIpV4() const { return readIp(BOT_PREFIX, BOT_KEY_LCL_IP); }
    uint16_t getLocalUdpPort() const { return readIp(BOT_PREFIX, BOT_KEY_LCL_PORT); }
    uint32_t getRemoteBotIpV4() const { return readIp(BOT_PREFIX, BOT_KEY_REM_IP); }
    uint16_t getRemoteBotUdpPort() const { return readIp(BOT_PREFIX, BOT_KEY_REM_PORT); }

private:
    template <typename T>
    T readGuiValue(const TGuiKey key) const {
        T result = T();
        auto it = guiKeyMap.find(key);
        if (it != guiKeyMap.cend()) {
            settings->beginGroup(GUI_PREFIX);
            result = settings->value(it->second).value <T> ();
            settings->endGroup();
        }
        return result;
    }

    template <typename T>
    void writeGuiValue(const TGuiKey key, const T value) {
        auto it = guiKeyMap.find(key);
        if (it != guiKeyMap.cend()) {
            settings->beginGroup(GUI_PREFIX);
            settings->setValue(it->second, QVariant::fromValue <T> (value));
            settings->endGroup();
        }
    }

    uint32_t readIp(const char *group, const char *key) const {
        QString txt;
        settings->beginGroup(group);
        txt = settings->value(key).toString();
        settings->endGroup();
        return QHostAddress(txt).toIPv4Address();
    }

    void writeIp(const char *group, const char *key, const uint32_t ip) {
        settings->beginGroup(group);
        settings->setValue(key, QHostAddress(ip).toString());
        settings->endGroup();
    }

    uint16_t readPort(const char *group, const char *key) const {
        uint16_t result = 0;
        settings->beginGroup(group);
        result = static_cast <uint16_t> (settings->value(key).toUInt());
        settings->endGroup();
        return result;
    }

    void writePort(const char *group, const char *key, const uint16_t port) {
        settings->beginGroup(group);
        settings->setValue(key, port);
        settings->endGroup();
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
    d_ptr->settings = new QSettings(QString::fromLocal8Bit(fname), QSettings::IniFormat);

    d_ptr->settings->beginGroup(BOT_PREFIX);
    if (!d_ptr->settings->contains(BOT_KEY_LCL_IP))
        d_ptr->settings->setValue(BOT_KEY_LCL_IP, BOT_DEF_LCL_IP);
    if (!d_ptr->settings->contains(BOT_KEY_LCL_PORT))
        d_ptr->settings->setValue(BOT_KEY_LCL_PORT, BOT_DEF_LCL_PORT);
    if (!d_ptr->settings->contains(BOT_KEY_REM_IP))
        d_ptr->settings->setValue(BOT_KEY_REM_IP, BOT_DEF_REM_IP);
    if (!d_ptr->settings->contains(BOT_KEY_REM_PORT))
        d_ptr->settings->setValue(BOT_KEY_REM_PORT, BOT_DEF_REM_PORT);
    d_ptr->settings->endGroup();

    d_ptr->settings->beginGroup(GUI_PREFIX);
    if (!d_ptr->settings->contains(guiKeyMap.at(ENGK_MSAA)))
        d_ptr->settings->setValue(guiKeyMap.at(ENGK_MSAA), GUI_TYPES::ENMSAA_4);
    d_ptr->settings->endGroup();
}

CAbstractGuiSettings &CMainSettings::guiSettings()
{
    return *d_ptr;
}

CAbstractBotSocketSettings &CMainSettings::socketSettings()
{
    return *d_ptr;
}
