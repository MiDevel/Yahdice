/*****************************************************************************
 *  
 *  PalmUtils.h - various PalmOS general-purpose utility functions
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


#define NUMGAMESOUNDS    6
#define MAXNOTESPERSOUND 4

typedef struct
{
  Long frequency;             // we don't need an amplitude value
  UInt duration;                // because of the Pilot's weak speaker
} NoteType;

typedef struct
{
  UInt numNotes;
  NoteType notes[MAXNOTESPERSOUND];
} SoundType;

typedef enum {
  sefSilentOK = 0,
  sefHighBeep,
  sefLowBeep,
  sefRoll,
  sefSelect,
  sefNewHighScore
} E_SoundEffect;


void    SoundEffect(E_SoundEffect sef, UInt sndVol);
void    SetFieldText(FieldPtr field_p, char *text_in_t, UInt length);
VoidPtr GetObjectPtr(Word objectID);
void    DrawTinyDigit(int x, int y, int num);
void    DrawTinyValue(int x, int y, int val);
int     SysRandomUnder(int num);
