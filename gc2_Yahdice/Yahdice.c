/*****************************************************************************
 *  Yahdice v.2.00
 *  Yahdice.c - The main module
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

#define __YAHDICE_C__
#include "Yahdice.h"

#include "YahdiceDef.h"
#include "YahdiceDB.h"


static int  frmMenuHdlr(EventType *e);
static int  frmMainHdlr(EventType *e);
static int  frmAboutHdlr(EventType *e);
static int  frmHallHdlr(EventType *e);
static int  frmPrefsHdlr(EventType *e);
static int  StartApplication(void);
static void StopApplication(void);
static void EventLoop(void);
static void InitializeMainForm(void);
static void InitializeMenuForm(void);
static void InitializeHallForm(void);
static void InitializePrefsForm(void);
static void ApplyPrefsForm(void);

static void DisplayHallOfFame(void);
static Boolean AddToHallOfFame(int idx);
static void ClearHallOfFame(int leaveCnt);

static void StartNewGame(Boolean fShow);
static void UndoLastScoring(void);
static void RollDice(void);
static void SetAndShowScore(int index, int iVal);
static void CalcScoreHints(void);
static void ShowScoreHints(Boolean fShow);
static void DrawAll(void);
static void ScoreSelected(int item);
static void ShowTotals(void);
static Boolean ShowCurrentScores(Boolean ifGameOver);
static void SubmitHighScores(void);


// Module globals
static T_GameState GameSnap;
static Boolean CanUndo = false;
static UInt16  SndVolume = 16; // games sound amplitude

static int     HallReturnsTo = FORM_MENU;
static Boolean isOS20;       // os 2.0 or higher
static Boolean isOS30;       // os 3.0 or higher
static Boolean isOS35;       // os 3.5 or higher
static Boolean isColor;      // color device

// pointers to frmMain UI elements
static FormPtr    frmptr_Main = NULL;
static FieldPtr   fldptr_player;
static ControlPtr ctlptr_button[ITEMS_COUNT];
static FieldPtr   fldptr_score[ITEMS_COUNT];
static FieldPtr   fldptr_prompt;
static ControlPtr ctlptr_Roll01, ctlptr_Roll02, ctlptr_Roll03;
static ControlPtr ctlptr_btnScores;
static FieldPtr   fldptr_total;
static FieldPtr   fldptr_bonus;
static FieldPtr   fldptr_totScore;

// Pointers to frmHall UI elements
static FieldPtr   fldptr_fldScoresToLeave;
static FieldPtr   fldptr_HallPlayer[MAXHALLITEMS];
static FieldPtr   fldptr_HallScore[MAXHALLITEMS];
static FieldPtr   fldptr_HallDate[MAXHALLITEMS];


//---------------------------------------------------------------------------
// Paint the face of a die
void DrawDieBitmap(int index)
{
  VoidHand  hbmp;
  BitmapPtr pbmp;
  int x, y;
  RectangleType  bounds; // die area
  Word           gadIndex;
  int            val = CrrPlayer->diceState[index];
  Boolean        act = CrrPlayer->dicePicked[index];

  switch (index)
  {
    case DIE_1: gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie1); break;
    case DIE_2: gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie2); break;
    case DIE_3: gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie3); break;
    case DIE_4: gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie4); break;
    case DIE_5: gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie5); break;
    default:    gadIndex = FrmGetObjectIndex(frmptr_Main, gadDie1); break;
  }
  FrmGetObjectBounds(frmptr_Main, gadIndex, &bounds);

  switch (val)
  {
    case  0: hbmp=DmGetResource('Tbmp', gbmp1_die0); break;
    case  1: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die1_0 : gbmp1_die1); break;
    case  2: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die2_0 : gbmp1_die2); break;
    case  3: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die3_0 : gbmp1_die3); break;
    case  4: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die4_0 : gbmp1_die4); break;
    case  5: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die5_0 : gbmp1_die5); break;
    case  6: hbmp=DmGetResource('Tbmp', (act && !Preferences.markKeep) ? gbmp1_die6_0 : gbmp1_die6); break;
    case 11: hbmp=DmGetResource('Tbmp', gbmp_dieanim01); break;
    case 12: hbmp=DmGetResource('Tbmp', gbmp_dieanim02); break;
    case 13: hbmp=DmGetResource('Tbmp', gbmp_dieanim03); break;
    case 14: hbmp=DmGetResource('Tbmp', gbmp_dieanim04); break;
    default: hbmp=DmGetResource('Tbmp', gbmp1_die0); break;
  }

  pbmp=MemHandleLock(hbmp);

  WinDrawBitmap(pbmp, bounds.topLeft.x, bounds.topLeft.y);
  MemHandleUnlock(hbmp);
  DmReleaseResource(hbmp);

  if ((val >= 1) && (val <= 6) && (act && Preferences.markKeep))
  {
    // mark the dice to be kept - fill its frame
    int x1, y1, x2, y2;

    if (isColor)
    {
      WinPushDrawState();
      WinSetForeColor(94);
    }
    x1 = bounds.topLeft.x + 1;
    y1 = bounds.topLeft.y + 1;
    x2 = bounds.topLeft.x + bounds.extent.x - 2;
    y2 = bounds.topLeft.y + bounds.extent.y - 2;
    WinDrawLine(x1, y1+1, x1, y2-1); WinDrawLine(x1+1, y1+1, x1+1, y2-1); // left
    WinDrawLine(x1+1, y1, x2-1, y1); WinDrawLine(x1+1, y1+1, x2-1, y1+1); // top
    WinDrawLine(x1+1, y2, x2-1, y2); WinDrawLine(x1+1, y2-1, x2-1, y2-1); // bottom
    WinDrawLine(x2, y1+1, x2, y2-1); WinDrawLine(x2-1, y1+1, x2-1, y2-1); // right
    if (isOS35)
    {
      WinDrawPixel(x1+1, y1+1);
      WinDrawPixel(x2-1, y1+1);
      WinDrawPixel(x1+1, y2-1);
      WinDrawPixel(x2-1, y2-1);
    }
    else
    {
      WinDrawLine(x1+1, y1+1, x1+1, y1+1);
      WinDrawLine(x2-1, y1+1, x2-1, y1+1);
      WinDrawLine(x1+1, y2-1, x1+1, y2-1);
      WinDrawLine(x2-1, y2-1, x2-1, y2-1);
    }
    if (isColor)
    {
      WinPopDrawState();
    }
  }
}
//---------------------------------------------------------------------------
// Show the count of performed rolls - update the roll number indicator
void MarkRollNum(void)
{
  CtlSetValue(ctlptr_Roll01, CrrPlayer->rollsCnt > 0 ? 1 : 0);
  CtlSetValue(ctlptr_Roll02, CrrPlayer->rollsCnt > 1 ? 1 : 0);
  CtlSetValue(ctlptr_Roll03, CrrPlayer->rollsCnt > 2 ? 1 : 0);
}
//---------------------------------------------------------------------------
// Entry point for the Pilot-API
DWord PilotMain(Word action, Ptr cmdPBP, Word launchFlags)
{
  int error;

  // check the launch coded here
  if (action != sysAppLaunchCmdNormalLaunch)
  return 0;

  // Application start code
  error = StartApplication();
  if (error) return error;

  // start event proessing
  EventLoop();

  // perform program termination code here
  StopApplication();
}
//---------------------------------------------------------------------------
// The main event loop
static void EventLoop(void)
{
  short err;
  int formID;
  FormPtr form;
  EventType event;
  Word error;
  do   // Start the endless loop here
  {
    // get event from event queue with time-out of 100ms
    EvtGetEvent(&event, 100);

    // first let the system handle events
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent((void *)0, &event, &error)) continue;

    // now it is time to do our form stuff
    if (event.eType == frmLoadEvent)
    {
      formID = event.data.frmLoad.formID;
      form = FrmInitForm(formID);

      FrmSetActiveForm(form);
      switch (formID)
      {
        case frmMenu:
             FrmSetEventHandler(form, (FormEventHandlerPtr)frmMenuHdlr);
             break;
        case frmMain:
             FrmSetEventHandler(form, (FormEventHandlerPtr)frmMainHdlr);
             break;
        case frmAbout:
             FrmSetEventHandler(form, (FormEventHandlerPtr)frmAboutHdlr);
             break;
        case frmHall:
             FrmSetEventHandler(form, (FormEventHandlerPtr)frmHallHdlr);
             break;
        case frmPrefs:
             FrmSetEventHandler(form, (FormEventHandlerPtr)frmPrefsHdlr);
             break;
      }
    }
    FrmDispatchEvent(&event);

  } while(event.eType != appStopEvent); // close application?
}
//---------------------------------------------------------------------------
// this is the event loop for frmMenu
static int frmMenuHdlr(EventType *event)
{
  int handled = 0;

  switch(event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         InitializeMenuForm(); // initialize players
         handled = 1; // event handled
         break;

    case ctlSelectEvent:
         switch (event->data.ctlEnter.controlID)
         {
           case btnMenuPlay: // close the menu window, start playing
                {
                  int i, iCnt = 0;
                  WriteAvailPlayers(); // save players for next games

                  // now get selected players
                  for (i = 0; i < MAXPLAYERS; i++)
                  {
                    if (AvailPlayers.check[i])
                    {
                      iCnt++;
                      strncpy(TheGame.players[iCnt-1].name, AvailPlayers.name[i], PLAYERNAMELEN);
                    }
                  }
                  if (iCnt > 0)
                  {
                    TheGame.playersCount = iCnt;
                    FrmGotoForm(frmMain);
                  }
                  else
                  {
                    FrmCustomAlert(msgBox1Str, "Define at least one player!", " ", " ");
                  }
                }
                handled = 1; // event handled
                break;

           case btnMenuHall:
                WriteAvailPlayers(); // save players for next games
                HallReturnsTo = FORM_MENU;
                FrmGotoForm(frmHall);
                handled = 1; // event handled
                break;
         }
         break;

    case nilEvent:
         break;

    default:
         break;
  }

  return handled; // was Event handled?
}
//---------------------------------------------------------------------------
// this is the event loop for frmMain
static int frmMainHdlr(EventType *event)
{
  int handled = 0;

  switch(event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         InitializeMainForm(); // init all pointers to form controls
         handled = 1; // event handled
         break;

    case frmCloseEvent:
          if (NULL != frmptr_Main)
          {
            VoidHand h;

            h = FrmGetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie1));
            if (h) MemHandleFree(h);

            h = FrmGetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie2));
            if (h) MemHandleFree(h);

            h = FrmGetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie3));
            if (h) MemHandleFree(h);

            h = FrmGetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie4));
            if (h) MemHandleFree(h);

            h = FrmGetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie5));
            if (h) MemHandleFree(h);
          }
         break;

    case ctlSelectEvent:
         switch (event->data.ctlEnter.controlID)
         {
           case btnRoll:
                 RollDice();
                 ShowScoreHints(true);
                 handled = 1; // event handled
                 break;

           case btnToggleAllDice: // toggle dice rolling marking
                 if ((CrrPlayer->rollsCnt > 0) && // else nothing to toggle
                    (CrrPlayer->rollsCnt < 3))
                 {
                   int i;
                   //SndPlaySystemSound(sndClick);
                   SoundEffect(sefSilentOK, SndVolume); // delicate confirmation
                   for (i = DIE_1; i <= DIE_5; i++)
                   {
                     CrrPlayer->dicePicked[i] = !CrrPlayer->dicePicked[i];
                     DrawDieBitmap(i);
                   }
                   handled = 1; // event handled
                 }
                 break;

           case btn1s:        ScoreSelected(ITM_ONES); break;
           case btn2s:        ScoreSelected(ITM_TWOS); break;
           case btn3s:        ScoreSelected(ITM_THREES); break;
           case btn4s:        ScoreSelected(ITM_FOURS); break;
           case btn5s:        ScoreSelected(ITM_FIVES); break;
           case btn6s:        ScoreSelected(ITM_SIXES); break;
           case btnThree:     ScoreSelected(ITM_THREE); break;
           case btnFour:      ScoreSelected(ITM_FOUR); break;
           case btnFullHouse: ScoreSelected(ITM_FULLHOUSE); break;
           case btnSmStrait:  ScoreSelected(ITM_SMALLSTR); break;
           case btnLgStrait:  ScoreSelected(ITM_LARGESTR); break;
           case btnYahdice:   ScoreSelected(ITM_YAHDICE); break;
           case btnChance:    ScoreSelected(ITM_CHANCE); break;

           case btnScores:
                 ShowCurrentScores(false);
                 handled = 1;
                 break;
         }
         break;

   case menuEvent:
        switch (event->data.menu.itemID)
        {
          case optNewGame:
                StartNewGame(true);
                FrmGotoForm(frmMenu);
                handled = 1; // event handled
                break;

          case optUndo:
                UndoLastScoring();
                handled = 1; // event handled
                break;

          case optPrefs:
                FrmGotoForm(frmPrefs);
                handled = 1; // event handled
                break;

          case optHall:
                HallReturnsTo = FORM_MAIN;
                FrmGotoForm(frmHall);
                handled = 1; // event handled
                break;

          case optRules:
                FrmHelp(gtxt_Rules);
                break;

          case optScoring:
                FrmHelp(gtxt_Scoring);
                break;

          case optYahScoring:
                FrmHelp(gtxt_YahScoring);
                break;

          case optPlaying:
                FrmHelp(gtxt_Playing);
                break;

          case optAbout:
                FrmGotoForm(frmAbout);
                handled = 1; // event handled
                break;
        }
        break;

    case penDownEvent:
         {
           int dieIndex = -1;
           RectangleType bounds;

           FrmGetObjectBounds(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie1), &bounds);
           if (RctPtInRectangle(event->screenX, event->screenY, &bounds))
             dieIndex = DIE_1;
           else
           {
             FrmGetObjectBounds(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie2), &bounds);
             if (RctPtInRectangle(event->screenX, event->screenY, &bounds))
               dieIndex = DIE_2;
             else
             {
               FrmGetObjectBounds(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie3), &bounds);
               if (RctPtInRectangle(event->screenX, event->screenY, &bounds))
                 dieIndex = DIE_3;
               else
               {
                 FrmGetObjectBounds(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie4), &bounds);
                 if (RctPtInRectangle(event->screenX, event->screenY, &bounds))
                   dieIndex = DIE_4;
                 else
                 {
                   FrmGetObjectBounds(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie5), &bounds);
                   if (RctPtInRectangle(event->screenX, event->screenY, &bounds))
                     dieIndex = DIE_5;
                 }
               }
             }
           }
           if (dieIndex >= 0)
           {
             if ((CrrPlayer->rollsCnt > 0) && // else nothing to toggle
                (CrrPlayer->rollsCnt < 3))
             {
               //SndPlaySystemSound(sndClick);
               SoundEffect(sefSilentOK, SndVolume); // delicate confirmation
               CrrPlayer->dicePicked[dieIndex] = !CrrPlayer->dicePicked[dieIndex];
               DrawDieBitmap(dieIndex);
             }
             handled = 1; // event handled
           }
         }
         break;

    case nilEvent:
         break;

    default:
         break;
  }

  return handled; // was Event handled?
}
//---------------------------------------------------------------------------
// this is the event loop for frmAbout
static int frmAboutHdlr(EventType *event)
{
  int handled = 0;

  switch(event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         handled = 1; // event handled
         break;

    case ctlSelectEvent:
         switch (event->data.ctlEnter.controlID)
         {
           case btnAboutOk: // close the About... window
                 FrmGotoForm(frmMain);
                 handled = 1; // event handled
                 break;
         }
         break;

    case nilEvent:
         break;

    default:
         break;
  }

  return handled; // was Event handled?
}
//---------------------------------------------------------------------------
// this is the event loop for frmPrefs
static int frmPrefsHdlr(EventType *event)
{
  int handled = 0;

  switch(event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         InitializePrefsForm(); // initialize preferences
         handled = 1; // event handled
         break;

    case ctlSelectEvent:
         switch (event->data.ctlEnter.controlID)
         {
           case btnPrefsAccept: // close Prefs, accept changes
                 ApplyPrefsForm();
                 FrmGotoForm(frmMain);
                 handled = 1; // event handled
                 break;

           case btnPrefsCancel: // close Prefs, cancel changes
                 FrmGotoForm(frmMain);
                 handled = 1; // event handled
                 break;
         }
         break;

    case nilEvent:
         break;

    default:
         break;
  }

  return handled; // was Event handled?
}
//---------------------------------------------------------------------------
// this is the event loop for frmHall
static int frmHallHdlr(EventType *event)
{
  int handled = 0;

  switch(event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         InitializeHallForm(); // initialize Hall of Fame
         DisplayHallOfFame();
         handled = 1; // event handled
         break;

    case ctlSelectEvent:
         switch (event->data.ctlEnter.controlID)
         {
           case btnHallClearAll: // clear high results
                if (FrmCustomAlert(msgBoxYesNo, "Are you sure?", " ", " ") == 0)
                {
                  ClearHallOfFame(0);
                  DisplayHallOfFame();
                }
                handled = 1; // event handled
                break;

           case btnHallClearMost: // clear high results except for the first one
                if (FrmCustomAlert(msgBoxYesNo, "Are you sure?", " ", " ") == 0)
                {
                  char* ptr;
                  int iNum;
                  ptr = FldGetTextPtr(FrmGetObjectPtr(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), fldScoresToLeave)));
                  iNum = StrAToI(ptr);
                  if (iNum < 0) iNum = 0;
                  else if (iNum > 8) iNum = 8;
                  ClearHallOfFame(iNum);
                  DisplayHallOfFame();
                }
                handled = 1; // event handled
                break;

           case btnHallClose: // return to the caller
                {
                  switch (HallReturnsTo)
                  {
                    case FORM_MAIN:
                         FrmGotoForm(frmMain);
                         break;
                    case FORM_MENU:
                         FrmGotoForm(frmMenu);
                         break;
                    case FORM_ABOUT:
                         FrmGotoForm(frmAbout);
                         break;
                  }
                }
                handled = 1; // event handled
                break;

           case btnHallSubmit: // send high scores to the Global Yahdice High Scores List
                {
                  SubmitHighScores();
                }
                handled = 1; // event handled
                break;
         }
         break;

    case nilEvent:
         break;

    default:
         break;
  }

  return handled; // was Event handled?
}
//---------------------------------------------------------------------------
//
static int StartApplication(void)
{
  int   error;
  Word  x;
  DWord romversion;

  error = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romversion);
  isOS20 = (romversion >= 0x02003000); // os 2.0 or higher
  isOS30 = (romversion >= 0x03003000); // os 3.0 or higher
  isOS35 = (romversion >= 0x03503000); // os 3.5 or higher

  if (!isOS20)
  {
    FrmCustomAlert(msgBox3Str, "This application needs", "Palm OS >= 2.0!", "Can't continue...");
    return 1;
  }

  if (isOS35)
  {
    WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL, &isColor);
  }
  else
  {
    isColor = false;
  }

  // sounds
  SndVolume = PrefGetPreference(prefSysSoundLevelV20); // get system sounds amplitude
  SndVolume = (SndVolume == slOn) ? sndMaxAmp : 0;

  // defaults
  Preferences.rollAnimate = true;  // animate dice while rolling?
  Preferences.autoSort    = false; // auto-sorting after each roll?
  Preferences.isPause     = true;  // pause between players change
  Preferences.pauseLen    = 1;     // pause length (in secs)
  Preferences.showHints   = true;  // show score hints?
  Preferences.allHints    = false; // hints also for scored items?
  Preferences.categHints  = true;  // show category hints?
  Preferences.advUser     = false; // advanced user?
  Preferences.markKeep    = false; // mark dice to be kept?
  Preferences.bdmy5       = false;
  Preferences.idmy1 = -1;
  Preferences.idmy2 = -1;
  Preferences.idmy3 = -1;
  Preferences.idmy4 = -1;
  Preferences.idmy5 = -1;

  // read preferences from the database
  ReadPreferences();

  StartNewGame(false);
  ReadGameState();
  if (TheGame.isFinished)
  {
    // start the initial form
    FrmGotoForm(frmMenu);
  }
  else
  {
    // continue the game
    FrmGotoForm(frmMain);
  }

  return 0;
}
//---------------------------------------------------------------------------
//
static void StopApplication(void)
{
  WriteGameState();
}
//---------------------------------------------------------------------------
// Initialize players
static void InitializeMenuForm(void)
{
  FormPtr form = FrmGetActiveForm();
  int i;

  fldptr_menunames[0] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldMenuPlayer1));
  fldptr_menunames[1] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldMenuPlayer2));
  fldptr_menunames[2] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldMenuPlayer3));
  fldptr_menunames[3] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldMenuPlayer4));

  ctlptr_menucheck[0] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkMenuPlayer1));
  ctlptr_menucheck[1] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkMenuPlayer2));
  ctlptr_menucheck[2] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkMenuPlayer3));
  ctlptr_menucheck[3] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkMenuPlayer4));

  // read last players from the database
  ReadAvailPlayers();

  for (i = 0; i < MAXPLAYERS; i++)
  {
    SetFieldText(fldptr_menunames[i], AvailPlayers.name[i], PLAYERNAMELEN + 1);
    FldDrawField(fldptr_menunames[i]);
    CtlSetValue(ctlptr_menucheck[i], AvailPlayers.check[i] ? 1 : 0);
  }

  TheGame.isFinished = true; // no game in progress
}
//---------------------------------------------------------------------------
// Initialize Hall of Fame
static void InitializeHallForm(void)
{
  FormPtr form = FrmGetActiveForm();

  fldptr_fldScoresToLeave = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldScoresToLeave));
  SetFieldText(fldptr_fldScoresToLeave, "3", 10);
  FldDrawField(fldptr_fldScoresToLeave);

  fldptr_HallPlayer[0] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer1));
  fldptr_HallPlayer[1] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer2));
  fldptr_HallPlayer[2] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer3));
  fldptr_HallPlayer[3] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer4));
  fldptr_HallPlayer[4] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer5));
  fldptr_HallPlayer[5] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer6));
  fldptr_HallPlayer[6] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer7));
  fldptr_HallPlayer[7] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallPlayer8));

  fldptr_HallScore[0] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore1));
  fldptr_HallScore[1] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore2));
  fldptr_HallScore[2] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore3));
  fldptr_HallScore[3] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore4));
  fldptr_HallScore[4] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore5));
  fldptr_HallScore[5] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore6));
  fldptr_HallScore[6] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore7));
  fldptr_HallScore[7] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallScore8));

  fldptr_HallDate[0] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate1));
  fldptr_HallDate[1] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate2));
  fldptr_HallDate[2] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate3));
  fldptr_HallDate[3] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate4));
  fldptr_HallDate[4] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate5));
  fldptr_HallDate[5] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate6));
  fldptr_HallDate[6] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate7));
  fldptr_HallDate[7] = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldHallDate8));

  return;
}
//---------------------------------------------------------------------------
// Read best scores from the database and display it
static void DisplayHallOfFame(void)
{
  int i;
  char sBff[10];
  char sDate[dowLongDateStrLength];
  SystemPreferencesType sysPrefs;

  PrefGetPreferences(&sysPrefs);

  ReadHallOfFame();
  for (i = 0; i < MAXHALLITEMS; i++)
  {
    // name
    SetFieldText(fldptr_HallPlayer[i], HallOfFame[i].name, PLAYERNAMELEN + 1);
    FldDrawField(fldptr_HallPlayer[i]);
    // score
    StrPrintF(sBff, "%d", HallOfFame[i].score);
    SetFieldText(fldptr_HallScore[i], sBff, 10);
    FldDrawField(fldptr_HallScore[i]);
    // date
    DateToAscii(HallOfFame[i].date.month, HallOfFame[i].date.day, HallOfFame[i].date.year, sysPrefs.longDateFormat, sDate);
    SetFieldText(fldptr_HallDate[i], sDate, dowLongDateStrLength);
    FldDrawField(fldptr_HallDate[i]);
  }
}
//---------------------------------------------------------------------------
// Add a new score to the Hall of Fame
static Boolean AddToHallOfFame(int idx)
{
  int i, j;
  Boolean retVal = false;

  ReadHallOfFame();
  for (i = 0; i < MAXHALLITEMS; i++)
  {
    if (TheGame.players[idx].totScore > HallOfFame[i].score)
    {
      // shift worse scores down
      for (j = MAXHALLITEMS - 2; j >= i; j--)
      {
        StrNCopy(HallOfFame[j+1].name, HallOfFame[j].name, PLAYERNAMELEN);
        HallOfFame[j+1].score = HallOfFame[j].score;
        HallOfFame[j+1].date  = HallOfFame[j].date;
      }
      // register the new score
      StrNCopy(HallOfFame[i].name, TheGame.players[idx].name, PLAYERNAMELEN);
      HallOfFame[i].score = TheGame.players[idx].totScore;
      TimSecondsToDateTime(TimGetSeconds(), &(HallOfFame[i].date));
      WriteHallOfFame();
      retVal = true;
      i = MAXHALLITEMS;
    }
  }
  return retVal;
}
//---------------------------------------------------------------------------
// Initialize the memory structure of the Hall of Fame
void InitMemHallOfFame(int leaveCnt)
{
  int i;

  for (i = leaveCnt; i < MAXHALLITEMS; i++)
  {
    StrNCopy(HallOfFame[i].name, "Yahdice", PLAYERNAMELEN);
    HallOfFame[i].score = 150 - i*10;
    TimSecondsToDateTime(TimGetSeconds(), &(HallOfFame[i].date));
  }
}
//---------------------------------------------------------------------------
// Reset the Hall of Fame
static void ClearHallOfFame(int leaveCnt)
{
  InitMemHallOfFame(leaveCnt);
  WriteHallOfFame();
}
//---------------------------------------------------------------------------
static void InitializePrefsForm(void)
{
  FormPtr    form = FrmGetActiveForm();
  ControlPtr ctlptr;
  FieldPtr   fldPtr;
  char       sBff[10];

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAnimation));
  CtlSetValue(ctlptr, Preferences.rollAnimate ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsPause));
  CtlSetValue(ctlptr, Preferences.isPause ? 1 : 0);

  fldPtr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldPrefsPause));
  StrPrintF(sBff, "%d", Preferences.pauseLen);
  SetFieldText(fldPtr, sBff, 10);
  FldDrawField(fldPtr);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsShowHints));
  CtlSetValue(ctlptr, Preferences.showHints ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAllHints));
  CtlSetValue(ctlptr, Preferences.allHints ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsCategHints));
  CtlSetValue(ctlptr, Preferences.categHints ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAdvUser));
  CtlSetValue(ctlptr, Preferences.advUser ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsMarkKeep));
  CtlSetValue(ctlptr, Preferences.markKeep ? 1 : 0);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAutoSort));
  CtlSetValue(ctlptr, Preferences.autoSort ? 1 : 0);
}
//---------------------------------------------------------------------------
static void ApplyPrefsForm(void)
{
  FormPtr    form = FrmGetActiveForm();
  ControlPtr ctlptr;
  FieldPtr   fldPtr;
  char*      ptr;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAnimation));
  Preferences.rollAnimate = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsPause));
  Preferences.isPause = CtlGetValue(ctlptr) == 1;

  fldPtr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, fldPrefsPause));
  ptr = FldGetTextPtr(fldPtr);
  Preferences.pauseLen = StrAToI(ptr);

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsShowHints));
  Preferences.showHints = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAllHints));
  Preferences.allHints = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsCategHints));
  Preferences.categHints = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAdvUser));
  Preferences.advUser = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsMarkKeep));
  Preferences.markKeep = CtlGetValue(ctlptr) == 1;

  ctlptr = FrmGetObjectPtr(form, FrmGetObjectIndex(form, chkPrefsAutoSort));
  Preferences.autoSort = CtlGetValue(ctlptr) == 1;

  // write preferences to the database
  WritePreferences();
}
//---------------------------------------------------------------------------
// Prepare all UI items pointers
static void InitializeMainForm(void)
{
  frmptr_Main = FrmGetActiveForm();

  // init pointers to UI elements
  ctlptr_Roll01 = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnRoll1));
  ctlptr_Roll02 = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnRoll2));
  ctlptr_Roll03 = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnRoll3));

  ctlptr_btnScores = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnScores));

  fldptr_player  = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldPlayer));
  fldptr_prompt  = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldPrompt));
  fldptr_total   = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldValTotal));
  fldptr_bonus   = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldValBonus));
  fldptr_totScore = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldValScore));

  ctlptr_button[ITM_ONES     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn1s));
  ctlptr_button[ITM_TWOS     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn2s));
  ctlptr_button[ITM_THREES   ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn3s));
  ctlptr_button[ITM_FOURS    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn4s));
  ctlptr_button[ITM_FIVES    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn5s));
  ctlptr_button[ITM_SIXES    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btn6s));
  ctlptr_button[ITM_THREE    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnThree));
  ctlptr_button[ITM_FOUR     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnFour));
  ctlptr_button[ITM_FULLHOUSE] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnFullHouse));
  ctlptr_button[ITM_SMALLSTR ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnSmStrait));
  ctlptr_button[ITM_LARGESTR ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnLgStrait));
  ctlptr_button[ITM_YAHDICE  ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnYahdice));
  ctlptr_button[ITM_CHANCE   ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, btnChance));

  fldptr_score[ITM_ONES     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore1s));
  fldptr_score[ITM_TWOS     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore2s));
  fldptr_score[ITM_THREES   ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore3s));
  fldptr_score[ITM_FOURS    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore4s));
  fldptr_score[ITM_FIVES    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore5s));
  fldptr_score[ITM_SIXES    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScore6s));
  fldptr_score[ITM_THREE    ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreThree));
  fldptr_score[ITM_FOUR     ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreFour));
  fldptr_score[ITM_FULLHOUSE] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreFullHouse));
  fldptr_score[ITM_SMALLSTR ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreSmStrait));
  fldptr_score[ITM_LARGESTR ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreLgStrait));
  fldptr_score[ITM_YAHDICE  ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreYahdice));
  fldptr_score[ITM_CHANCE   ] = FrmGetObjectPtr(frmptr_Main, FrmGetObjectIndex(frmptr_Main, fldScoreChance));

  // create gadgets
  {
    VoidHand h;

    h = MemHandleNew(sizeof(Word));
    if (h)
    {
      *(Word*)MemHandleLock(h)=1;  MemHandleUnlock(h);
      FrmSetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie1), h);
    }

    h = MemHandleNew(sizeof(Word));
    if (h)
    {
      *(Word*)MemHandleLock(h)=1;  MemHandleUnlock(h);
      FrmSetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie2), h);
    }

    h = MemHandleNew(sizeof(Word));
    if (h)
    {
      *(Word*)MemHandleLock(h)=1;  MemHandleUnlock(h);
      FrmSetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie3), h);
    }

    h = MemHandleNew(sizeof(Word));
    if (h)
    {
      *(Word*)MemHandleLock(h)=1;  MemHandleUnlock(h);
      FrmSetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie4), h);
    }

    h = MemHandleNew(sizeof(Word));
    if (h)
    {
      *(Word*)MemHandleLock(h)=1;  MemHandleUnlock(h);
      FrmSetGadgetData(frmptr_Main, FrmGetObjectIndex(frmptr_Main, gadDie5), h);
    }
  }

  if (TheGame.isFinished)
  {
    StartNewGame(true);
  }
  else // only redisplay the board
  {
    DrawAll();
  }
}
//---------------------------------------------------------------------------
// Show scores of all players in a message dialog
Boolean ShowCurrentScores(Boolean ifGameOver)
{
  typedef struct
  {
    char name[PLAYERNAMELEN + 1]; // player's name
    int score;                // his score
  } T_OneScore;

  int i, j;
  char sMsg[MAXPLAYERS * (PLAYERNAMELEN + 15)] = "";
  char sBff[PLAYERNAMELEN + 15] = "";
  T_OneScore tmpScore, scores[MAXPLAYERS];
  int minHallScore;
  Boolean isInHall = false;


  for (i = 0; i < TheGame.playersCount; i++)
  {
    strncpy(scores[i].name, TheGame.players[i].name, PLAYERNAMELEN);
    scores[i].score = TheGame.players[i].totScore;
  }

  // bubble-sort players
  for (i = 0; i < TheGame.playersCount - 1; i++)
    for (j = i + 1; j < TheGame.playersCount; j++)
    {
      if (scores[j].score > scores[i].score)
      {
        tmpScore = scores[j];
        scores[j] = scores[i];
        scores[i] = tmpScore;
      }
    }

  ReadHallOfFame();
  minHallScore = HallOfFame[MAXHALLITEMS - 1].score;

  for (i = 0; i < TheGame.playersCount; i++)
  {
    StrPrintF(sBff, "%d) %s %s: %d\n", i+1, (scores[i].score > minHallScore) ? "\216" : " ", scores[i].name, scores[i].score);
    StrCat(sMsg, sBff);
  }
  if (scores[0].score > minHallScore) // at least one new score in the hall
  {
    StrCat(sMsg, "\n\216 - Hall of Fame!");
    isInHall = true;
  }

  FrmCustomAlert(msgBox3Str,
                 ifGameOver ? "Game over!" : "Current scores",
                 " ",
                 sMsg);

  return isInHall;
}
//---------------------------------------------------------------------------
// Convert the given string to the hexadecimal representation
char sHexBff[32];
char* CvtStr2HexStr(char* sStr)
{
  int  i;
  char sBff[9];

  *sHexBff = 0;
  for (i = 0; i < StrLen(sStr); i++)
  {
//    StrPrintF(sBff, "%2X", (unsigned char)sStr[i]); // it always makes a 4-digit long string?!?
    StrPrintF(sBff, "%x", (unsigned char)sStr[i]); // on Visors it produces an 8-digit long string
    if (StrLen(sBff) > 2)
      StrCat(sHexBff, sBff + StrLen(sBff) - 2); // take last 2 digits
    else
      StrCat(sHexBff, sBff);
  }
  return sHexBff;
}
//---------------------------------------------------------------------------
// Return a crypted version of the given score.
// Shame on you if you use this code to submit fake scores!
char sCodeBff[32];
char* CodeScore(int iScore, int iXor)
{
  int  iRnd, iNewVal;
  char sScore[51];
  int  j;

  *sCodeBff = 0;
  iRnd = SysRandomUnder(9)+1;
  iNewVal = iRnd*2345 - iScore + iRnd;

  StrPrintF(sScore, "%d%d", iRnd, iNewVal);
  for (j = 0; j < StrLen(sScore); j++)
  {
    sScore[j] = (char)(sScore[j] ^ iXor);
  }
  StrCopy(sScore, CvtStr2HexStr(sScore));
  StrPrintF(sCodeBff, "YD%d%s%d", SysRandomUnder(10), sScore, SysRandomUnder(10));
  return sCodeBff;
}
//---------------------------------------------------------------------------
// Generate codes for submitting high scores
void SubmitHighScores(void)
{
  int  i, j;
  char sBff[30] = ""; // one score key
  char sMsg[30 * 5] = ""; // all scores concatenated

  ReadHallOfFame();
  for (i = 0; i < 5; i++)
  {
    StrPrintF(sBff, "%d-%s\n", HallOfFame[i].score, CodeScore(HallOfFame[i].score, 65));
    StrCat(sMsg, sBff);
  }

  if (FrmCustomAlert(msgBoxHighScores, sMsg, "E-mail above high scores keys, your name and country to scores@mirekw.com", " ") == 0)
  {
    Char szText[300] = "";
    Char sDate[20];
    DateTimeType dtNow; 
    SystemPreferencesType sysPrefs;
    Err errCode;

    PrefGetPreferences(&sysPrefs);
    TimSecondsToDateTime(TimGetSeconds(), &dtNow);
    DateToAscii(dtNow.month, dtNow.day, dtNow.year, sysPrefs.longDateFormat, sDate);
    StrCopy(szText, "Yahdice 2 scores ");
    StrCat(szText, sDate);
    StrCat(szText, "\n\n");
    StrCat(szText, sMsg);
    StrCat(szText, "\n");
    StrCat(szText, "E-mail above high scores keys, your name and country to scores@mirekw.com");

    errCode = WriteMemoRecord(szText);
    if (0 == errCode) // success
    {
      FrmCustomAlert(msgBox1Str, "Scores successfully exported to Memo Pad application", " ", " ");
    }
    else // error
    {
      Char sBff[20];
      StrPrintF(sBff, "Error %d", errCode);
      FrmCustomAlert(msgBox3Str, sBff, "while exporting scores to Memo Pad", " ");
    }
  }

  return;
}
//---------------------------------------------------------------------------
// Show the user what to do now
void ShowRollPrompt(void)
{
  switch (CrrPlayer->rollsCnt)
  {
    case 0:
            SetFieldText(fldptr_prompt, "Roll the dice", 50);
            break;
    case 1:
    case 2:
            SetFieldText(fldptr_prompt, "Roll again or pick score", 50);
            break;
    case 3:
            SetFieldText(fldptr_prompt, "Pick your score", 50);
            break;
  }

  FldDrawField(fldptr_prompt);
}
//---------------------------------------------------------------------------
// Draw all UI elements
void DrawAll(void)
{
  int i;

  SetFieldText(fldptr_player, CrrPlayer->name, PLAYERNAMELEN);
  FldDrawField(fldptr_player);

  ShowRollPrompt(); // show the user what to do now

  // button Scores visible only for more than 1 player
  //CtlSetUsable(ctlptr_btnScores, (TheGame.playersCount > 1) ? 1 : 0);

  for (i = ITM_ONES; i <= ITM_CHANCE; i++)
  {
    SetAndShowScore(i, CrrPlayer->items[i].score); // show all scores
    CtlSetValue(ctlptr_button[i], CrrPlayer->items[i].used ? ITEM_USED : ITEM_FREE); // mark used items
    if (CrrPlayer->rollsCnt > 0) // any game in progress
    {
      CtlSetEnabled(ctlptr_button[i], !CrrPlayer->items[i].used); // enable all not used items
    }
    else // no rolls yet, all items disabled
    {
      CtlSetEnabled(ctlptr_button[i], false);
    }
  }

  ShowScoreHints(TheGame.areHintsShown); // show or hide all hints

  // paint all dice
  for (i = DIE_1; i <= DIE_5; i++)
  {
    DrawDieBitmap(i);
  }

  ShowTotals();

  MarkRollNum(); // update the roll number indicator
}
//---------------------------------------------------------------------------
// Prepare for a new game - reset to all defaults
void StartNewGame(Boolean fShow)
{
  int i, iGame;

  CanUndo = false;

  TheGame.areHintsShown = false;
  TheGame.isFinished = false;

  for (iGame = 0; iGame < MAXPLAYERS; iGame++)
  {
    TheGame.players[iGame].rollsCnt = 0; // no rolls yet
    TheGame.players[iGame].roundCnt = 0; // no rounds played yet
    TheGame.players[iGame].subtotal = 0; // total of 6 left items

    for (i = ITM_ONES; i <= ITM_CHANCE; i++)
    {
      TheGame.players[iGame].items[i].score = 0;
      TheGame.players[iGame].items[i].hint = 0;
      TheGame.players[iGame].items[i].used = false;
    }

    for (i = DIE_1; i <= DIE_5; i++)
    {
      TheGame.players[iGame].diceState[i]  = 0;    // state of all 5 dice: 0, 1..6; 0 - unknown yet
      TheGame.players[iGame].dicePicked[i] = true; // activity of all 5 dice
    }
  }

  TheGame.crrPlayerIdx = 0;
  CrrPlayer = &(TheGame.players[TheGame.crrPlayerIdx]);

  if (fShow)
  {
    FrmDrawForm(FrmGetActiveForm());
    DrawAll();
  }
}
//---------------------------------------------------------------------------
// Function allows to undo last scoring
static void UndoLastScoring(void)
{
  if (CanUndo)
  {
    TheGame = GameSnap;
    CrrPlayer = &(TheGame.players[TheGame.crrPlayerIdx]);
    CanUndo = false;
  }
  else
  {
    FrmCustomAlert(msgBox3Str, "Nothing to undo at the moment!", " ", "One can undo last scoring providing the dice were not rolled yet again.");
  }
  DrawAll();
}
//---------------------------------------------------------------------------
void SetAndShowScore(int index, int iVal)
{
  char wrkBuf[10] = "";

  CrrPlayer->items[index].score = iVal;

  if (CrrPlayer->items[index].used) // only for used items
  {
    StrPrintF(wrkBuf, "%d", iVal);
  }
  SetFieldText(fldptr_score[index], wrkBuf, 10);
  FldDrawField(fldptr_score[index]);
}
//---------------------------------------------------------------------------
// Animate (blink) one of items
void BlinkItem(int idx, int iCnt)
{
  int i;

  if ((idx == ITM_YAHDICE) || (Preferences.categHints && (!CrrPlayer->items[idx].used)))
  {
    for (i = 1; i <= iCnt; i++)
    {
      if (idx == ITM_YAHDICE)
        if ((i % 4) == 1)
          SndPlaySystemSound(sndStartUp);
      CtlSetValue(ctlptr_button[idx], 1 - CtlGetValue(ctlptr_button[idx]));
      SysTaskDelay(SysTicksPerSecond() / 5); // wait 1/5 a second
    }
  }
}
//---------------------------------------------------------------------------
// Calculate the actual values of all 5 dice for all games,
// put results to hints
void CalcScoreHints(void)
{
  int i, j, iVal, iSum = 0;
  int valAry[7] = { 0,0,0,0,0,0,0 }; // indices 1..6 are in use
  Boolean isTwo = false, isThree = false, isFour = false, isFive = false;
  Boolean isFHouse = false, isLgStr = false, isSmStr = false;
  Boolean isJoker = false;

  // count occurrences
  for (i = DIE_1; i <= DIE_5; i++)
  {
    iVal = CrrPlayer->diceState[i];
    if ((iVal > 0) && (iVal < 7))
      valAry[iVal]++;
  }

  for (i = 1; i <= 6; i++)
  {
    iVal = valAry[i];
    if (iVal == 5)
      isFive = true;
    else if (iVal == 4)
      isFour = true;
    else if (iVal == 3)
      isThree = true;
    else if (iVal == 2)
      isTwo = true;
  }

  for (i = DIE_1; i <= DIE_5; i++)
  {
    iSum += CrrPlayer->diceState[i];
  }

  // first reset all hints
  for (i = ITM_ONES; i <= ITM_CHANCE; i++)
  {
    CrrPlayer->items[i].hint = 0;
  }

  // ITM_YAHDICE
  if (isFive)
  {
    //SoundEffect(sefHighBeep, SndVolume);
    BlinkItem(ITM_YAHDICE, 12); // animate Yahdice
    if (CrrPlayer->items[ITM_YAHDICE].score > 0) // next yahdice!!!
    {
      SetAndShowScore(ITM_YAHDICE, CrrPlayer->items[ITM_YAHDICE].score + 100); // automatically +100
      ShowTotals();
      FrmCustomAlert(msgBox3Str,
                     "Next Yahdice",
                     "Extra 100 points!",
                     "Please pick your score.");
      isJoker = true;
    }
    else // the first yahdice
    {
      if (!CrrPlayer->items[ITM_YAHDICE].used) // not picked yet
      {
        CrrPlayer->items[ITM_YAHDICE].hint = 50;
      }
      else
      {
        isJoker = true; // no score for yahdice, but use Joker rules
      }
    }
  }

  // ITM_ONES..ITM_SIXES
  for (i = ITM_ONES; i <= ITM_SIXES; i++)
  {
    iVal = 0;
    for (j = DIE_1; j <= DIE_5; j++)
    {
      iVal += ((CrrPlayer->diceState[j] == i + 1) ? i + 1 : 0);
    }
    CrrPlayer->items[i].hint = iVal;
  }

  // ITM_THREE
  if (isThree || isFour || isFive)
    CrrPlayer->items[ITM_THREE].hint = iSum;

  // ITM_FOUR
  if (isFour || isFive)
    CrrPlayer->items[ITM_FOUR].hint = iSum;

  // ITM_FULLHOUSE
  isFHouse = (isThree && isTwo);
  if (isFHouse || isJoker)
    CrrPlayer->items[ITM_FULLHOUSE].hint = 25;
  if (isFHouse)
    BlinkItem(ITM_FULLHOUSE, 2);

  // ITM_SMALLSTR
  isSmStr =   ((valAry[1] > 0) && (valAry[2] > 0) && (valAry[3] > 0) && (valAry[4] > 0))
           || ((valAry[2] > 0) && (valAry[3] > 0) && (valAry[4] > 0) && (valAry[5] > 0))
           || ((valAry[3] > 0) && (valAry[4] > 0) && (valAry[5] > 0) && (valAry[6] > 0));
  if (isSmStr || isJoker)
    CrrPlayer->items[ITM_SMALLSTR].hint = 30;

  // ITM_LARGESTR
  isLgStr =   ((valAry[1] > 0) && (valAry[2] > 0) && (valAry[3] > 0) && (valAry[4] > 0) && (valAry[5] > 0))
           || ((valAry[2] > 0) && (valAry[3] > 0) && (valAry[4] > 0) && (valAry[5] > 0) && (valAry[6] > 0));
  if (isLgStr || isJoker)
    CrrPlayer->items[ITM_LARGESTR].hint = 40;

  if (isLgStr)
    BlinkItem(ITM_LARGESTR, 2);
  else if (isSmStr)
    BlinkItem(ITM_SMALLSTR, 2);

  // ITM_CHANCE
  CrrPlayer->items[ITM_CHANCE].hint = iSum;
}
//---------------------------------------------------------------------------
// Display score hints for all free items
void ShowScoreHints(Boolean fShow)
{
  RectangleType bounds, rect;
  int i;

  rect.extent.x = 8; // 2 digits max.
  rect.extent.y = 7;

  for (i = ITM_ONES; i <= ITM_CHANCE; i++)
  {
    FldGetBounds(fldptr_score[i], &bounds);
    rect.topLeft.x = bounds.topLeft.x + bounds.extent.x + 2; // 2 pixels of margin
    rect.topLeft.y = bounds.topLeft.y + bounds.extent.y - 6;

    WinEraseRectangle(&rect, 0);

    if ( (Preferences.showHints) && fShow &&  // should hints be displayed?
         ((Preferences.allHints) || (!CrrPlayer->items[i].used)) // and this one?
       )
    {
      if (isColor)
      {
        WinPushDrawState();
        WinSetForeColor(226);
        DrawTinyValue(rect.topLeft.x + 4, rect.topLeft.y, CrrPlayer->items[i].hint);
        WinPopDrawState();
      }
      else
      {
        DrawTinyValue(rect.topLeft.x + 4, rect.topLeft.y, CrrPlayer->items[i].hint);
      }
    }
  }

  TheGame.areHintsShown = fShow;
}
//---------------------------------------------------------------------------
// Roll all selected dice
void RollDice(void)
{
  int i, j;
  Boolean doRoll = false;

  if (CrrPlayer->rollsCnt < 3)
  {
    for (i = DIE_1; i <= DIE_5; i++)
    {
      if (Preferences.markKeep) // roll all not picked
      {
        if ((!CrrPlayer->dicePicked[i]) || (CrrPlayer->diceState[i] == 0)) // only when picked for rolling
          doRoll = true;
      }
      else // roll all picked
      {
        if ((CrrPlayer->dicePicked[i]) || (CrrPlayer->diceState[i] == 0)) // only when picked for rolling
          doRoll = true;
      }
    }

    if (doRoll)
    {
      CanUndo = false;

      // enable all not used items
      for (i = ITM_ONES; i <= ITM_CHANCE; i++)
      {
        CtlSetEnabled(ctlptr_button[i], !CrrPlayer->items[i].used);
      }

      CrrPlayer->rollsCnt++;
      MarkRollNum(); // update the roll number indicator
      ShowRollPrompt(); // show the user what to do now
      //SoundEffect(sefRoll, SndVolume);
      //SoundEffect(sefSilentOK, SndVolume);

      if (Preferences.rollAnimate)
      {
        for (j = 1; j <= 8; j++)
        {
          for (i = DIE_1; i <= DIE_5; i++)
          {
//            Boolean doAnimate = false;
            if ((CrrPlayer->diceState[i] == 0) || (CrrPlayer->diceState[i] > 6) || (Preferences.markKeep != CrrPlayer->dicePicked[i]))
/*            if (Preferences.markKeep) // roll all not picked
            {
              if ((!CrrPlayer->dicePicked[i]) || (CrrPlayer->diceState[i] == 0)) // only when picked for rolling
                doAnimate = true;
            }
            else // roll all picked
            {
              if ((CrrPlayer->dicePicked[i]) || (CrrPlayer->diceState[i] == 0)) // only when picked for rolling
                doAnimate = true;
            }
            if (doAnimate)*/
            {
              CrrPlayer->diceState[i] = 11 + j % 4; // animation
              DrawDieBitmap(i);
            }
          }
          SysTaskDelay(SysTicksPerSecond() / 10); // wait 1/10 a second
        }
      }
      else // no animation, make a short pause
      {
        SysTaskDelay(SysTicksPerSecond() / 10); // wait 1/10 a second
      }

      // roll dice
      for (i = DIE_1; i <= DIE_5; i++)
      {
        if (   (Preferences.markKeep ? !CrrPlayer->dicePicked[i] : CrrPlayer->dicePicked[i]) // selected for rolling
            || (CrrPlayer->diceState[i] == 0) // undefined
            || (CrrPlayer->diceState[i] > 10) // being animated
           )
        { 
          int iTmp;
          for (j = 0; j <= SysRandomUnder(6); j++) 
            iTmp = SysRandomUnder(100); // Q&D method of improving the randomness

          CrrPlayer->diceState[i] = SysRandomUnder(6)+1;  // state of all 5 dice: 0, 1..6 0 - unknown yet
          CrrPlayer->dicePicked[i] = false;
        }
      }
      // sort dice optionally
      if (Preferences.autoSort)
      {
        int tmpStt;
        Boolean tmpPik;
        for (i = DIE_1; i <= DIE_4; i++)
          for (j = i+1; j <= DIE_5; j++)
            if (CrrPlayer->diceState[i] > CrrPlayer->diceState[j])
            {
              tmpStt = CrrPlayer->diceState[i];
              tmpPik = CrrPlayer->dicePicked[i];
              CrrPlayer->diceState[i]  = CrrPlayer->diceState[j];
              CrrPlayer->dicePicked[i] = CrrPlayer->dicePicked[j];
              CrrPlayer->diceState[j]  = tmpStt;
              CrrPlayer->dicePicked[j] = tmpPik;
            }
      }
      // finally paint all dice
      for (i = DIE_1; i <= DIE_5; i++)
        DrawDieBitmap(i);

      CalcScoreHints(); // evaluate the hand
    }
    else
    {
      if (Preferences.markKeep)
      {
        FrmCustomAlert(msgBox1Str, "Unmark at least one die for rolling!", " ", " ");
      }
      else
      {
        FrmCustomAlert(msgBox1Str, "Mark at least one die for rolling!", " ", " ");
      }
    }
  }
  else
  {
    FrmCustomAlert(msgBox3Str,
                   "You can't roll more than 3 times in one round.",
                   " ",
                   "Please pick your score now!");
    DrawAll();
  }
}
//---------------------------------------------------------------------------
// Calculate and show current score
void ShowTotals(void)
{
  int i, iTotal = 0, iBonus = 0, iScore = 0, iOver = 0;
  char sBff[10] = "";

  for (i = ITM_ONES; i <= ITM_SIXES; i++)
  {
    iTotal += CrrPlayer->items[i].score;
    if (CrrPlayer->items[i].used) // item played
      iOver += CrrPlayer->items[i].score - 3*(i+1);
  }

  if (iTotal > 62)
  {
    if ((!Preferences.advUser) && (CrrPlayer->subtotal <= 62))
    {
      FrmCustomAlert(msgBox3Str,
                     "Left section > 62",
                     " ",
                     "35 BONUS POINTS!!!");
    }
    iBonus = 35;
  }

  CrrPlayer->subtotal = iTotal; // total of 6 left items

  for (i = ITM_ONES; i <= ITM_CHANCE; i++)
  {
    iScore += CrrPlayer->items[i].score;
  }
  iScore += iBonus;

  StrPrintF(sBff, "%d /%s%d", iTotal, (iOver > 0) ? "+" : "", iOver);
  SetFieldText(fldptr_total, sBff, 10);
  FldDrawField(fldptr_total);

  StrPrintF(sBff, "%d", iBonus);
  SetFieldText(fldptr_bonus, sBff, 10);
  FldDrawField(fldptr_bonus);

  StrPrintF(sBff, "%d", iScore);
  SetFieldText(fldptr_totScore, sBff, 10);
  FldDrawField(fldptr_totScore);

  CrrPlayer->totScore = iScore;
}
//---------------------------------------------------------------------------
// One of scores picked, prepare to the next round
void ScoreSelected(int item)
{
  int i;
  char sBff[10] = "";
  Boolean isEnd = false;

  //SoundEffect(sefSelect, SndVolume); // delicate confirmation

  GameSnap = TheGame; // make the game snap
  CanUndo = true;

  CrrPlayer->items[item].used = true;
  ShowScoreHints(false); // hide all hints

  CtlSetValue(ctlptr_button[item], ITEM_USED);
  SetAndShowScore(item, CrrPlayer->items[item].hint);

  for (i = ITM_ONES; i <= ITM_CHANCE; i++)
  {
    CtlSetEnabled(ctlptr_button[i], false);
  }

  ShowTotals();

  if (   (Preferences.isPause) // pause between players change
      && (TheGame.playersCount > 1) // more than 1 player
     )
  {
    if (Preferences.pauseLen > 0) SysTaskDelay(SysTicksPerSecond() * Preferences.pauseLen);
  }
  else // a small pause is always nice
  {
    SysTaskDelay(SysTicksPerSecond() / 5);
  }

  if (TheGame.playersCount > 1) // more than 1 player
  {
    SndPlaySystemSound(sndInfo);
  }

  CrrPlayer->roundCnt++; // one more round played

  if (CrrPlayer->roundCnt >= ITEMS_COUNT) // the last round
    if (TheGame.crrPlayerIdx == TheGame.playersCount - 1) // the last player
      isEnd = true;

  if (isEnd) // the last game
  {
    Boolean showHall = false;
    int i;

    TheGame.isFinished = true;

    // check if can get to the Hall of Fame
    for (i = 0; i < TheGame.playersCount; i++)
    {
      if (AddToHallOfFame(i))
        showHall = true;
    }

    ShowCurrentScores(true); // show final scores
    StartNewGame(false); // clear all results

    // we got there! Show the hall.
    if (showHall)
    {
      HallReturnsTo = FORM_MENU;
      FrmGotoForm(frmHall); // show Hall of Fame
    }
    else // no news in the hall, start a new game
    {
      FrmGotoForm(frmMenu);
    }
  }
  else // the next round
  {
    // prepare the next round
    CrrPlayer->rollsCnt = 0;

    for (i = DIE_1; i <= DIE_5; i++)
    {
      CrrPlayer->diceState[i]  = 0;    // state of all 5 dice: 0, 1..6 0 - unknown yet
      CrrPlayer->dicePicked[i] = true; // activity of all 5 dice
      DrawDieBitmap(i);
    }

    if (TheGame.playersCount > 1) // more players, activate the next one
    {
      TheGame.crrPlayerIdx = (TheGame.crrPlayerIdx + 1) % TheGame.playersCount;
      CrrPlayer = &(TheGame.players[TheGame.crrPlayerIdx]);
      DrawAll();
    }
    else // one player only
    {
      MarkRollNum(); // update the roll number indicator
    }

    ShowRollPrompt(); // show the user what to do now
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
