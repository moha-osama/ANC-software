#include <iostream>
#include <Windows.h>
#include <thread>
#include <atomic>
#include <fstream>

using namespace  std;

#pragma comment(lib, "winmm.lib")

short int waveIn[8000 * 3 * 60];
atomic<bool> stopRecording(false);

void RecordAudio();
void KeyListener();
void SaveAudioToFile(const char* filename, WAVEHDR& header, WAVEFORMATEX& pFormat);

void KeyListener() {
    cout << "Press any key to stop recording..." << endl;
    getchar();
    stopRecording = true;
}

void RecordAudio() {
    const int sampleRate = 8000;
    HWAVEIN hWaveIn;
    MMRESULT result;
    WAVEFORMATEX pFormat = {};
    pFormat.wFormatTag = WAVE_FORMAT_PCM;
    pFormat.nChannels = 1;
    pFormat.nSamplesPerSec = sampleRate;
    pFormat.nAvgBytesPerSec = sampleRate * 2;
    pFormat.nBlockAlign = 2;
    pFormat.wBitsPerSample = 16;
    pFormat.cbSize = 0;

    result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, CALLBACK_NULL);
    
    WAVEHDR WaveInHdr = {};
    WaveInHdr.lpData = (LPSTR)waveIn;
    WaveInHdr.dwBufferLength = sizeof(waveIn);
    WaveInHdr.dwBytesRecorded = 0;
    WaveInHdr.dwUser = 0L;
    WaveInHdr.dwFlags = 0L;
    WaveInHdr.dwLoops = 0L;
    waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

    result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    result = waveInStart(hWaveIn);

    cout << "Recording started. Press any key to stop..." << endl;

    while (!stopRecording) {
        Sleep(100);
    }


    waveInStop(hWaveIn);
    waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    waveInClose(hWaveIn);

    SaveAudioToFile("output.wav", WaveInHdr, pFormat);
}

void SaveAudioToFile(const char* filename, WAVEHDR& header, WAVEFORMATEX& format) {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "File could not be opened." << endl;
        return;
    }

    // Write the RIFF header
    uint32_t chunkSize = header.dwBytesRecorded + 36;
    uint32_t subchunk1Size = 16;  // PCM header size
    uint16_t audioFormat = 1;     // PCM
    uint16_t numChannels = format.nChannels;
    uint32_t sampleRate = format.nSamplesPerSec;
    uint32_t byteRate = format.nAvgBytesPerSec;
    uint16_t blockAlign = format.nBlockAlign;
    uint16_t bitsPerSample = format.wBitsPerSample;
    uint32_t subchunk2Size = header.dwBytesRecorded;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunkSize), sizeof(chunkSize));
    file.write("WAVE", 4);
    file.write("fmt ", 4);
    file.write(reinterpret_cast<const char*>(&subchunk1Size), sizeof(subchunk1Size));
    file.write(reinterpret_cast<const char*>(&audioFormat), sizeof(audioFormat));
    file.write(reinterpret_cast<const char*>(&numChannels), sizeof(numChannels));
    file.write(reinterpret_cast<const char*>(&sampleRate), sizeof(sampleRate));
    file.write(reinterpret_cast<const char*>(&byteRate), sizeof(byteRate));
    file.write(reinterpret_cast<const char*>(&blockAlign), sizeof(blockAlign));
    file.write(reinterpret_cast<const char*>(&bitsPerSample), sizeof(bitsPerSample));
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&subchunk2Size), sizeof(subchunk2Size));
    file.write(reinterpret_cast<const char*>(header.lpData), header.dwBytesRecorded);
    cout << "Audio saved to '" << filename << "'" << endl;
}

int main()
{
    thread keyListener(KeyListener);
    RecordAudio();
    keyListener.join();
    return 0;
}
