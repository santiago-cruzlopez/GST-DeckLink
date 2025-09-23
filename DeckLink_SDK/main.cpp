#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <csignal>
#include <atomic>
#include "DeckLinkAPI.h"
#include "callbacks.h"
#include "decklink_utils.h"

std::atomic<bool> g_stopFlag{false};

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received." << std::endl;
    g_stopFlag = true;
}

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

    int frameWidth = displayMode->GetWidth();
    int frameHeight = displayMode->GetHeight();
    BMDTimeValue frameDuration;
    BMDTimeScale timeScale;
    displayMode->GetFrameRate(&frameDuration, &timeScale);
    double videoFps = static_cast<double>(timeScale) / frameDuration;
    displayMode->Release();

    std::cout << "SDI Input Initialized: " << frameWidth << "x" << frameHeight << " @ " << std::fixed << std::setprecision(2) << videoFps << " fps" << std::endl;

    OutputCallback* outputCb = new OutputCallback();
    output->SetScheduledFrameCompletionCallback(outputCb);

    InputCallback* inputCb = new InputCallback(output, timeScale);
    input->SetCallback(inputCb);

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

    std::signal(SIGINT, signalHandler);

    while (!g_stopFlag.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

cleanup:
    input->StopStreams();
    output->StopScheduledPlayback(0, nullptr, timeScale);
    input->DisableVideoInput();
    input->DisableAudioInput();
    output->DisableVideoOutput();
    output->DisableAudioOutput();

    auto endTimePoint = std::chrono::steady_clock::now();
    double totalSeconds = std::chrono::duration<double>(endTimePoint - inputCb->getStartTime()).count();
    double averageFps = static_cast<double>(inputCb->getFrameCount()) / totalSeconds;

    auto endTime = std::time(nullptr);
    auto localEnd = *std::localtime(&endTime);
    std::cout << "Execution ended: " << std::put_time(&localEnd, "%Y-%m-%d %H:%M:%S") << std::endl;

    std::cout << "Metrics:" << std::endl;
    std::cout << "Total frames: " << inputCb->getFrameCount() << std::endl;
    std::cout << "Dropped frames: " << inputCb->getDropCount() << std::endl;
    std::cout << "Average FPS: " << std::fixed << std::setprecision(2) << averageFps << std::endl;
    std::cout << "Total audio samples: " << inputCb->getAudioSampleCount() << std::endl;
    std::cout << "Total runtime: " << totalSeconds << " seconds" << std::endl;

    input->SetCallback(nullptr);
    output->SetScheduledFrameCompletionCallback(nullptr);

    input->Release();
    output->Release();
    inputDevice->Release();
    outputDevice->Release();

    // Release callbacks if necessary, but since they are refcounted, they will delete themselves

    return (hr == S_OK) ? 0 : 1;
}