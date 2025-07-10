#include "WindowGenerator.h"
#include "WindowMain.h"
#include <TGClient.h>
#include <TGFrame.h>
#include <TGNumberEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>

WindowGenerator::WindowGenerator(const TGWindow* p, WindowMain* parent)
    : TGMainFrame(p, 300, 500), fParent(parent), fIsUpdating(false) {
    SetCleanup(kDeepCleanup);
    SetWindowName("Configure Signal Parameters");
    Connect("CloseWindow()", "WindowGenerator", this, "Close()");

    TGVerticalFrame* vFrame = new TGVerticalFrame(this, 300, 500);

    TGLabel* labelPolarity = new TGLabel(vFrame, "Polarity:");
    vFrame->AddFrame(labelPolarity, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fComboPolarity = new TGComboBox(vFrame);
    fComboPolarity->AddEntry("Negative", 0);
    fComboPolarity->AddEntry("Positive", 1);
    fComboPolarity->Select(fParent->GetSignalParameters().polarity == -1 ? 0 : 1);
    fComboPolarity->Connect("Selected(Int_t)", "WindowGenerator", this, "HandlePolarity(Int_t)");
    fComboPolarity->Resize(150, 30);
    vFrame->AddFrame(fComboPolarity, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelSampleWidth = new TGLabel(vFrame, "Sample Width (ns):");
    vFrame->AddFrame(labelSampleWidth, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntrySampleWidth = new TGNumberEntry(vFrame, fParent->GetSignalParameters().sampleWidth, 5, -1,
                                         TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive);
    fEntrySampleWidth->Connect("ValueSet(Long_t)", "WindowGenerator", this, "UpdateParameters()");
    vFrame->AddFrame(fEntrySampleWidth, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelNSamples = new TGLabel(vFrame, "Number of Samples:");
    vFrame->AddFrame(labelNSamples, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntryNSamples = new TGNumberEntry(vFrame, fParent->GetSignalParameters().nSamples, 5, -1,
                                       TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive);
    fEntryNSamples->Connect("ValueSet(Long_t)", "WindowGenerator", this, "UpdateParameters()");
    vFrame->AddFrame(fEntryNSamples, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelTraceLength = new TGLabel(vFrame, "Trace Length (ns):");
    vFrame->AddFrame(labelTraceLength, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntryTraceLength = new TGNumberEntry(vFrame, fParent->GetSignalParameters().nSamples * fParent->GetSignalParameters().sampleWidth,
                                         5, -1, TGNumberFormat::kNESReal);
    fEntryTraceLength->SetState(kFALSE);
    vFrame->AddFrame(fEntryTraceLength, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelTauRise = new TGLabel(vFrame, "Tau Rise (ns):");
    vFrame->AddFrame(labelTauRise, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntryTauRise = new TGNumberEntry(vFrame, fParent->GetSignalParameters().tauRise, 5, -1,
                                      TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive);
    fEntryTauRise->Connect("ValueSet(Long_t)", "WindowGenerator", this, "UpdateParameters()");
    vFrame->AddFrame(fEntryTauRise, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelTauDecay = new TGLabel(vFrame, "Tau Decay (ns):");
    vFrame->AddFrame(labelTauDecay, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntryTauDecay = new TGNumberEntry(vFrame, fParent->GetSignalParameters().tauDecay, 5, -1,
                                       TGNumberFormat::kNESReal, TGNumberFormat::kNEAPositive);
    fEntryTauDecay->Connect("ValueSet(Long_t)", "WindowGenerator", this, "UpdateParameters()");
    vFrame->AddFrame(fEntryTauDecay, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelDist = new TGLabel(vFrame, "Distribution:");
    vFrame->AddFrame(labelDist, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fComboDist = new TGComboBox(vFrame);
    fComboDist->AddEntry("Uniform", 0);
    fComboDist->AddEntry("Gaussian", 1);
    fComboDist->AddEntry("Poisson", 2);
    fComboDist->AddEntry("Na-22", 3);
    fComboDist->AddEntry("Cs-137", 4);
    fComboDist->AddEntry("Co-60", 5);
    fComboDist->AddEntry("Ra-226", 6);
    fComboDist->Select(fParent->GetSignalParameters().distMode);
    fComboDist->Connect("Selected(Int_t)", "WindowGenerator", this, "HandleDist(Int_t)");
    fComboDist->Resize(150, 30);
    vFrame->AddFrame(fComboDist, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    TGLabel* labelFrequency = new TGLabel(vFrame, "Frequency (Hz):");
    vFrame->AddFrame(labelFrequency, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));
    fEntryFrequency = new TGNumberEntry(vFrame, fParent->GetSignalParameters().frequency, 5, -1,
                                        TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive);
    fEntryFrequency->Connect("ValueSet(Long_t)", "WindowGenerator", this, "UpdateParameters()");
    vFrame->AddFrame(fEntryFrequency, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 5));

    AddFrame(vFrame, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
    MapSubwindows();
    Resize();
    MapWindow();

    UpdateTraceLength();
    UpdateParameters();
}

WindowGenerator::~WindowGenerator() {
    printf("WindowGenerator::~WindowGenerator: Cleaning up\n");
    if (fParent) {
        fParent->SignalDialogClosed();
        printf("WindowGenerator::~WindowGenerator: Notified parent of dialog closure\n");
    } else {
        printf("WindowGenerator::~WindowGenerator: Parent already null\n");
    }
}

void WindowGenerator::HandlePolarity(Int_t id) {
    UpdateParameters();
}

void WindowGenerator::HandleDist(Int_t id) {
    UpdateParameters();
}

void WindowGenerator::UpdateParameters() {
    std::lock_guard<std::mutex> lock(fUpdateMutex);
    if (fIsUpdating) return;
    fIsUpdating = true;

    SignalParameters params;
    params.polarity = (fComboPolarity->GetSelected() == 0) ? -1 : 1;
    params.sampleWidth = fEntrySampleWidth->GetNumber();
    params.nSamples = fEntryNSamples->GetIntNumber();
    params.tauRise = fEntryTauRise->GetNumber();
    params.tauDecay = fEntryTauDecay->GetNumber();
    params.distMode = fComboDist->GetSelected();
    params.frequency = fEntryFrequency->GetIntNumber();

    fParent->UpdateSignalParameters(params);
    UpdateTraceLength();

    fIsUpdating = false;
}

void WindowGenerator::UpdateTraceLength() {
    Double_t traceLength = fEntryNSamples->GetIntNumber() * fEntrySampleWidth->GetNumber();
    fEntryTraceLength->SetNumber(traceLength);
}

void WindowGenerator::Close() {
    DeleteWindow();
}