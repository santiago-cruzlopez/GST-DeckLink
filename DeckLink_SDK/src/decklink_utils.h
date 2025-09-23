#ifndef DECKLINK_UTILS_H
#define DECKLINK_UTILS_H

#include "DeckLinkAPI.h"
#include <string>

// IID constants (to avoid rvalue address issues)
extern const REFIID kIID_IUnknown;
extern const REFIID kIID_IDeckLinkVideoOutputCallback;
extern const REFIID kIID_IDeckLinkInputCallback;

// Utility functions
IDeckLink* findDeckLinkDevice(const std::string& modelName, int64_t subIndex);
bool setDeviceProfile(IDeckLink* device, BMDProfileID profileID);

#endif // DECKLINK_UTILS_H