#pragma once
#include <string>
#include <vector>
namespace csopesy {

struct ProcessRow {
    const char* name;
    int   pid;
    float cpu;
    float memMB;
    unsigned rng;
};

float nextUnit(unsigned& state);
void updateProcess(ProcessRow& row);
float aggregateCpu(const std::vector<ProcessRow>& rows);
float aggregateMemMB(const std::vector<ProcessRow>& rows);
std::vector<ProcessRow> makeDefaultProcesses();
}
