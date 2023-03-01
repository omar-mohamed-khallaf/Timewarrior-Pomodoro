#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <codecvt>
#include <locale>

#include "utils.h"

utils::ProcessResult
utils::executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false) {
    int fields[2];  // 0: read fd, 1: write fd
    char buf[256]{0};
    auto status{0};
    std::string output;

    if (pipe(fields) == -1) throw std::runtime_error("Failed to create pipe");
    auto pid{fork()};

    switch (pid) {
        case -1:
            throw std::runtime_error("Failed to fork");
        case 0:
            close(fields[0]);
            dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
            dup2(fields[1], STDOUT_FILENO);
            dup2(fields[1], STDERR_FILENO);
            execv(path.c_str(), (char *const *) (args.begin().base()));     // shouldn't return
            close(fields[1]);
            _exit(-1);          // to terminate all threads
        default:
            close(fields[1]);
            waitpid(pid, &status, 0);
            while (read(fields[0], buf, sizeof(buf)) > 0) { output.append(buf); }
            close(fields[0]);

            if (WEXITSTATUS(status) == 127) throw std::runtime_error("Failed to exec: " + path);
            break;
    }
    output.append(1, '\n');                                     // make sure we have a line end
    return {static_cast<uint8_t>(WEXITSTATUS(status)), output}; // get one line from output
}

std::string utils::formatDescription(const std::string &description) {
    std::string newDescription;
    for (auto it{description.begin() + 9}; it < description.end(); ++it) { // skip "Tracking " word
        if (*it == '\n')
            break;
        else if (*it != '\\')
            newDescription.append(1, *it);
    }
    return newDescription;
}

std::wstring utils::stringToUtf(const std::string &string) {
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.from_bytes(string);
}

std::string utils::utfToString(const std::wstring &wstring) {
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wstring);
}

std::unique_ptr<char>
utils::WavReader::loadWAV(const std::string &audioFile, unsigned int &chan, unsigned int &sampleRate, unsigned int &bps,
                          unsigned int &size) {
    auto file = std::unique_ptr<FILE, decltype(&std::fclose)>(std::fopen(audioFile.c_str(), "r"), std::fclose);
    struct WavHeader wavHeader{};
    std::fread(&wavHeader, 1, sizeof(wavHeader), file.get());
    chan = wavHeader.numOfChan;
    sampleRate = wavHeader.samplesPerSec;
    bps = wavHeader.bitsPerSample;
    size = wavHeader.subChunk2Size;
    auto buffer = std::unique_ptr<char>(new char[size]);
    std::fread(buffer.get(), sizeof(char), size, file.get());
    return buffer;
}
