#pragma once
#include <vector>
namespace csopesy {

struct ProcessRow {
    const char* name;   // points to a string literal (static storage)
    int   pid;
    float cpu;
    float memMB;
    unsigned rng;
};

// Advance LCG, return float in [0,1). Exposed for testing; prefer updateProcess.
float nextUnit(unsigned& state);
void updateProcess(ProcessRow& row);
float aggregateCpu(const std::vector<ProcessRow>& rows);
float aggregateMemMB(const std::vector<ProcessRow>& rows);
std::vector<ProcessRow> makeDefaultProcesses();
}
