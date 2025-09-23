#include <iostream>
#include <thread>
#include <chrono>
#include "DeckLinkAPI.h"
#include "callbacks.h"
#include "decklink_utils.h"

int main() {
    IDeckLink* inputDevice = findDeckLinkDevice("DeckLink Duo", 3);
    IDeckLink* outputDevice = findDeckLinkDevice("DeckLink Duo", 0);

    if (!inputDevice || !outputDevice) {
        std::cerr << "Could not find required sub-devices on DeckLink Duo" << std::endl;
        if (inputDevice) inputDevice->Release();
        if (outputDevice) outputDevice->Release();
        return 1;
    }

    // Set profiles
    if (!setDeviceProfile(inputDevice, bmdProfileTwoSubDevicesHalfDuplex) ||
        !setDeviceProfile(outputDevice, bmdProfileTwoSubDevicesHalfDuplex)) {
        std::cerr << "Failed to set device profiles" << std::endl;
        inputDevice->Release();
        outputDevice->Release();
        return 1;
    }

    IDeckLinkInput* input = nullptr;
    inputDevice->QueryInterface(IID_IDeckLinkInput, reinterpret_cast<void**>(&input));

    IDeckLinkOutput* output = nullptr;
    outputDevice->QueryInterface(IID_IDeckLinkOutput, reinterpret_cast<void**>(&output));

    BMDDisplayMode selectedMode = bmdModeHD1080i5994;
    IDeckLinkDisplayMode* displayMode = nullptr;
    input->GetDisplayMode(selectedMode, &displayMode);
    if (!displayMode) {
        std::cerr << "Unsupported display mode" << std::endl;
        input->Release();
        output->Release();
        inputDevice->Release();
        outputDevice->Release();
        return 1;
    }

    BMDTimeValue frameDuration;
    BMDTimeScale timeScale;
    displayMode->GetFrameRate(&frameDuration, &timeScale);
    displayMode->Release();

    output->SetScheduledFrameCompletionCallback(new OutputCallback());
    input->SetCallback(new InputCallback(output, timeScale));

    HRESULT hr = input->EnableVideoInput(selectedMode, bmdFormat10BitYUV, bmdVideoInputFlagDefault);
    if (hr != S_OK) {
        std::cerr << "Failed to enable video input" << std::endl;
        goto cleanup;
    }

    hr = input->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 2);
    if (hr != S_OK) {
        std::cerr << "Failed to enable audio input" << std::endl;
        goto cleanup;
    }

    hr = output->EnableVideoOutput(selectedMode, bmdVideoOutputFlagDefault);
    if (hr != S_OK) {
        std::cerr << "Failed to enable video output" << std::endl;
        goto cleanup;
    }

    hr = output->EnableAudioOutput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 2, bmdAudioOutputStreamContinuous);
    if (hr != S_OK) {
        std::cerr << "Failed to enable audio output" << std::endl;
        goto cleanup;
    }

    input->StartStreams();
    output->StartScheduledPlayback(0, timeScale, 1.0);

    std::this_thread::sleep_for(std::chrono::hours(1));

cleanup:
    input->StopStreams();
    output->StopScheduledPlayback(0, nullptr, timeScale);
    input->DisableVideoInput();
    input->DisableAudioInput();
    output->DisableVideoOutput();
    output->DisableAudioOutput();

    input->SetCallback(nullptr);
    output->SetScheduledFrameCompletionCallback(nullptr);

    input->Release();
    output->Release();
    inputDevice->Release();
    outputDevice->Release();

    return (hr == S_OK) ? 0 : 1;
}