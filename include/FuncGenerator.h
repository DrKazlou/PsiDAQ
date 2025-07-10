#ifndef FUNCGENERATOR_H
#define FUNCGENERATOR_H

#include <vector>
#include <TRandom3.h>
#include <TH1D.h>

class WindowMain;

Double_t ComptonEnergy(Double_t E, Double_t cosTheta, TRandom3& rand);
Double_t KleinNishina(Double_t E, Double_t cosTheta);
Double_t SampleComptonEnergy(Double_t E, TRandom3& rand);
Double_t EnergyResolution(Double_t E);
Double_t GenerateAmplitude(Int_t distMode, TRandom3& rand, WindowMain* gui);
Double_t GenerateNa22(TRandom3& rand, WindowMain* gui);
Double_t GenerateCs137(TRandom3& rand, WindowMain* gui);
Double_t GenerateCo60(TRandom3& rand, WindowMain* gui);
Double_t GenerateRa226(TRandom3& rand, WindowMain* gui);
std::vector<Double_t> GenerateSignal(Int_t nSamples, Double_t sampleWidth,
                                                       Double_t tauRise, Double_t tauDecay,
                                                       Int_t polarity, Int_t distMode,
                                                       TRandom3& rand,
                                                       WindowMain* gui);

#endif