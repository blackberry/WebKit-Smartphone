/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_LocalizeResource_h
#define Olympia_Platform_LocalizeResource_h

// FIXME Rename file LocalizedResource (with "d")

#define COUNTRY_NUM 28

namespace BlackBerry {
namespace Platform {

enum CountryLocaleId {
     AR = 0x61720000, // Arabic
     CA = 0x63610000, // Catalan
     CS = 0x63730000, // Czech
     DE = 0x64650000, // German
     EL = 0x656c0000, // Greek
     EN = 0x656E0000, // English
     EN_GB = 0x656E4742, // English - Great Britain
     EN_US = 0x656E5553, // English - USA
     ES = 0x65730000, // Spanish
     ES_MX = 0x65734D58, // Spanish - Mexico
     FA = 0x66610000, // Farsi (Persian)
     FR = 0x66720000, // French
     FR_CA = 0x66724341, // French - Canada
     HE = 0x68650000, // Hebrew
     HU = 0x68750000, // Hungarian
     IT = 0x69740000, // Italian
     JA = 0x6a610000, // Japanese
     KO = 0x6b6f0000, // Korean
     NL = 0x6e6c0000, // Dutch
     PL = 0x706c0000, // Polish
     PT = 0x70740000, // Portuguese
     PT_BR = 0x70744252, // Portuguese - Brazil
     RU = 0x72750000, // Russian
     TR = 0x74720000, // Turkish
     UR = 0x75720000, // Urdu
     ZH = 0x7A680000, // Chinese
     ZH_CN = 0x7A68434E, // Chinese - mainland
     ZH_HK = 0x7A68484B // Chinese - Hong Kong
};

enum CountryLocaleType {
     LOCALE_ar = 0,
     LOCALE_ca,
     LOCALE_cs,
     LOCALE_de,
     LOCALE_el,
     LOCALE_en,
     LOCALE_en_GB,
     LOCALE_en_US,
     LOCALE_es,
     LOCALE_es_MX,
     LOCALE_fa,
     LOCALE_fr,
     LOCALE_fr_CA,
     LOCALE_he,     
     LOCALE_hu, 
     LOCALE_it, 
     LOCALE_ja, 
     LOCALE_ko, 
     LOCALE_nl, 
     LOCALE_pl, 
     LOCALE_pt, 
     LOCALE_pt_BR,
     LOCALE_ru, 
     LOCALE_tr, 
     LOCALE_ur, 
     LOCALE_zh, 
     LOCALE_zh_CN,
     LOCALE_zh_HK
};

// FIXME: Enums look like macros.
enum ResourcesType {
    FILE_CHOOSE_BUTTON_LABEL = 0,
    FILE_BUTTON_NO_FILE_SELECTED_LABEL,
    RESET_BUTTON_LABEL,
    SUBMIT_BUTTON_LABEL
};


class LocalizeResource{

public:

    static char* getString(ResourcesType);
};

} // Olympia
} // Platform

#endif // LocalizeResource_h
