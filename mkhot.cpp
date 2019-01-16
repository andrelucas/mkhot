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
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

using namespace std;

#include <libgen.h>

#include "boost/thread/barrier.hpp"
#include "boost/filesystem.hpp"
#include "boost/program_options.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

struct options
{
    uint32_t tpc;      // Threads per core.
    uint16_t hwthread; // Hardware threads.
};

void load(size_t n, options opt, boost::barrier &b)
{
    // Sync on the passed-in barrier before doing any busy-waiting.
    b.wait();

    while (true)
    {
    }
}

void generate(options &opt)
{
    size_t nthread = opt.hwthread * opt.tpc;
    clog << "Assuming " << opt.hwthread << " hardware thread(s), starting " << nthread << " load-generator thread(s)" << endl;

    vector<thread> thr;
    boost::barrier b(nthread + 1);

    for (size_t n = 0; n < nthread; n++)
    {
        thr.emplace_back(load, n, opt, std::ref(b));
    }
    // Wait until all threads start.
    b.wait();
    // These threads won't exit, so this will wait forever.
    for (auto &t : thr)
    {
        t.join();
    }
}

int main(int argc, char *argv[])
{
    options opt{};
    // Rely on std::thread to fill the default number of hardware threads.
    opt.hwthread = std::thread::hardware_concurrency();

    fs::path prog{fs::basename(getprogname())};
    string desc = "Usage: " + prog.native() + " [OPTIONS]\nWhere OPTIONS can be";
    po::options_description od{desc};
    od.add_options()                                                                                                                          //
        ("help", "Display this message")                                                                                                      //
        ("threads-per-core,t", po::value<uint32_t>(&opt.tpc)->default_value(1), "Threads per hardware core/thread")                           //
        ("override-hw-threads,h", po::value<uint16_t>(&opt.hwthread)->default_value(opt.hwthread), "Override the number of hardware threads") //
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, od), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        clog << od << "\n";
        return 1;
    }

    generate(opt);

    return 0;
}
