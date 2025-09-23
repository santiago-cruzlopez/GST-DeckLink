#include "decklink_utils.h"
#include <iostream>
#include <cstdlib>

// Define IID constants
const REFIID kIID_IUnknown = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46};
const REFIID kIID_IDeckLinkVideoOutputCallback = {0x20,0xAA,0x52,0x25,0x19,0x58,0x47,0xCB,0x82,0x0B,0x80,0xA8,0xD5,0x21,0xA6,0xEE};
const REFIID kIID_IDeckLinkInputCallback = {0xDD,0x04,0xE5,0xEC,0x74,0x15,0x42,0xAB,0xAE,0x4A,0xE8,0x0C,0x4D,0xFC,0x04,0x4A};

IDeckLink* findDeckLinkDevice(const std::string& modelName, int64_t subIndex) {
    IDeckLinkIterator* iterator = CreateDeckLinkIteratorInstance();
    if (!iterator) return nullptr;

    IDeckLink* deckLink = nullptr;
    while (iterator->Next(&deckLink) == S_OK) {
        const char* currentModelName = nullptr;
        deckLink->GetModelName(&currentModelName);
        std::string currentModel(currentModelName ? currentModelName : "");
        free(const_cast<char*>(currentModelName));

        if (currentModel.find(modelName) == std::string::npos) {
            deckLink->Release();
            continue;
        }

        IDeckLinkProfileAttributes* attrs = nullptr;
        if (deckLink->QueryInterface(IID_IDeckLinkProfileAttributes, reinterpret_cast<void**>(&attrs)) == S_OK) {
            int64_t currentSubIndex;
            if (attrs->GetInt(BMDDeckLinkSubDeviceIndex, &currentSubIndex) == S_OK && currentSubIndex == subIndex) {
                attrs->Release();
                iterator->Release();
                return deckLink;
            }
            attrs->Release();
        }
        deckLink->Release();
    }
    iterator->Release();
    return nullptr;
}

bool setDeviceProfile(IDeckLink* device, BMDProfileID profileID) {
    IDeckLinkProfileAttributes* attrs = nullptr;
    if (device->QueryInterface(IID_IDeckLinkProfileAttributes, reinterpret_cast<void**>(&attrs)) != S_OK) {
        return false;
    }

    int64_t currentProfile;
    attrs->GetInt(BMDDeckLinkProfileID, &currentProfile);
    attrs->Release();

    if (currentProfile == profileID) {
        return true;
    }

    IDeckLinkProfileManager* profileMgr = nullptr;
    if (device->QueryInterface(IID_IDeckLinkProfileManager, reinterpret_cast<void**>(&profileMgr)) != S_OK) {
        return false;
    }

    IDeckLinkProfile* newProfile = nullptr;
    if (profileMgr->GetProfile(profileID, &newProfile) == S_OK && newProfile) {
        newProfile->SetActive();
        newProfile->Release();
    }
    profileMgr->Release();
    return true;
}