#include "BankSimulator/bank-simulator.hpp"

const uint16_t bank_working_time = 100;

int main()
{
    BankSimulator::Bank::start_working(bank_working_time);

    return 0;
}