#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <string>

#include "Commands/Assignment.h"

#include "../Shell/Shell.h"

#include "../Helpers/MultiThreading.h"
#include "../Helpers/Console/CommandLineParser.h"

using namespace Shell;

int main(int argc, char** argv) {
    CommandLineParser clp(argc, argv);
    pinThreadToCoreId(clp.value<int>("core", 1));
    checkAsserts();

    ::Shell::Shell shell;
    new ParseCSAFromCSV(shell);
    new GroupAssignment(shell);
    shell.run();
    return 0;
}
