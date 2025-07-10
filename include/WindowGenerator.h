#ifndef WINDOWGENERATOR_H
#define WINDOWGENERATOR_H

#include "FuncMain.h"
#include <TGFrame.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <Rtypes.h>
#include <mutex>

class WindowMain;

class WindowGenerator : public TGMainFrame {
private:
    WindowMain* fParent;         // Parent GUI
    TGComboBox* fComboPolarity;  // Polarity combo box
    TGNumberEntry* fEntrySampleWidth; // Sample width entry
    TGNumberEntry* fEntryNSamples; // Number of samples entry
    TGNumberEntry* fEntryTraceLength; // Trace length (read-only)
    TGNumberEntry* fEntryTauRise; // Tau rise entry
    TGNumberEntry* fEntryTauDecay; // Tau decay entry
    TGComboBox* fComboDist;      // Combo box for distribution selection
    TGNumberEntry* fEntryFrequency; // Frequency entry (Hz)
    
    std::mutex fUpdateMutex;     // Mutex for synchronizing parameter updates
    bool fIsUpdating;            // Flag to prevent multiple simultaneous updates

public:
    WindowGenerator(const TGWindow* p, WindowMain* parent);
    ~WindowGenerator();
	
	void UpdateSignalParameters(const SignalParameters& params);
    void UpdateParameters();
    void UpdateTraceLength();
    void HandlePolarity(Int_t id);
    void HandleDist(Int_t id);
    void Close();
	
	
	
    ClassDef(WindowGenerator, 1)
};

#endif