///*
//  ==============================================================================
//
//    CalibrationManager.cpp
//    Created: 22 Jun 2024 11:08:05am
//    Author:  Tyler Gee
//
//  ==============================================================================
//*/
//
//#include "CalibrationManager.h"
//#include <random>
//
//CalibrationManager::CalibrationManager ()
//{
//    //curve.setSetPointMap (std::make_unique<std::map<float, CalibratedSetPoint>> (setPoints));
//}
//
//void CalibrationManager::setDelegate (CalibrationManagerDelegate* delegate) 
//{
//    this->delegate = delegate;
//}
//
//void CalibrationManager::setSampleRate (float newSampleRate)
//{
//    sequencer.setSampleRate (newSampleRate);
//    
//    // Might as well prepare here, but this is kinda a bad idea
//    Melody melody = calibrationSequence.getNextMelody();
//    changeMelodyTo (melody);
//}
//
//void CalibrationManager::chooseOption (CalibrationChoice choice)
//{
////    std::cout << "CHOOSING OPTION: " << std::endl;
////    std::cout << "curr val: " << setPoints[currentSetPointFreq].estimatedValue() << std::endl;;
//    // Calibrate the relevant (current) choice
//    setPoints[currentSetPointFreq].calibrateWith (choice);
////    std::cout << "curr val after: " << setPoints[currentSetPointFreq].estimatedValue() << std::endl;
//    
//    // Move to the next melody
//    Melody melody = calibrationSequence.getNextMelody();
//    changeMelodyTo (melody);
//    
//    // Print out all set points
//    for (const auto& pair : setPoints) {
//        std::cout << "Set Point (" << pair.first << "): Estimated Value = " << pair.second.estimatedValue() << '\n';
//    }
//    
//}
//
//float CalibrationManager::getNextSample()
//{
//    return sequencer.getNextSample();
//}
//
//void CalibrationManager::changeMelodyTo (const Melody& melody)
//{
//    // Add new set point if we don't have one for the controlled frequency
//    if (setPoints.find (melody.getControlledFrequency()) == setPoints.end())
//    {
//        float variance = 12.0; // dB range to binary search under for new set point
//        float estimatedGain = curve.valueAtFrequency (melody.getControlledFrequency()).real();
//        CalibratedSetPoint newSetPoint (estimatedGain - variance, estimatedGain + variance);
//        setPoints[melody.getControlledFrequency()] = newSetPoint;
//    }
//    
//    std::cout << "Melody controlled freq: " << melody.getControlledFrequency() << std::endl;
//    
//    currentSetPointFreq = melody.getControlledFrequency();
//    
//    // Create a NoteSequence from the melody
//    NoteSequence noteSequence;
//    
//    // Randomize order
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> dis(0, 1);
//    int flip = dis(gen);
//    flip = 1; // hard code it to be non-random
//    
//    // First play the lower bound
//    for (int noteVal : melody.getNotes())
//    {
//        float freq = melody.freqForNote (noteVal);
//        float gain = curve.valueAtFrequency (melody.freqForNote (noteVal)).real();
//        
////        std::cout << "controlled freq: " << freq << std::endl;
////        std::cout << "freq: " << melody.getControlledFrequency() << std::endl;
//        if (freq == melody.getControlledFrequency())
//        {
//            if (flip)
//            {
//                gain = setPoints[melody.getControlledFrequency()].getLowerBound();
//            }
//            else
//            {
//                gain = setPoints[melody.getControlledFrequency()].getUpperBound();
//            }
////            std::cout << "lower bound: " << gain << std::endl;
//        }
//        
//        Note note (gain, freq, 0.0);
//        noteSequence.addNote (note);
//    }
//    
//    // Then play the higher bound
//    for (int noteVal : melody.getNotes())
//    {
//        float freq = melody.freqForNote (noteVal);
//        float gain = curve.valueAtFrequency (melody.freqForNote (noteVal)).real();
//        
//        if (freq == melody.getControlledFrequency())
//        {
//            if (flip)
//            {
//                gain = setPoints[melody.getControlledFrequency()].getUpperBound();
//            }
//            else
//            {
//                gain = setPoints[melody.getControlledFrequency()].getLowerBound();
//            }
////            std::cout << "upper bound: " << gain << std::endl;
//        }
//        
//        Note note (gain, freq, 0.0);
//        noteSequence.addNote (note);
//    }
//    
//    // Move to that melody
//    sequencer.queueNextNoteSequence (noteSequence);
//}
//
//
