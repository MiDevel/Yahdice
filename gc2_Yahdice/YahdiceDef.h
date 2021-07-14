/*****************************************************************************
 *  Yahdice v.2.00
 *  YahdiceDef.h - The main module header file
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


#define ThisAppID          'm_YA'

#define MAXPLAYERS 4
#define PLAYERNAMELEN 20

#define MAXHALLITEMS 8

#define ITEMS_COUNT   13
#define ITM_ONES       0
#define ITM_TWOS       1
#define ITM_THREES     2
#define ITM_FOURS      3
#define ITM_FIVES      4
#define ITM_SIXES      5
#define ITM_THREE      6
#define ITM_FOUR       7
#define ITM_FULLHOUSE  8
#define ITM_SMALLSTR   9
#define ITM_LARGESTR  10
#define ITM_YAHDICE   11
#define ITM_CHANCE    12

#define DIE_1 0
#define DIE_2 1
#define DIE_3 2
#define DIE_4 3
#define DIE_5 4

#define ITEM_FREE 1
#define ITEM_USED 0

#define FORM_MAIN 1
#define FORM_MENU 2
#define FORM_HALL 3
#define FORM_ABOUT 4
#define FORM_PREFS 5


void InitMemHallOfFame(int leaveCnt);


// Available players structure
typedef struct
{
  char name[MAXPLAYERS][PLAYERNAMELEN + 1]; // player's name
  Boolean check[MAXPLAYERS];                // is checked for playing?
  Boolean ai[MAXPLAYERS];                   // is it a Palm player?
} T_AvailPlayers;

// Hall of Fame item
typedef struct
{
  char name[PLAYERNAMELEN + 1]; // player's name
  int  score;                   // player's score
  DateTimeType date;            // when was it
} T_HallItem;

// Program preferences
typedef struct
{
  Boolean rollAnimate; // animate dice while rolling?
  Boolean autoSort;    // auto-sorting after each rolling?
  Boolean isPause;     // pause between players change
  int     pauseLen;    // pause length (in secs)
  Boolean showHints;   // should hints be displayed?
  Boolean allHints;    // hints also for scored items?
  Boolean categHints;  // show category hints?
  Boolean advUser;     // advanced user?
  Boolean markKeep;    // mark dice to be kept?
  Boolean bdmy5;
  int     idmy1;
  int     idmy2;
  int     idmy3;
  int     idmy4;
  int     idmy5;
} T_Preferences;

// One Yahdice item - Ones, FullHouse, etc.
typedef struct
{
  Boolean used; // used (scored) item?
  int score;    // scored value
  int hint;     // virtual score, hint
} T_OneItem;

// Full state of one player
typedef struct
{
  char      name[PLAYERNAMELEN + 1]; // player's name
  T_OneItem items[ITEMS_COUNT]; // from 1's up to Chance
  int       diceState[5];  // state of all 5 dice: 0, 1..6 0 - unknown yet
  Boolean   dicePicked[5]; // should the die be rolled?
  int       rollsCnt; // how many times did we roll? 0..2
  int       roundCnt; // how many rounds did we play? 0..13
  int       subtotal; // total of 6 left items
  int       totScore; // total score
} T_OnePlayer;

// Full state of the game
typedef struct
{
  T_OnePlayer players[MAXPLAYERS];
  int         playersCount;
  int         crrPlayerIdx;
  Boolean     areHintsShown;
  Boolean     isFinished;
} T_GameState;


// global variables
#ifdef __YAHDICE_C__
  T_GameState TheGame;
  T_AvailPlayers AvailPlayers =
                    {
                      { "Player 1", "Player 2", "Player 3", "Player 4" },
                      { true, false, false, false },
                      { false, false, false, false }
                    };
  T_Preferences Preferences;
  T_HallItem HallOfFame[MAXHALLITEMS] =
                    {
                      { "", 0, 0 }, { "", 0, 0 }, { "", 0, 0 }, { "", 0, 0 },
                      { "", 0, 0 }, { "", 0, 0 }, { "", 0, 0 }, { "", 0, 0 }
                    };
  T_OnePlayer* CrrPlayer;

  // Pointers to frmMenu UI elements
  FieldPtr   fldptr_menunames[MAXPLAYERS];
  ControlPtr ctlptr_menucheck[MAXPLAYERS];
#else
  extern T_GameState TheGame;
  extern T_AvailPlayers AvailPlayers;
  extern T_Preferences Preferences;
  extern T_HallItem HallOfFame[];
  extern T_OnePlayer* CrrPlayer;
  extern FieldPtr   fldptr_menunames[];
  extern ControlPtr ctlptr_menucheck[];
#endif

