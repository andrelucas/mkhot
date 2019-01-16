/**
 * @file mkhot.cpp
 * @author André Lucas <andre@ae-35.com
 * @brief Use CPU resources like they're going out of fashion.
 * @version 0.1
 * @date 2019-01-16
 *
 * @copyright Copyright (c) 2019 André Lucas
 */

#include <chrono>
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

using namespace std;

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

enum class busymode { none = 0, wait, io };

static string to_string(busymode m) {
    switch (m) {
        case busymode::wait:
            return "wait";
        case busymode::io:
            return "io";
        default:
            return "none";
    }
}

struct options {
    uint32_t tpc;       // Threads per core.
    uint16_t hwthread;  // Hardware threads.
    busymode mode;      // The way we'll burn CPU.
};

/**
 * @brief A good old-fashioned spin loop.
 *
 * @param n     The thread number, ignored.
 * @param opt   Options, ignored.
 */
void busywait(size_t n, const options opt) {
    while (true) {
    }
}

/**
 * @brief Perform some reads to generate system calls.
 *
 * @param n     The thread number, ignored.
 * @param opt   Options, ignored.
 */
void busyio(size_t n, const options opt) {
    string infile{"/dev/zero"};
    int f = open(infile.c_str(), O_RDONLY);
    if (f == -1) {
        throw runtime_error("Failed to open input file");
    }

    while (true) {
        char c[4096];
        ssize_t err = read(f, &c, sizeof(c));
        if (err == -1) {
            throw runtime_error("Failed to read input file");
        }
    }
    close(f);
}

/**
 * @brief A load-generator thread, calls a specific load type.
 *
 * @param n     The thread number, ignored (but passed along).
 * @param opt   Options, ignored (but passed along).
 */
void load(size_t n, const options opt) {
    // Choose. Your. Pain.
    switch (opt.mode) {
        case busymode::io:
            busyio(n, opt);
            break;
        default:
            busywait(n, opt);
            break;
    }
}

/**
 * @brief Start load-generator threads and wait forever.
 *
 * @param opt Options.
 */
void generate(const options &opt) {
    size_t nthread = opt.hwthread * opt.tpc;
    clog << "Starting " << nthread << " load-generator thread(s), mode '" << to_string(opt.mode) << "'" << endl;

    vector<thread> thr;

    for (size_t n = 0; n < nthread; n++) {
        thr.emplace_back(load, n, opt);
    }

    // These threads won't exit, so this will wait forever.
    for (auto &t : thr) {
        t.join();
    }
}

int main(int argc, char *argv[]) {
    options opt{};
    // Rely on std::thread to fill the default number of hardware threads.
    opt.hwthread = std::thread::hardware_concurrency();
    // Default is a busy-wait loop.
    bool iowait = false;
    bool nofork = false;

    fs::path prog{fs::basename(argv[0])};
    string desc = "Usage: " + prog.native() + " [OPTIONS]\nWhere OPTIONS can be";
    po::options_description od{desc};
    od.add_options()                                                                //
        ("help", "Display this message")                                            //
        ("io-load,i", po::bool_switch(&iowait), "Perform I/O in the load threads")  //
        ("no-fork,n", po::bool_switch(&nofork), "Do not fork() on startup")         //
        ("override-hw-threads,h", po::value<uint16_t>(&opt.hwthread)->default_value(opt.hwthread),
         "Override the number of hardware threads")                                                                  //
        ("threads-per-core,t", po::value<uint32_t>(&opt.tpc)->default_value(1), "Threads per hardware core/thread")  //
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, od), vm);
    po::notify(vm);

    if (vm.count("help")) {
        clog << od << "\n";
        return 1;
    }

    if (iowait)
        opt.mode = busymode::io;
    else
        opt.mode = busymode::wait;

    if (!nofork) {
        // Fork so we're not PID 1. This way, we get signals such as ctrl-c even
        // when running in a Docker container.
        int status;
        if (fork()) {
            wait(&status);
            return status;
        }
    }

    // If we forked, this is now child-process only.
    generate(opt);

    return 0;
}
