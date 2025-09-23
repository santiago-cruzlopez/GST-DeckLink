#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <atomic>
#include <chrono>
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

    std::atomic<uint64_t> frameCount{0};
    std::atomic<uint64_t> dropCount{0};
    std::atomic<uint64_t> audioSampleCount{0};

    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point lastPrintTime;
    uint64_t lastFrameCount = 0;

public:
    InputCallback(IDeckLinkOutput* output, BMDTimeScale timeScale);
    virtual ~InputCallback();

    virtual HRESULT QueryInterface(REFIID iid, LPVOID *ppv) override;
    virtual ULONG AddRef() override;
    virtual ULONG Release() override;
    virtual HRESULT VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode* mode, BMDDetectedVideoInputFormatFlags flags) override;
    virtual HRESULT VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) override;

    uint64_t getFrameCount() const { return frameCount.load(); }
    uint64_t getDropCount() const { return dropCount.load(); }
    uint64_t getAudioSampleCount() const { return audioSampleCount.load(); }
    std::chrono::steady_clock::time_point getStartTime() const { return startTime; }
};

#endif // CALLBACKS_H