/*****************************************************************************
 *  
 *  PalmUtils.c - various PalmOS general-purpose utility functions
 *
 *  Copyright (C) 2001,2002 Mirek Wójtowicz
 *
 *  Author:     Mirek Wójtowicz
 *  E-mail:     info@mirekw.com, mirwoj@life.pl
 *  Homepage:   http://www.mirekw.com/
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation;
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 *****************************************************************************/

#include <PalmOS.h>
#include <PalmCompatibility.h>

#include "PalmUtils.h"


SoundType SoundEffects[NUMGAMESOUNDS] =
  {
    1, {{8000, 2}},  // sefSilentOK
    1, {{4000, 20}}, // sefHighBeep
    1, {{150, 20}},  // sefLowBeep
    4, {{200, 5}, { 300, 5}, {250, 5}, { 200, 5}}, // sefRoll, thanks to Daniel McCarty (Groupper game)
    2, {{200, 10}, { 500, 10}}, // sefSelect
    4, {{1047, 90}, { 523, 90}, {784, 90}, {1047, 90}}  // sefNewHighScore, thanks to Daniel McCarty (Groupper game)
  };

//---------------------------------------------------------------------------
// Play one of game built-in sound effects
void SoundEffect(E_SoundEffect sef, UInt sndVol)
{
  int i;
  SndCommandType snd;

  if (sndVol > 0)
  {
    snd.cmd = sndCmdFreqDurationAmp;
    snd.param3 = sndVol; // sndDefaultAmp=64 - default - max. amplitude

    for (i = 0; i < SoundEffects[sef].numNotes; i++)
    {
      // note: put pause code in here if necessary (freq = 0)
      snd.param1 = SoundEffects[sef].notes[i].frequency;
      snd.param2 = SoundEffects[sef].notes[i].duration;
      SndDoCmd(0, &snd, true); // true: noWait
    }
  }
}
//---------------------------------------------------------------------------
void SetFieldText(FieldPtr field_p, char *text_in_t, UInt length)
{
  char *new_field_t;
  VoidHand old_handle;
  VoidHand new_handle;

  if (field_p == NULL)
    return;

  old_handle = (VoidHand) FldGetTextHandle(field_p);
  new_handle = MemHandleNew(length) ;
  new_field_t = MemHandleLock(new_handle);
  StrCopy(new_field_t, text_in_t);

  FldSetTextHandle(field_p, (Handle) new_handle);
  MemHandleUnlock(new_handle);

  if (old_handle == NULL)
    return;
  MemHandleFree(old_handle);
}
//---------------------------------------------------------------------------
// This routine returns a pointer to an object in the current form
VoidPtr GetObjectPtr(Word objectID)
{
	FormPtr frm;
	VoidPtr obj;

	frm = FrmGetActiveForm();
	obj = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, objectID));

	return obj;
}
//---------------------------------------------------------------------------
// Draw at (x,y) vectors specified as bits in (n).
void DrawTiny(int x, int y, int n)
{
  if (n & 1)  WinDrawLine(x,  y,  x+2,y  );
  if (n & 2)  WinDrawLine(x,  y,  x,  y+2);
  if (n & 4)  WinDrawLine(x+2,y,  x+2,y+2);
  if (n & 8)  WinDrawLine(x,  y+2,x+2,y+2);
  if (n & 16) WinDrawLine(x,  y+2,x,  y+4);
  if (n & 32) WinDrawLine(x+2,y+2,x+2,y+4);
  if (n & 64) WinDrawLine(x,  y+4,x+2,y+4);
}
//---------------------------------------------------------------------------
// Draw one tiny digit
void DrawTinyDigit(int x, int y, int num)
{
  const numbers[10] = { 119,36,93,109,46,107,123,37,127,111 };
  if ((num >= 0) && (num <= 9))
    DrawTiny(x, y, numbers[num]);
}
//---------------------------------------------------------------------------
// Draw one tiny number
void DrawTinyValue(int x, int y, int val)
{
  int rest = val;
  int dgt;
  int iCnt = 0;

  do
  {
    dgt = rest % 10;
    DrawTinyDigit(x - iCnt * 4, y, dgt);
    rest = rest / 10;
    iCnt++;
  } while (rest > 0);
}
//---------------------------------------------------------------------------
// Returns the random value in range 0..num-1
int SysRandomUnder(int num)
{
  return ( (Int)
           ( (DWord)
                 (   (DWord) SysRandom(0)*(DWord)(num)
                 ) / (DWord)((DWord)sysRandomMax+1)
           )
         );
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
