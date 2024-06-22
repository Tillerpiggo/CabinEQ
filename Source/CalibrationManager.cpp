/*
  ==============================================================================

    CalibrationManager.cpp
    Created: 22 Jun 2024 11:08:05am
    Author:  Tyler Gee

  ==============================================================================
*/

#include "CalibrationManager.h"

void CalibrationManager::setDelegate (CalibrationManagerDelegate* delegate) 
{
    this->delegate = delegate;
}

void CalibrationManager::calibrateWith (Choice choice)
{
    if (choice == Choice::LowerPreferred) {
        std::cout << "Lower Preferred chosen.\n";
    } else {
        std::cout << "Higher Preferred chosen.\n";
    }
}

double CalibrationManager::getNextSample()
{
    return sequencer.getNextSample();
}


