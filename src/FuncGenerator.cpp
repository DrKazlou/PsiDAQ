#include "FuncGenerator.h"
#include "WindowMain.h"
#include <TMath.h>
#include <vector>
#include <algorithm>

Double_t ComptonEnergy(Double_t E, Double_t cosTheta, TRandom3& rand) {
    const Double_t mec2 = 511.0;
    return E / (1 + (E / mec2) * (1 - cosTheta)) / 1000.0;
}

Double_t KleinNishina(Double_t E, Double_t cosTheta) {
    const Double_t mec2 = 511.0;
    Double_t E_prime = E / (1 + (E / mec2) * (1 - cosTheta));
    Double_t ratio = E_prime / E;
    return ratio * ratio * (ratio + 1 / ratio - 1 + cosTheta * cosTheta);
}

Double_t SampleComptonEnergy(Double_t E, TRandom3& rand) {
    Double_t cosTheta;
    Double_t max_prob = KleinNishina(E, 1.0);
    do {
        cosTheta = rand.Uniform(-1.0, 1.0);
        Double_t prob = KleinNishina(E, cosTheta);
        if (rand.Uniform(0.0, max_prob) < prob) {
            return ComptonEnergy(E, cosTheta, rand);
        }
    } while (true);
}

Double_t EnergyResolution(Double_t E) {
    return 6.4 * TMath::Sqrt(E / 1274.5);
}

Double_t GenerateAmplitude(Int_t distMode, TRandom3& rand, WindowMain* gui) {
    switch (distMode) {
        case 0: return rand.Uniform(0.0, 1.0); // Uniform
        case 1: return TMath::Max(0.0, TMath::Min(1.0, rand.Gaus(0.5, 0.15))); // Gaussian
        case 2: return TMath::Min(1.0, rand.Poisson(5.0) / 10.0); // Poisson
        case 3: return GenerateNa22(rand, gui); // Na-22
        case 4: return GenerateCs137(rand, gui); // Cs-137
        case 5: return GenerateCo60(rand, gui); // Co-60
        case 6: return GenerateRa226(rand, gui); // Ra-226
        default: return rand.Uniform(0.0, 1.0);
    }
}

Double_t GenerateNa22(TRandom3& rand, WindowMain* gui) {
    Double_t r = rand.Uniform(0.0, 1.0);
    if (r < 0.4) {
        return rand.Gaus(0.511, EnergyResolution(511.0) / 1000.0 / 2.355);
    } else if (r < 0.75) {
        return rand.Gaus(1.2745, EnergyResolution(1274.5) / 1000.0 / 2.355);
    } else if (r < 0.85) {
        return rand.Gaus(0.09, EnergyResolution(90.0) / 1000.0 / 2.355);
    } else {
        return SampleComptonEnergy(1274.5, rand);
    }
}

Double_t GenerateCs137(TRandom3& rand, WindowMain* gui) {
    Double_t r = rand.Uniform(0.0, 1.0);
    if (r < 0.7) {
        return rand.Gaus(0.6617, EnergyResolution(661.7) / 1000.0 / 2.355);
    } else if (r < 0.85) {
        return rand.Gaus(0.032, EnergyResolution(32.0) / 1000.0 / 2.355);
    } else {
        return SampleComptonEnergy(661.7, rand);
    }
}

Double_t GenerateCo60(TRandom3& rand, WindowMain* gui) {
    Double_t r = rand.Uniform(0.0, 1.0);
    if (r < 0.35) {
        return rand.Gaus(1.1732, EnergyResolution(1173.2) / 1000.0 / 2.355);
    } else if (r < 0.7) {
        return rand.Gaus(1.3325, EnergyResolution(1332.5) / 1000.0 / 2.355);
    } else if (r < 0.85) {
        return rand.Gaus(0.015, EnergyResolution(15.0) / 1000.0 / 2.355);
    } else {
        return SampleComptonEnergy(1332.5, rand);
    }
}

Double_t GenerateRa226(TRandom3& rand, WindowMain* gui) {
    Double_t r = rand.Uniform(0.0, 1.0);
    if (r < 0.4) {
        return rand.Gaus(0.6093, EnergyResolution(609.3) / 1000.0 / 2.355);
    } else if (r < 0.65) {
        return rand.Gaus(1.7645, EnergyResolution(1764.5) / 1000.0 / 2.355);
    } else if (r < 0.85) {
        return rand.Gaus(0.09, EnergyResolution(90.0) / 1000.0 / 2.355);
    } else {
        return SampleComptonEnergy(1764.5, rand);
    }
}

std::vector<Double_t> GenerateSignal(Int_t nSamples, Double_t sampleWidth,
                                                       Double_t tauRise, Double_t tauDecay,
                                                       Int_t polarity, Int_t distMode,
                                                       TRandom3& rand,
                                                       WindowMain* gui) {
    // Only reset and fill h_trace if it's not nullptr
    //if (h_trace) {
    //    h_trace->Reset();
    //}
    std::vector<Double_t> signal(nSamples, 0.0);
    
    Double_t amplitude = GenerateAmplitude(distMode, rand, gui);

    for (Int_t i = 0; i < nSamples; ++i) {
        Double_t t = i * sampleWidth;
        Double_t s = 0.0;
        if (t >= 0) {
            s = amplitude * (TMath::Exp(-t / tauDecay) - TMath::Exp(-t / tauRise));
        }
        s *= polarity;
        s += rand.Gaus(0.0, 0.01);
        signal[i] = s;
       // if (h_trace) { // Only fill h_trace if it exists
       //     h_trace->SetBinContent(i + 1, s);
       // }
        
    }

    return signal;
}