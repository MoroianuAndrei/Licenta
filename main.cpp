#include <iostream>
#include <iomanip>

#include "backend/testing/ManualTests.h"
#include "backend/testing/ValidationTests.h"

int main()
{
    std::cout << std::fixed << std::setprecision(4);

    runAllManualTests();
    runAllValidationTests();

    return 0;
}