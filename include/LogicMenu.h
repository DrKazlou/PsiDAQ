#ifndef LogicMenu_H
#define LogicMenu_H

#include "FuncMain.h"
#include "DigitizerClass.h"
#include <TGFrame.h>
#include <TGTab.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGComboBox.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <RQ_OBJECT.h>

//#include <TSystem.h>
//#include <TSystemDirectory.h>
//#include <TEnv.h>
//#include <TFile.h>
//#include <TKey.h>
//#include <TGFontDialog.h>


class WindowMain;


class LogicMenu : public TGMainFrame{

private:
	TGMainFrame *fMain;
	WindowMain* fParent; 
	TGCompositeFrame     *fCF_couple[2], *fCF_board[2], *fCF;	
	TGTab *fTab;
	TGTextButton      *fSetButton[2], *fSwitchOffButton[2];
   	
	TGGroupFrame *fGF[2][2][8];	
	TGHorizontalFrame    *fHF0couple[2], *fHF0board[2], *fHFcouple[2][8], *fHFboard[2][8], *hf_main[2], *hf_add[2];
	TGVerticalFrame *vframe[2];
	TGCheckButton *fCcouple[2][8][8], *fCboard[2][8][14], *fCTrgIn[2][8];	
		
	TGLabel	*fLabel[2][2][14], *cInfo[2], *bInfo[2];
   
	TGNumberEntry	*fNumericEntries[2][16][16];
   
	TGTextEntry        *fTEntries[2][16][16];
	TGTextBuffer *tbuf[2][16][16];
	
	Digitizer *fDigitizer[2]; // Pointer to Digitizer instances
    V1730DigiConfig *fVcfg[2]; // Configuration per board
	uint32_t (*log_val)[2][8]; // Pointer to WindowMain's log_val array
	
	int fNch; // Number of channels per board
   	

public:
	LogicMenu(const TGWindow *p, WindowMain *parent, Digitizer *digi[2], V1730DigiConfig *vcfg[2]);	
   
	
	

   virtual ~LogicMenu();

	const char *sFont = "-Ubuntu-bold-r-*-*-16-*-*-*-*-*-iso8859-1";
	const char *labelFont = "-Ubuntu-bold-r-*-*-14-*-*-*-*-*-iso8859-1";
	const char *paramFont = "-adobe-courier-bold-r-*-*-15-*-*-*-*-*-iso8859-1";

	int Nch;

   // slots
	void TryToClose( );
	void CloseWindow( );
	void DoClose( );
	void DoSetVal( );
	void DoCheckBox( );
	void DoTab( );	
	void SetButton( );
	void SwitchOffButton( );
	
	ClassDef(LogicMenu, 1);
      
};

#endif