#include <iostream>
#include <sndfile.h>

void invertAudio(const char* inputFile, const char* outputFile) {
    SF_INFO sfinfo;
    SNDFILE* infile, * outfile;

    // Open the input file
    infile = sf_open(inputFile, SFM_READ, &sfinfo);
    if (infile == nullptr) {
        std::cerr << "Error opening input file: " << sf_strerror(nullptr) << std::endl;
        return;
    }

    // Create the output file with the same parameters as input
    outfile = sf_open(outputFile, SFM_WRITE, &sfinfo);
    if (outfile == nullptr) {
        std::cerr << "Error opening output file: " << sf_strerror(nullptr) << std::endl;
        sf_close(infile);
        return;
    }

    // Allocate buffer for reading data
    const int bufferSize = 1024;
    float buffer[bufferSize];
    sf_count_t readCount;

    // Read data, invert, and write to output file
    while ((readCount = sf_readf_float(infile, buffer, bufferSize)) > 0) {
        for (sf_count_t i = 0; i < readCount * sfinfo.channels; ++i) {
            buffer[i] = -buffer[i]; // Invert the waveform
        }
        sf_writef_float(outfile, buffer, readCount);
    }

    // Clean up
    sf_close(infile);
    sf_close(outfile);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_audio_file> <output_audio_file>" << std::endl;
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];

    invertAudio(inputFile, outputFile);
    std::cout << "Inversion completed. Output saved to: " << outputFile << std::endl;

    return 0;
}
