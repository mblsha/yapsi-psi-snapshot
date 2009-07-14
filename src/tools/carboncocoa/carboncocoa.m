/*
 * cocoautil.mm - Qt-Cocoa integration
 * Copyright (C) 2008  Yandex LLC (Michail Pishchagin)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "carboncocoa.h"

#include <AppKit/AppKit.h>

NSAutoreleasePool* localPool = 0;

// copied from sparkleplus, SUUtilities.m
const char* ownSUCurrentSystemVersionString()
{
	static NSString *currentSystemVersion = nil;
	if (!currentSystemVersion) {
		// OS version (Apple recommends using SystemVersion.plist instead of Gestalt() here, don't ask me why).
		// This code *should* use NSSearchPathForDirectoriesInDomains(NSCoreServiceDirectory, NSSystemDomainMask, YES)
		// but that returns /Library/CoreServices for some reason
		NSString *versionPlistPath = @"/System/Library/CoreServices/SystemVersion.plist";
		//gets a version string of the form X.Y.Z
currentSystemVersion = [[[NSDictionary dictionaryWithContentsOfFile:versionPlistPath] objectForKey: @"ProductVersion"] retain];
	}

	return [currentSystemVersion UTF8String];
}
