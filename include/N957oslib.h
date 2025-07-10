////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file    N957oslib.h
*   \brief   Operating system dependent definitions
*   \author  CAEN S.p.A. ( http://www.caen.it )
*   \version 1.2
*   \date    05/2006
*            
*/
////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __N957OSLIB_DEF_H
#define __N957OSLIB_DEF_H

////////////////////////////////////////////
// File includes
////////////////////////////////////////////

////////////////////////////////////////////
// Global defines
////////////////////////////////////////////
#if defined(WIN32)
	
	/*!
	* Win 32 section
	*/
	#pragma once


	#define WIN32_LEAN_AND_MEAN		/* !< /brief Exclude rarely-used stuff from Windows headers */
	/*!
	* Windows Header Files:
	*/
	#include <windows.h>
	#include <winioctl.h>

	#define N957_API N957ErrorCodes __stdcall

#elif defined(LINUX)

	/*!
	* Linux section
	*/
	#define N957_API N957ErrorCodes 

#else
	#error No operating system defined
#endif


#endif 
