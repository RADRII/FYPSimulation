#include "Setups.h"
#include "Util.h"
#include <iostream>

//Takes an int and sets vars equal to setupconfig for that int
Setups::Setups(int numSetup)
{
    //SETUP 0: 15 of each pop, normal perc of wipeout/plenties
    if(numSetup == 0)
    {
        numA = 15;
        numB = 15;
        numC = 15;
        percentW = 0.0025;
        percentP = 0.0025;
    }
    //SETUP 1: 20 of pop a, normal perc of wipeout/plenties
    else if(numSetup == 1)
    {
        numA = 20;
        numB = 0;
        numC = 0;
        percentW = 0.0025;
        percentP = 0.0025;
    }
    //SETUP 1: 20 of pop b, normal perc of wipeout/plenties
    else if(numSetup == 2)
    {
        numA = 0;
        numB = 20;
        numC = 0;
        percentW = 0.0025;
        percentP = 0.0025;
    }
    //SETUP 1: 20 of pop c, normal perc of wipeout/plenties
    else if(numSetup == 3)
    {
        numA = 0;
        numB = 0;
        numC = 20;
        percentW = 0.0025;
        percentP = 0.0025;
    }
    //SETUP 0: 15 of each pop, higher perc of wipeout/plenties
    if(numSetup == 4)
    {
        numA = 15;
        numB = 15;
        numC = 15;
        percentW = 0.005;
        percentP = 0.005;
    }
}