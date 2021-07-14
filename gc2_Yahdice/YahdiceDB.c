/*****************************************************************************
 *  Yahdice
 *  YahdiceDB.c - Yahdice databases
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

#include "../gc2_common/PalmUtils.h"

#include "YahdiceDef.h"

#define __YAHDICEDB_C__
#include "YahdiceDB.h"


//---------------------------------------------------------------------------
// Read game state from the database
Boolean ReadGameState(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_GameState *locGameState;

  TheGame.isFinished = true; // safe assumption

  dbRef = DmOpenDatabaseByTypeCreator(DBID_GameState, ThisAppID, dmModeReadWrite);
  if (!dbRef) // if database does not exist create it
  {
    UInt index = 0;

    if (DmCreateDatabase(0, DBNam_GameState, ThisAppID, DBID_GameState, false))
      return false; // error
    dbRef = DmOpenDatabaseByTypeCreator(DBID_GameState, ThisAppID, dmModeReadWrite);
    recHandle = DmNewRecord(dbRef, &index, sizeof(T_GameState));
    locGameState = MemHandleLock(recHandle);
    DmWrite(locGameState, 0, &TheGame, sizeof(T_GameState));
  }
  else
  {
    recHandle = DmGetRecord(dbRef, 0);
    locGameState = MemHandleLock(recHandle);
    MemMove(&TheGame, locGameState, sizeof(T_GameState));
  }
  MemPtrUnlock(locGameState);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  CrrPlayer = &(TheGame.players[TheGame.crrPlayerIdx]);

  return true;
}
//---------------------------------------------------------------------------
// Write game state to the database for future use
Boolean WriteGameState(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_GameState *locGameState;
  int i;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_GameState, ThisAppID, dmModeReadWrite);
  recHandle = DmGetRecord(dbRef, 0);
  locGameState = MemHandleLock(recHandle);

  DmWrite(locGameState, 0, &TheGame, sizeof(T_GameState));

  MemPtrUnlock(locGameState);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Read last players from the database
Boolean ReadAvailPlayers(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_AvailPlayers *locPlayers;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_AvailPlayers, ThisAppID, dmModeReadWrite);

  if (!dbRef) // if database does not exist create it
  {
    UInt index = 0;

    if (DmCreateDatabase(0, DBNam_AvailPlayers, ThisAppID, DBID_AvailPlayers, false))
      return false; // error
    dbRef = DmOpenDatabaseByTypeCreator(DBID_AvailPlayers, ThisAppID, dmModeReadWrite);
    recHandle = DmNewRecord(dbRef, &index, sizeof(T_AvailPlayers));
    locPlayers = MemHandleLock(recHandle);
    DmWrite(locPlayers, 0, &AvailPlayers, sizeof(T_AvailPlayers));
  }
  else
  {
    recHandle = DmGetRecord(dbRef, 0);
    locPlayers = MemHandleLock(recHandle);
    MemMove(&AvailPlayers, locPlayers, sizeof(T_AvailPlayers));
  }
  MemPtrUnlock(locPlayers);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Write defined players to the database for future use
Boolean WriteAvailPlayers(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_AvailPlayers *locPlayers;
  int i;

  for (i = 0; i < MAXPLAYERS; i++)
  {
    StrNCopy(AvailPlayers.name[i], FldGetTextPtr(fldptr_menunames[i]), PLAYERNAMELEN);
    AvailPlayers.check[i] = CtlGetValue(ctlptr_menucheck[i]) == 1;
    AvailPlayers.ai[i] = false;
  }

  dbRef = DmOpenDatabaseByTypeCreator(DBID_AvailPlayers, ThisAppID, dmModeReadWrite);
  recHandle = DmGetRecord(dbRef, 0);
  locPlayers = MemHandleLock(recHandle);

  DmWrite(locPlayers, 0, &AvailPlayers, sizeof(T_AvailPlayers));

  MemPtrUnlock(locPlayers);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Read preferences from the database
Boolean ReadPreferences(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_Preferences *locPreferences;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_Preferences, ThisAppID, dmModeReadWrite);

  if (!dbRef) // if database does not exist create it
  {
    UInt index = 0;

    if (DmCreateDatabase(0, DBNam_Preferences, ThisAppID, DBID_Preferences, false))
      return false; // error
    dbRef = DmOpenDatabaseByTypeCreator(DBID_Preferences, ThisAppID, dmModeReadWrite);
    recHandle = DmNewRecord(dbRef, &index, sizeof(T_Preferences));
    locPreferences = MemHandleLock(recHandle);
    DmWrite(locPreferences, 0, &Preferences, sizeof(T_Preferences));
  }
  else
  {
    recHandle = DmGetRecord(dbRef, 0);
    locPreferences = MemHandleLock(recHandle);
    MemMove(&Preferences, locPreferences, sizeof(T_Preferences));
  }
  MemPtrUnlock(locPreferences);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Write defined players to the database for future use
Boolean WritePreferences(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_Preferences *locPreferences;
  int i;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_Preferences, ThisAppID, dmModeReadWrite);
  recHandle = DmGetRecord(dbRef, 0);
  locPreferences = MemHandleLock(recHandle);

  DmWrite(locPreferences, 0, &Preferences, sizeof(T_Preferences));

  MemPtrUnlock(locPreferences);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Read the Hall of Fame from the database
Boolean ReadHallOfFame(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_HallItem *locHall;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_HallOfFame, ThisAppID, dmModeReadWrite);
  if (!dbRef) // if database does not exist create it
  {
    UInt index = 0;

    if (DmCreateDatabase(0, DBNam_HallOfFame, ThisAppID, DBID_HallOfFame, false))
      return false; // error
    dbRef = DmOpenDatabaseByTypeCreator(DBID_HallOfFame, ThisAppID, dmModeReadWrite);

    // save default results
    InitMemHallOfFame(0); // create safe defaults
    recHandle = DmNewRecord(dbRef, &index, sizeof(T_HallItem)*MAXHALLITEMS);
    locHall = MemHandleLock(recHandle);
    DmWrite(locHall, 0, &HallOfFame, sizeof(T_HallItem)*MAXHALLITEMS);
  }
  else
  {
    recHandle = DmGetRecord(dbRef, 0);
    locHall = MemHandleLock(recHandle);
    MemMove(&HallOfFame, locHall, sizeof(T_HallItem)*MAXHALLITEMS);
  }
  MemPtrUnlock(locHall);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Write the Hall of Fame to the database
Boolean WriteHallOfFame(void)
{
  DmOpenRef dbRef;
  VoidHand  recHandle;
  T_HallItem *locHall;

  dbRef = DmOpenDatabaseByTypeCreator(DBID_HallOfFame, ThisAppID, dmModeReadWrite);

  recHandle = DmGetRecord(dbRef, 0);
  locHall = MemHandleLock(recHandle);
  DmWrite(locHall, 0, &HallOfFame, sizeof(T_HallItem)*MAXHALLITEMS);

  MemPtrUnlock(locHall);
  DmReleaseRecord(dbRef, 0, false);
  DmCloseDatabase(dbRef);

  return true;
}
//---------------------------------------------------------------------------
// Create a new note in Memo Pad containing the given text
// QQtodo: Check for the string length does not exceed max size allowed in one memopad record
#define memoPadDBType   'DATA'
#define memoPadMaxChar  4000
Err WriteMemoRecord(Char* szData)
{
  Ptr pRec;
  VoidHand memoRec;
  Err error;
  DmOpenRef pdbMemoPad;
  UInt16 index;

  pdbMemoPad = DmOpenDatabaseByTypeCreator(memoPadDBType, sysFileCMemo, dmModeReadWrite);

  index = 0; // at the beginning
  memoRec = DmNewRecord(pdbMemoPad, &index, StrLen(szData) + 1); // create a new record in the database

  pRec = (Char*)MemHandleLock(memoRec); // lock down the block containing the new record

  error = DmStrCopy(pRec, 0, szData); // write record
  if (error != 0)
  {
    DmCloseDatabase(pdbMemoPad); // close the application's database
    return error;
  }

  error = MemPtrUnlock(pRec); // unlock the block of the new record
  if (error != 0)
  {
    DmCloseDatabase(pdbMemoPad); // Close the application's database
    return error;
  }

  error = DmReleaseRecord(pdbMemoPad, index, true);
  if (error != 0)
  {
    DmCloseDatabase(pdbMemoPad); // Close the application's database.
    return error;
  }

  DmCloseDatabase(pdbMemoPad); // close the application's database

  return 0;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
