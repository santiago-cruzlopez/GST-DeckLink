#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <atomic>
#include "DeckLinkAPI.h"

class OutputCallback : public IDeckLinkVideoOutputCallback {
private:
    std::atomic<ULONG> refCount{1};

public:
    virtual ~OutputCallback();

    virtual HRESULT QueryInterface(REFIID iid, LPVOID *ppv) override;
    virtual ULONG AddRef() override;
    virtual ULONG Release() override;
    virtual HRESULT ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) override;
    virtual HRESULT ScheduledPlaybackHasStopped() override;
};

class InputCallback : public IDeckLinkInputCallback {
private:
    std::atomic<ULONG> refCount{1};
    IDeckLinkOutput* m_output;
    BMDTimeScale m_timeScale;

public:
    InputCallback(IDeckLinkOutput* output, BMDTimeScale timeScale);
    virtual ~InputCallback();

    virtual HRESULT QueryInterface(REFIID iid, LPVOID *ppv) override;
    virtual ULONG AddRef() override;
    virtual ULONG Release() override;
    virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode* mode, BMDDetectedVideoInputFormatFlags flags) override;
    virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override;
};

#endif // CALLBACKS_H