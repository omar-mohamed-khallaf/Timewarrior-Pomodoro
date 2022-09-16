#include <thread>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/wait.h>
#include <codecvt>
#include <locale>

#include "utils.h"

std::wstring utils::executeProcess(const std::string &path, const std::vector<const char *> &args) noexcept(false) {
    int fields[2];  // 0: read fd, 1: write fd
    pipe(fields);
    char buf[256]{0};
    std::string output;
    auto pid{vfork()};
    switch (pid) {
        case -1:
            throw std::runtime_error("Failed to fork");
        case 0:
            dup2(open("/dev/null", O_RDONLY), STDIN_FILENO);
            dup2(fields[1], STDOUT_FILENO);
            dup2(fields[1], STDERR_FILENO);
            execv(path.c_str(), (char *const *) (args.begin().base()));     // shouldn't return
            break;
        default:
            // TODO: handle errors
            read(fields[0], buf, sizeof(buf));
            waitpid(pid, nullptr, 0);
            output.append(buf);
            if (output.empty()) throw std::runtime_error("Failed to run " + path);
            break;
    }
    output.append(1, '\n');                                             // make sure we have a line end
    return utils::toWstring(output.substr(0, output.find('\n') + 1));   // get one line from output
}

std::wstring utils::toWstring(const std::string &string) {
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.from_bytes(string);
}

std::string utils::toString(const std::wstring &wstring) {
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wstring);
}


std::unique_ptr<char>
utils::WavReader::loadWAV(const std::string &audioFile, unsigned int &chan, unsigned int &sampleRate, unsigned int &bps,
                          unsigned int &size) {
    auto file = std::unique_ptr<FILE, decltype(&fclose)>(std::fopen(audioFile.c_str(), "r"), fclose);
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