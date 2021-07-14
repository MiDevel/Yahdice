/*****************************************************************************
 *  Yahdice
 *  YahdiceDB.h - Yahdice databases
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

#ifndef __YAHDICEDB_H__
#define __YAHDICEDB_H__

#define DBID_GameState    'game'
#define DBID_AvailPlayers 'nams'
#define DBID_HallOfFame   'hall'
#define DBID_Preferences  'pref'

#define DBNam_GameState    "Yahdice_GameState"
#define DBNam_AvailPlayers "Yahdice_AvailPlayers"
#define DBNam_HallOfFame   "Yahdice_HallOfFame"
#define DBNam_Preferences  "Yahdice_Preferences"

Boolean ReadGameState(void);
Boolean WriteGameState(void);
Boolean ReadAvailPlayers(void);
Boolean WriteAvailPlayers(void);
Boolean ReadHallOfFame(void);
Boolean WriteHallOfFame(void);
Boolean ReadPreferences(void);
Boolean WritePreferences(void);
//Int     ExportToMemoPad(VoidHand hndExportText);
Err     WriteMemoRecord(Char* szData);


#endif
