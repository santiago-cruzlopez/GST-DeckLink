#include "callbacks.h"
#include <iostream>
#include <cstring> // for memcmp
#include <iomanip> // for std::fixed
#include "decklink_utils.h" // for IID constants

OutputCallback::~OutputCallback() {}

HRESULT OutputCallback::QueryInterface(REFIID iid, LPVOID *ppv) {
    if (!ppv) return E_INVALIDARG;
    *ppv = nullptr;
    if (memcmp(&iid, &kIID_IDeckLinkVideoOutputCallback, sizeof(REFIID)) == 0 ||
        memcmp(&iid, &kIID_IUnknown, sizeof(REFIID)) == 0) {
        *ppv = static_cast<IDeckLinkVideoOutputCallback*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG OutputCallback::AddRef() {
    return ++refCount;
}

ULONG OutputCallback::Release() {
    ULONG newRef = --refCount;
    if (newRef == 0) {
        delete this;
        return 0;
    }
    return newRef;
}

HRESULT OutputCallback::ScheduledFrameCompleted(IDeckLinkVideoFrame* completedFrame, BMDOutputFrameCompletionResult result) {
    if (completedFrame) {
        completedFrame->Release();
    }
    return S_OK;
}

HRESULT OutputCallback::ScheduledPlaybackHasStopped() {
    return S_OK;
}

InputCallback::InputCallback(IDeckLinkOutput* output, BMDTimeScale timeScale) 
    : m_output(output), m_timeScale(timeScale) {
    startTime = lastPrintTime = std::chrono::steady_clock::now();
}

InputCallback::~InputCallback() {}

HRESULT InputCallback::QueryInterface(REFIID iid, LPVOID *ppv) {
    if (!ppv) return E_INVALIDARG;
    *ppv = nullptr;
    if (memcmp(&iid, &kIID_IDeckLinkInputCallback, sizeof(REFIID)) == 0 ||
        memcmp(&iid, &kIID_IUnknown, sizeof(REFIID)) == 0) {
        *ppv = static_cast<IDeckLinkInputCallback*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

ULONG InputCallback::AddRef() {
    return ++refCount;
}

ULONG InputCallback::Release() {
    ULONG newRef = --refCount;
    if (newRef == 0) {
        delete this;
        return 0;
    }
    return newRef;
}

HRESULT InputCallback::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode* mode, BMDDetectedVideoInputFormatFlags flags) {
    std::cout << "Video format changed" << std::endl;
    return S_OK;
}

HRESULT InputCallback::VideoInputFrameArrived(IDeckLinkVideoInputFrame* videoFrame, IDeckLinkAudioInputPacket* audioPacket) {
    if (videoFrame) {
        frameCount++;
        videoFrame->AddRef();
        BMDTimeValue streamTime, duration;
        videoFrame->GetStreamTime(&streamTime, &duration, m_timeScale);
        m_output->ScheduleVideoFrame(videoFrame, streamTime, duration, m_timeScale);
    } else {
        dropCount++;
    }

    if (audioPacket) {
        void* buffer;
        audioPacket->GetBytes(&buffer);
        uint32_t sampleCountLocal = audioPacket->GetSampleFrameCount();
        audioSampleCount += sampleCountLocal;
        BMDTimeValue packetTime;
        audioPacket->GetPacketTime(&packetTime, m_timeScale);
        m_output->ScheduleAudioSamples(buffer, sampleCountLocal, packetTime, m_timeScale, nullptr);
    }

    auto currentTime = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastPrintTime).count();
    if (delta >= 1) {
        double fps = static_cast<double>(frameCount - lastFrameCount) / delta;
        std::cout << "Current FPS: " << std::fixed << std::setprecision(2) << fps << std::endl;
        lastFrameCount = frameCount.load();
        lastPrintTime = currentTime;
    }

    return S_OK;
}