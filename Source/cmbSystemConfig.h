//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME SystemConfig.h - An .h included by every one for build configuration/macros.
// .SECTION Description
// .SECTION See Also

#ifndef __SystemConfig_h
#define __SystemConfig_h

//Windows specific stuff
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__)
#if !defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#pragma warning(disable : 4251) /* missing DLL-interface */
#endif                          //!defined(SMTK_DISPLAY_INGORED_WIN_WARNINGS)
#endif                          //Windows specific stuff

#endif //__SystemConfig_h
