

#define APPNAME "Pong"

// Windows Header Files:
#include <windows.h>
#include <string.h>

// Local Header Files
#include "pong.h"
#include "resource.h"

// Makes it easier to determine appropriate code paths:
#if defined (WIN32)
	#define IS_WIN32 TRUE
#else
	#define IS_WIN32 FALSE
#endif
#define IS_NT      IS_WIN32 && (BOOL)(GetVersion() < 0x80000000)
#define IS_WIN32S  IS_WIN32 && (BOOL)(!(IS_NT) && (LOBYTE(LOWORD(GetVersion()))<4))
#define IS_WIN95 (BOOL)(!(IS_NT) && !(IS_WIN32S)) && IS_WIN32

// Global Variables:
HINSTANCE g_hinst;
char szAppName[] = APPNAME; // The name of this application
char INIFile[] = APPNAME".ini";
char szTitle[]   = APPNAME; // The title bar text

#define APP_XSIZE (440+10)
#define APP_YSIZE (300+60)
int AppXSize=APP_XSIZE,AppYSize=APP_YSIZE,AppXSizeR,AppYSizeR;
BYTE doubleSize=1,bSuoni=1;
HWND ghWnd,hStatusWnd;
HBRUSH hBrush,hBrush2;
HPEN hPen1,hPen2,hPen3;
HFONT hFont,hFont2,hFont4,hTitleFont;
WORD playTime,playTime2End;		// 
enum TIPO_GIOCO tipoGioco=GIOCO_TENNIS,savedTipoGioco;
BYTE batSize=2,ballSpeed=MAX_BALL_SPEED,ballAngle=1,autoServe=1,giocoTempoPunti=1;
WORD score[2],credit;
int nauLChar[2]={VK_RIGHT,'Q'},nauRChar[2]={VK_LEFT,'A'},nauFChar[2]={VK_SPACE,'Z'};
/*BYTE sinTable[16]={				// 0..90 reale
	0,12,24,37,48,60,70,80,
	90,98,106,112,117,122,125,127
	};*/
BYTE sinTable[8]={				// uso 0..45�
//	 12, 38, 60, 78, 94,108,119,128
	 32, 32+24, 32+24+22, 32+24+22+18, 32+24+22+18+14,32+24+22+18+14+10,32+24+22+18+14+10+6,128
	};

BYTE lastTouched;
struct MOB myMob[10];
HDC hCompDC;
UINT hTimer;
WORD demoTime;

enum PLAY_STATE bPlayMode;
enum SUB_PLAY_STATE subPlayMode;
DWORD subPlayModeTime;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;
	HANDLE hAccelTable;

	if(!hPrevInstance) {
		if(!InitApplication(hInstance)) {
			return (FALSE);
		  }
	  }

	if(!InitInstance(hInstance, nCmdShow)) {
		return (FALSE);
  	}

	if(*lpCmdLine) {
		PostMessage(ghWnd,WM_USER+1,0,(LPARAM)lpCmdLine);
		}
	hAccelTable = LoadAccelerators (hInstance,MAKEINTRESOURCE(IDR_ACCELERATOR1));
	while(GetMessage(&msg, NULL, 0, 0)) {
		if(!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

  return (msg.wParam);
	}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId,wmEvent;
	PAINTSTRUCT ps;
	HDC hDC;
 	POINT pnt;
	HMENU hMenu;
 	BOOL bGotHelp;
	int i,j,k;
	long l;
	char myBuf[128];
	LOGBRUSH br;
	RECT rc;
	SIZE mySize;
	static int TimerState=-1,TimerCnt;
	static BYTE introScreen=0;
	HFONT hOldFont;
	HPEN hOldPen;
	struct MOB *mp;

	switch(message) { 
		case WM_COMMAND:
			wmId    = LOWORD(wParam); // Remember, these are...
			wmEvent = HIWORD(wParam); // ...different for Win32!

			switch(wmId) {
				case ID_APP_ABOUT:
					DialogBox(g_hinst,MAKEINTRESOURCE(IDD_ABOUT),hWnd,(DLGPROC)About);
					break;

				case ID_APP_EXIT:
					PostMessage(hWnd,WM_CLOSE,0,0l);
					break;

				case ID_FILE_NEW:
					if(credit>0)			// disattivare se credit=0??
						credit--;
					playTime=0;
					if(!giocoTempoPunti)
						playTime2End=MAX_TEMPO;
					score[0]=score[1]=0;
					TimerCnt=19;			// forza redraw iniziale!
					bPlayMode=PLAY_STARTING;

//					loadMobs();

					RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_ERASE);
					// continua a venir cancellata DOPO...

updPlay:
					TimerState=-1;
					break;

				case ID_FILE_CLOSE:
					InvalidateRect(hWnd,NULL,TRUE);
					bPlayMode=PLAY_IDLE;
					demoTime=600;
					introScreen=0;
					goto updPlay;
					break;

				case ID_FILE_UPDATE:
					if(bPlayMode==PLAY_PLAY) {
						bPlayMode=PLAY_PAUSED;
						SetWindowText(hWnd,APPNAME" (in pausa)");
						}
					else {
						bPlayMode=PLAY_PLAY;
						SetWindowText(hWnd,APPNAME);
						}
					break;

				case ID_OPZIONI_ATEMPO:
					giocoTempoPunti=0;
					InvalidateRect(hWnd,NULL,TRUE);
          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;
				case ID_OPZIONI_APUNTI:
					giocoTempoPunti=1;
					InvalidateRect(hWnd,NULL,TRUE);
          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;

				case ID_OPZIONI_RACCHETTEPICCOLE:
					batSize= 1;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_RACCHETTEGRANDI:
					batSize=2;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_VELOCITA_NORMALE:
					ballSpeed=MAX_BALL_SPEED;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_VELOCITRIDOTTA:
					ballSpeed=MAX_BALL_SPEED/2;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_ANGOLI20:
					ballAngle=1;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_ANGOLI40:
					ballAngle=2;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_SERVIZIOAUTOMATICO:
					autoServe=1;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_SERVIZIOMANUALE:
					autoServe=0;
					InvalidateRect(hWnd,NULL,TRUE);
					break;

				case ID_OPZIONI_DIMENSIONEDOPPIA:
					doubleSize= doubleSize==1 ? 2 : 1;
					InvalidateRect(hWnd,NULL,TRUE);
          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;

				case ID_OPZIONI_GIOCO_TENNIS:
					savedTipoGioco=tipoGioco=GIOCO_TENNIS;
					InvalidateRect(hWnd,NULL,TRUE);
//??          MessageBox(GetFocus(),"Riavviare il gioco!",szAppName,MB_OK|MB_ICONEXCLAMATION);
					break;
				case ID_OPZIONI_GIOCO_HOCKEY:
					savedTipoGioco=tipoGioco=GIOCO_HOCKEY;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_GIOCO_SQUASH:
					savedTipoGioco=tipoGioco=GIOCO_TENNIS;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_GIOCO_PELOTA:
					savedTipoGioco=tipoGioco=GIOCO_PELOTA;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
				case ID_OPZIONI_GIOCO_FUCILE:
					savedTipoGioco=tipoGioco=GIOCO_FUCILE;
					InvalidateRect(hWnd,NULL,TRUE);
					break;
/*				case ID_OPZIONI_GIOCO_FUCILE2:
					savedTipoGioco=tipoGioco=GIOCO_FUCILE2;
					InvalidateRect(hWnd,NULL,TRUE);
					break;*/

				case ID_OPZIONI_SUONI:
					bSuoni=!bSuoni;
					break;

				case ID_EDIT_PASTE:
					break;

        case ID_HELP: // Only called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_FINDER,(DWORD)0);
          if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_INDEX: // Not called in Windows 95
          bGotHelp = WinHelp(hWnd, APPNAME".HLP", HELP_CONTENTS,(DWORD)0);
		      if(!bGotHelp) {
            MessageBox(GetFocus(),"Unable to activate help",
              szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_FINDER: // Not called in Windows 95
          if(!WinHelp(hWnd, APPNAME".HLP", HELP_PARTIALKEY,	(DWORD)(LPSTR)"")) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName,MB_OK|MB_ICONHAND);
					  }
					break;

				case ID_HELP_USING: // Not called in Windows 95
					if(!WinHelp(hWnd, (LPSTR)NULL, HELP_HELPONHELP, 0)) {
						MessageBox(GetFocus(),"Unable to activate help",
							szAppName, MB_OK|MB_ICONHAND);
					  }
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));
					break;
				}
			break;

		case WM_NCRBUTTONUP: // RightClick on windows non-client area...
			if(IS_WIN95 && SendMessage(hWnd, WM_NCHITTEST, 0, lParam) == HTSYSMENU) {
				// The user has clicked the right button on the applications
				// 'System Menu'. Here is where you would alter the default
				// system menu to reflect your application. Notice how the
				// explorer deals with this. For this app, we aren't doing
				// anything
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
			else {
				// Nothing we are interested in, allow default handling...
				return DefWindowProc(hWnd, message, wParam, lParam);
			  }
      break;

    case WM_RBUTTONDOWN: // RightClick in windows client area...
      pnt.x = LOWORD(lParam);
      pnt.y = HIWORD(lParam);
      ClientToScreen(hWnd, (LPPOINT)&pnt);
      hMenu = GetSubMenu(GetMenu(hWnd),2);
      if(hMenu) {
        TrackPopupMenu(hMenu, 0, pnt.x, pnt.y, 0, hWnd, NULL);
        }
      break;

		case WM_PAINT:
			hDC=BeginPaint(hWnd,&ps);
			hOldPen=SelectObject(hDC,hPen1);
			hOldFont=SelectObject(hDC,hFont2);
			switch(bPlayMode) {
				case PLAY_IDLE:
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					goto plot_credit;
					break;
				case PLAY_STARTING:
				case PLAY_PLAY:
				case PLAY_PAUSED:
				case PLAY_DEMO:
				case PLAY_ENDING:
					hOldPen=SelectObject(hDC,hPen2);
					switch(tipoGioco) {
						case GIOCO_TENNIS:
						case GIOCO_HOCKEY:		// fare "porte + piccole" qua?? v. foto in giro...
						case GIOCO_SQUASH:
						case GIOCO_PELOTA:
							MoveToEx(hDC,5*doubleSize,5*doubleSize,NULL);
							LineTo(hDC,AppXSizeR-5*doubleSize,5*doubleSize);
							MoveToEx(hDC,5*doubleSize,AppYSizeR-5*doubleSize,NULL);
							LineTo(hDC,AppXSizeR-5*doubleSize,AppYSizeR-5*doubleSize);
							if(tipoGioco<GIOCO_SQUASH) {		// in alcune foto � al contrario, tratteggio il bordo e intera la rete...
								SelectObject(hDC,hPen3);
								MoveToEx(hDC,AppXSizeR/2,8*doubleSize,NULL);
								LineTo(hDC,AppXSizeR/2,AppYSizeR-5*doubleSize);
								}
							else {
								MoveToEx(hDC,5*doubleSize,5*doubleSize,NULL);
								LineTo(hDC,5*doubleSize,AppYSizeR-5*doubleSize);
								}
							SetTextColor(hDC,RGB(255,255,255));
							SetBkColor(hDC,RGB(0,0,0));
							hOldFont=SelectObject(hDC,hFont4);
							if(tipoGioco != GIOCO_PELOTA) {
								wsprintf(myBuf,"%02u",score[0]);
								TextOut(hDC,AppXSizeR/2-54*doubleSize,15*doubleSize,myBuf,_tcslen(myBuf));
								wsprintf(myBuf,"%02u",score[1]);
								TextOut(hDC,AppXSizeR/2+12*doubleSize,15*doubleSize,myBuf,strlen(myBuf));
								}
							else {
								wsprintf(myBuf,"%02u",score[0]);
								TextOut(hDC,AppXSizeR/2-10*doubleSize,15*doubleSize,myBuf,strlen(myBuf));
								}
							break;
						case GIOCO_FUCILE:
						case GIOCO_FUCILE2:
							SetTextColor(hDC,RGB(255,255,255));
							SetBkColor(hDC,RGB(0,0,0));
							hOldFont=SelectObject(hDC,hFont4);
							wsprintf(myBuf,"%02u",score[0]);
							TextOut(hDC,AppXSizeR/2-10*doubleSize,15*doubleSize,myBuf,strlen(myBuf));
							break;
						}
plot_credit:
					hOldFont=SelectObject(hDC,hFont2);
					if(!giocoTempoPunti) {
						wsprintf(myBuf,"TIME %02u",playTime2End);
						TextOut(hDC,32*doubleSize,AppYSizeR-14*doubleSize,myBuf,_tcslen(myBuf));
						}
					wsprintf(myBuf,"CREDIT %02u",credit);
					TextOut(hDC,AppXSizeR-98*doubleSize,AppYSizeR-14*doubleSize,myBuf,_tcslen(myBuf));
					break;
				}
			SelectObject(hDC,hOldFont);
			SelectObject(hDC,hOldPen);
			EndPaint(hWnd,&ps);
			break;        

		case WM_SIZE:
			GetClientRect(hWnd,&rc);
			MoveWindow(hStatusWnd,0,rc.bottom-10*doubleSize,rc.right,10*doubleSize,1);
			break;        

		case WM_KEYDOWN:
			switch(bPlayMode) {
				case PLAY_PLAY:
					i=(TCHAR)wParam;
					if(tipoGioco==GIOCO_PELOTA) {
						if(i==nauLChar[0]) {
							myMob[4].speed.y=-BAT_SPEED*doubleSize;
							}
						else if(i==nauRChar[0]) {
							myMob[4].speed.y=+BAT_SPEED*doubleSize;
							}
						else if(i==nauFChar[0]) {
							if(subPlayMode==SUBPLAY_WAITSERVE) {
								subPlayMode=SUBPLAY_WAITSERVEDONE;
								}
							}
						}
					else {
						if(i==nauLChar[1]) {
							myMob[4].speed.y=-BAT_SPEED*doubleSize;
							if(myMob[6].bVis)
								myMob[6].speed.y=-BAT_SPEED*doubleSize;
							}
						else if(i==nauRChar[1]) {
							myMob[4].speed.y=+BAT_SPEED*doubleSize;
							if(myMob[6].bVis)
								myMob[6].speed.y=+BAT_SPEED*doubleSize;
							}
						else if(i==nauFChar[1]) {
							if(subPlayMode==SUBPLAY_WAITSERVE) {
								subPlayMode=SUBPLAY_WAITSERVEDONE;
								}
							}
						if(i==nauLChar[0]) {
							myMob[5].speed.y=-BAT_SPEED*doubleSize;
							if(myMob[7].bVis)
								myMob[7].speed.y=-BAT_SPEED*doubleSize;
							}
						else if(i==nauRChar[0]) {
							myMob[5].speed.y=+BAT_SPEED*doubleSize;
							if(myMob[7].bVis)
								myMob[7].speed.y=+BAT_SPEED*doubleSize;
							}
						else if(i==nauFChar[0]) {
							if(subPlayMode==SUBPLAY_WAITSERVE) {
								subPlayMode=SUBPLAY_WAITSERVEDONE;
								}
							}
						}
					break;
				case PLAY_IDLE:
					switch(wParam) {
						case VK_UP:
						case VK_LEFT:
							if(tipoGioco>0)
								tipoGioco--;
							savedTipoGioco=tipoGioco;
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
							break;
						case VK_DOWN:
						case VK_RIGHT:
							if(tipoGioco<5 /* GIOCO_FUCILE2*/)
								tipoGioco++;
							savedTipoGioco=tipoGioco;
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
							break;
						case VK_RETURN:
							break;
						}
					break;
				}
			break;        
		case WM_KEYUP:
			i=(TCHAR)wParam;
			if(tipoGioco==GIOCO_PELOTA) {
				if(i==nauLChar[0] || i==nauRChar[0]) {
					myMob[4].speed.y=0;
					}
				}
			else {
				if(i==nauLChar[1] || i==nauRChar[1]) {
					myMob[4].speed.y=0;
					if(myMob[6].bVis)
						myMob[6].speed.y=0;
					}
				if(i==nauLChar[0] || i==nauRChar[0]) {
					myMob[5].speed.y=0;
					if(myMob[7].bVis)
						myMob[7].speed.y=0;
					}
				}
			break;        

		case WM_CREATE:
//			bInFront=GetPrivateProfileInt(APPNAME,"SempreInPrimoPiano",0,INIFile);

			srand( (unsigned)time( NULL ) );  

			doubleSize=GetPrivateProfileInt(APPNAME,"DoubleSize",1,INIFile);
			bSuoni=GetPrivateProfileInt(APPNAME,"Suoni",1,INIFile);
			savedTipoGioco=tipoGioco=GetPrivateProfileInt(APPNAME,"TipoGioco",1,INIFile);
			batSize=GetPrivateProfileInt(APPNAME,"BatSize",2,INIFile);
			ballSpeed=GetPrivateProfileInt(APPNAME,"BallSpeed",MAX_BALL_SPEED,INIFile);
			ballAngle=GetPrivateProfileInt(APPNAME,"BallAngle",1,INIFile);
			giocoTempoPunti=GetPrivateProfileInt(APPNAME,"GiocoTempoPunti",1,INIFile);
			autoServe=GetPrivateProfileInt(APPNAME,"AutoServe",1,INIFile);
			AppXSize=APP_XSIZE*doubleSize;
			AppYSize=APP_YSIZE*doubleSize;

			hFont=CreateFont(8*doubleSize,4*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_MODERN, (LPSTR)"Courier New");
			hFont2=CreateFont(12*doubleSize,6*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"HD44780 regular" /*"Arial"*/);
			hFont4=CreateFont(36*doubleSize,18*doubleSize,0,0,FW_LIGHT,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"Bit5x3 regular" /*"Arial"*/);
			hTitleFont=CreateFont(16*doubleSize,8*doubleSize,0,0,FW_NORMAL,0,0,0,
				ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS, (LPSTR)"HD44780 regular" /*"Arial"*/);


			GetWindowRect(hWnd,&rc);
			rc.right=rc.left+AppXSize;
			rc.bottom=rc.top+AppYSize;
			MoveWindow(hWnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,TRUE);

			GetClientRect(hWnd,&rc);
			hStatusWnd = CreateWindow("static","",
				WS_BORDER | SS_LEFT | WS_CHILD,
				0,rc.bottom-10*doubleSize,AppXSize-GetSystemMetrics(SM_CXVSCROLL)-2*GetSystemMetrics(SM_CXSIZEFRAME),10*doubleSize,
				hWnd,1001,g_hinst,NULL);
			ShowWindow(hStatusWnd, SW_SHOW);
			GetClientRect(hWnd,&rc);
			AppXSizeR=rc.right-rc.left;
			AppYSizeR=rc.bottom-rc.top-12*doubleSize;
			SendMessage(hStatusWnd,WM_SETFONT,(WPARAM)hFont,0);
			hPen1=CreatePen(PS_SOLID,1,RGB(255,255,255));
			hPen2=CreatePen(PS_SOLID,4*doubleSize,RGB(255,255,255));
			br.lbStyle=BS_SOLID;
			br.lbColor=0xFFFFFF;
			br.lbHatch=0;
			hPen3=ExtCreatePen(PS_GEOMETRIC | PS_DOT | PS_ENDCAP_FLAT | PS_JOIN_BEVEL,4*doubleSize,&br,0,NULL);
			br.lbStyle=BS_SOLID;
			br.lbColor=0x000000;
			br.lbHatch=0;
			hBrush=CreateBrushIndirect(&br);
			br.lbStyle=BS_SOLID;
			br.lbColor=GetSysColor(COLOR_MENU);
			br.lbHatch=0;
			hBrush2=CreateBrushIndirect(&br);
			loadMobs(3);
			hDC=GetDC(hWnd);
			hCompDC=CreateCompatibleDC(hDC);
			ReleaseDC(hWnd,hDC);

			credit=0;

			timeBeginPeriod(1);		// � lento.. e non fa nulla..
			hTimer=SetTimer(hWnd,1,TIMER_GRANULARITY,NULL);

#ifdef _DEBUG
			SetWindowText(hStatusWnd,"<debugmode>");
// tipoGioco!!			SetWindowText(hStatusWnd,myBuf);
#endif
			break;

		case WM_TIMER:
			TimerCnt++;
			switch(bPlayMode) {
				case PLAY_STARTING:
					tipoGioco=savedTipoGioco;
					hDC=GetDC(hWnd);
					SetTextColor(hDC,RGB(255,255,255));
					SetBkColor(hDC,RGB(0,0,0));
					hOldFont=SelectObject(hDC,hFont4);

					loadMobs(3);
					lastTouched=0;

					mp=initBall(NULL);
					hDC=GetDC(hWnd);
//					SelectObject(hDC,hBrush);
//					PatBlt(hDC,mp->x,mp->y,mp->s.cx,mp->s.cy,PATCOPY);
					MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);

					ReleaseDC(hWnd,hDC);
					bPlayMode=PLAY_PLAY;
					if(autoServe)
						subPlayMode=SUBPLAY_NONE;
					else {
						int x=mp->speed.x,y=mp->speed.y;
						mp->speed.x=mp->speed.y=0;		// se no si muove un attimo!
						animateMobs(hWnd);
						mp->speed.x=x;mp->speed.y=y;
						subPlayMode=SUBPLAY_WAITSERVE;
						}
					break;

				case PLAY_PLAY:
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							animateMobs(hWnd);


							if(!(TimerCnt % 50)) {
								playTime++;
								if(!giocoTempoPunti) {
									RECT rc={30*doubleSize,AppYSizeR-16*doubleSize,(38+80)*doubleSize,AppYSizeR-2*doubleSize};
									if(!--playTime2End)
										bPlayMode=PLAY_ENDING;
									InvalidateRect(hWnd,&rc,TRUE);
									}
								}
							break;
						case SUBPLAY_HITBORDER:
							subPlayMode=SUBPLAY_NONE;
							break;
						case SUBPLAY_HITBAT:
							switch(tipoGioco) {
								case GIOCO_TENNIS:
									break;
								case GIOCO_HOCKEY:
									break;
								case GIOCO_SQUASH:
									break;
								case GIOCO_PELOTA:
									break;
								case GIOCO_FUCILE:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;
									break;
								case GIOCO_FUCILE2:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;
									break;
								}
							subPlayMode=SUBPLAY_NONE;
							break;

						case SUBPLAY_BALLOUT:
							switch(tipoGioco) {
								case GIOCO_TENNIS:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI || score[1]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;
									break;
								case GIOCO_HOCKEY:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI || score[1]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;

									break;
								case GIOCO_SQUASH:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI || score[1]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;
									break;
								case GIOCO_PELOTA:
									if(giocoTempoPunti)
										if(score[0]>=MAX_PUNTI)
											bPlayMode=PLAY_ENDING;
									break;
								case GIOCO_FUCILE:
									break;
								case GIOCO_FUCILE2:
									break;
								}
							subPlayMode=SUBPLAY_NONE;
							InvalidateRect(hWnd,NULL,TRUE);
//							RedrawWindow(hWnd,NULL,NULL,RDW_ERASE | RDW_UPDATENOW | RDW_ERASENOW);
							UpdateWindow(hWnd);// per farlo subito, prima di Mobs!
							if(bPlayMode == PLAY_PLAY) {
								if(autoServe)
									initBall(NULL);
								else {
									loadMobs(1);
									animateMobs(hWnd);
									subPlayMode=SUBPLAY_WAITSERVE;
									}
								}
							break;

						case SUBPLAY_WAITSERVE:
							break;
						case SUBPLAY_WAITSERVEDONE:
							initBall(NULL);
							if(bSuoni)
								PlayResource(MAKEINTRESOURCE(IDR_WAVE_SERVE));
							subPlayMode=SUBPLAY_NONE;
							break;
						}


					break;

				case PLAY_PAUSED:

					subPlayMode=SUBPLAY_NONE;
					break;

				case PLAY_ENDING:
					demoTime=600;
					introScreen=0;
					switch(subPlayMode) {
						case SUBPLAY_NONE:
							hDC=GetDC(hWnd);
							hOldFont=SelectObject(hDC,hTitleFont);
							TextOut(hDC,AppXSizeR/2-43*doubleSize,AppYSizeR/2-15*doubleSize,"GAME OVER",9);
							SelectObject(hDC,hOldFont);
							ReleaseDC(hWnd,hDC);
							subPlayModeTime=timeGetTime()+3000;		// 
							subPlayMode=SUBPLAY_ENDGAME;
							break;
						case SUBPLAY_ENDGAME:
							if(subPlayModeTime<timeGetTime()) {
								subPlayMode=SUBPLAY_NONE;
								PostMessage(hWnd,WM_COMMAND,ID_FILE_CLOSE,0);
								InvalidateRect(hWnd,NULL,TRUE);
								}
							break;
						}
					
					break;

				case PLAY_IDLE:
					if(!(TimerCnt % 15)) {
						TimerState++;
						switch(TimerState) {
							HBITMAP hImg;
							case 0:
//								InvalidateRect(hWnd,NULL,TRUE);
								hDC=GetDC(hWnd);
								hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(IDB_PONG));
								SelectObject(hCompDC,hImg);
								StretchBlt(hDC,0,0,AppXSizeR,AppYSizeR,hCompDC,0,0,400,300,SRCCOPY);
								ReleaseDC(hWnd,hDC);
								break;
							case 1:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(255,155,55));
								SetBkColor(hDC,RGB(0,0,0));
								hOldFont=SelectObject(hDC,hTitleFont);
								LoadString(g_hinst,IDS_OPENSTRING8,myBuf,64);
								TextOut(hDC,72*doubleSize,53*doubleSize,myBuf,strlen(myBuf));
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 2:
								hDC=GetDC(hWnd);
								SetTextColor(hDC,RGB(220,120,35));
								SetBkColor(hDC,RGB(0,0,0));
								hOldFont=SelectObject(hDC,hTitleFont);
								for(i=0; i<6; i++) {
									LoadString(g_hinst,IDS_OPENSTRING2+i,myBuf,64);
									TextOut(hDC,AppXSizeR/2-20*doubleSize,(95+i*25)*doubleSize,myBuf,strlen(myBuf));
									if(tipoGioco==i) {
										MobDrawXY(hDC,&myMob[0],AppXSizeR/2-myMob[0].s.cx-32*doubleSize,(99+i*25)*doubleSize);
										}
									}
								SelectObject(hDC,hOldFont);
								ReleaseDC(hWnd,hDC);
								break;
							case 3:
								introScreen++;
								if(introScreen>3) {
									bPlayMode=PLAY_DEMO;
									savedTipoGioco=tipoGioco;
									tipoGioco= rand() % 5 /*GIOCO_FUCILE+1*/;
									InvalidateRect(hWnd,NULL,TRUE);
									loadMobs(3);
									initBall(NULL);
									score[0]=score[1]=0;
									demoTime=600;
									}
								TimerState=-1;
								break;
							}
						}
					break;
				case PLAY_DEMO:
				  animateMobs(hWnd);
					hDC=GetDC(hWnd);
					if(!(TimerCnt % 15)) {

						// usaRE initBall!
						if(!myMob[0].bVis)
							initBall(NULL);

						if(tipoGioco<GIOCO_FUCILE) {
							mp=&myMob[4];
							SelectObject(hDC,hBrush);
							PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
							if(rand() > 20000) {
								if(mp->y.whole < (AppYSizeR - LOWER_BORDER*doubleSize))
									mp->speed.y=((rand() & 1) + 3)*doubleSize;
								else
									mp->speed.y=0;
								}
							else if(rand() > 10000) {
								if(mp->y.whole > UPPER_BORDER*doubleSize)
									mp->speed.y=-(((rand() & 1) + 3)*doubleSize);
								else
									mp->speed.y=0;
								}
							MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
							if(tipoGioco != GIOCO_PELOTA) {
								mp=&myMob[5];
								SelectObject(hDC,hBrush);
								PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
								if(rand() > 20000) {
									if(mp->y.whole < (AppYSizeR - LOWER_BORDER*doubleSize))
										mp->speed.y=((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								else if(rand() > 10000) {
									if(mp->y.whole > UPPER_BORDER*doubleSize)
										mp->speed.y=-((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
								}
							if(tipoGioco==GIOCO_HOCKEY) {
								mp=&myMob[6];
								SelectObject(hDC,hBrush);
								PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
								if(rand() > 20000) {
									if(mp->y.whole < (AppYSizeR - LOWER_BORDER*doubleSize))
										mp->speed.y=((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								else if(rand() > 10000) {
									if(mp->y.whole > UPPER_BORDER*doubleSize)
										mp->speed.y=-((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
								mp=&myMob[7];
								SelectObject(hDC,hBrush);
								PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
								if(rand() > 20000) {
									if(mp->y.whole < (AppYSizeR - LOWER_BORDER*doubleSize))
										mp->speed.y=((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								else if(rand() > 10000) {
									if(mp->y.whole > UPPER_BORDER*doubleSize)
										mp->speed.y=-((rand() & 1) + 3)*doubleSize;
									else
										mp->speed.y=0;
									}
								MobDrawXY(hDC,mp,mp->x.whole,mp->y.whole);
								}
							}
						}
					ReleaseDC(hWnd,hDC);

					demoTime--;
					if(!demoTime) {
						introScreen=0;
						tipoGioco=savedTipoGioco;
						bPlayMode=PLAY_IDLE;
						InvalidateRect(hWnd,NULL,TRUE);
						}
					break;
				}
			break;

		case WM_CLOSE:
			if(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED || bPlayMode==PLAY_STARTING || bPlayMode==PLAY_ENDING) {
				if(MessageBox(hWnd,"Terminare la partita in corso?",APPNAME,MB_YESNO | MB_DEFBUTTON2)==IDYES)
					DestroyWindow(hWnd);
			  }
			else
				DestroyWindow(hWnd);
			break;

		case WM_DESTROY:
			WritePrivateProfileInt(APPNAME,"DoubleSize",doubleSize,INIFile);
			WritePrivateProfileInt(APPNAME,"Suoni",bSuoni,INIFile);
			WritePrivateProfileInt(APPNAME,"TipoGioco",tipoGioco,INIFile);
			WritePrivateProfileInt(APPNAME,"BatSize",batSize,INIFile);
			WritePrivateProfileInt(APPNAME,"BallSpeed",ballSpeed,INIFile);
			WritePrivateProfileInt(APPNAME,"BallAngle",ballAngle,INIFile);
			WritePrivateProfileInt(APPNAME,"GiocoTempoPunti",giocoTempoPunti,INIFile);
			WritePrivateProfileInt(APPNAME,"AutoServe",autoServe,INIFile);
//			WritePrivateProfileInt(APPNAME,"SempreInPrimoPiano",bInFront,INIFile);
			timeEndPeriod(1);
			KillTimer(hWnd,hTimer);
			// Tell WinHelp we don't need it any more...
	    WinHelp(hWnd,APPNAME".HLP",HELP_QUIT,(DWORD)0);
			DeleteDC(hCompDC);
			DeleteObject(hBrush);
			DeleteObject(hBrush2);
			DeleteObject(hPen1);
			DeleteObject(hPen2);
			DeleteObject(hFont);
			DeleteObject(hFont2);
			DeleteObject(hFont4);
			DeleteObject(hTitleFont);
			PostQuitMessage(0);
			break;

   	case WM_INITMENU:
   	  EnableMenuItem((HMENU)wParam,ID_FILE_NEW,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_UPDATE,(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  EnableMenuItem((HMENU)wParam,ID_FILE_CLOSE,(bPlayMode==PLAY_PLAY || bPlayMode==PLAY_PAUSED) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_DIMENSIONEDOPPIA,doubleSize==2 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_SUONI,bSuoni ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_RACCHETTEPICCOLE,batSize==1 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_RACCHETTEGRANDI,batSize==2 ? MF_CHECKED : MF_UNCHECKED);
//   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_RACCHETTEPICCOLE,batSize==1 ? MF_GRAYED : MF_ENABLED);
//   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_RACCHETTEGRANDI,batSize==2 ? MF_GRAYED : MF_ENABLED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_VELOCITA_NORMALE,ballSpeed==MAX_BALL_SPEED ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_VELOCITRIDOTTA,ballSpeed!=MAX_BALL_SPEED ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_ANGOLI20,ballAngle==1 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_ANGOLI40,ballAngle==2 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_SERVIZIOAUTOMATICO,autoServe==1 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_SERVIZIOMANUALE,autoServe==0 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_ATEMPO,giocoTempoPunti==0 ? MF_CHECKED : MF_UNCHECKED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_APUNTI,giocoTempoPunti==1 ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_TENNIS,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_TENNIS,tipoGioco==GIOCO_TENNIS ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_TENNIS,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_HOCKEY,tipoGioco==GIOCO_HOCKEY ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_HOCKEY,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_SQUASH,tipoGioco==GIOCO_SQUASH ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_SQUASH,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_PELOTA,tipoGioco==GIOCO_PELOTA ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_PELOTA,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_FUCILE,tipoGioco==GIOCO_FUCILE ? MF_CHECKED : MF_UNCHECKED);
   	  EnableMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_FUCILE,(bPlayMode==PLAY_IDLE || bPlayMode==PLAY_DEMO) ? MF_ENABLED : MF_GRAYED);
//   	  CheckMenuItem((HMENU)wParam,ID_OPZIONI_GIOCO_FUCILE2,tipoGioco==GIOCO_FUCILE2 ? MF_CHECKED : MF_UNCHECKED);

			break;

		case WM_CTLCOLORSTATIC:
			SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
			SetBkColor((HDC)wParam,GetSysColor(COLOR_MENU));
			return (long)hBrush2;
			break;

		default:
			return (DefWindowProc(hWnd, message, wParam, lParam));
		}
	return (0);
	}



ATOM MyRegisterClass(CONST WNDCLASS *lpwc) {
	HANDLE  hMod;
	FARPROC proc;
	WNDCLASSEX wcex;

	hMod=GetModuleHandle("USER32");
	if(hMod != NULL) {

#if defined (UNICODE)
		proc = GetProcAddress (hMod, "RegisterClassExW");
#else
		proc = GetProcAddress (hMod, "RegisterClassExA");
#endif

		if(proc != NULL) {
			wcex.style         = lpwc->style;
			wcex.lpfnWndProc   = lpwc->lpfnWndProc;
			wcex.cbClsExtra    = lpwc->cbClsExtra;
			wcex.cbWndExtra    = lpwc->cbWndExtra;
			wcex.hInstance     = lpwc->hInstance;
			wcex.hIcon         = lpwc->hIcon;
			wcex.hCursor       = lpwc->hCursor;
			wcex.hbrBackground = lpwc->hbrBackground;
    	wcex.lpszMenuName  = lpwc->lpszMenuName;
			wcex.lpszClassName = lpwc->lpszClassName;

			// Added elements for Windows 95:
			wcex.cbSize = sizeof(WNDCLASSEX);
			wcex.hIconSm = LoadIcon(wcex.hInstance, "SMALL");
			
			return (*proc)(&wcex);//return RegisterClassEx(&wcex);
		}
	}
return (RegisterClass(lpwc));
}


BOOL InitApplication(HINSTANCE hInstance) {
  WNDCLASS  wc;
  HWND      hwnd;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = (WNDPROC)WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_APP32));
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(GetStockObject(BLACK_BRUSH));

        // Since Windows95 has a slightly different recommended
        // format for the 'Help' menu, lets put this in the alternate menu like this:
  if(IS_WIN95) {
		wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    } else {
	  wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU1);
    }
  wc.lpszClassName = szAppName;

  if(IS_WIN95) {
	  if(!MyRegisterClass(&wc))
			return 0;
    }
	else {
	  if(!RegisterClass(&wc))
	  	return 0;
    }


  }

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	
	g_hinst=hInstance;

	ghWnd = CreateWindow(szAppName, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX /*| WS_MAXIMIZEBOX */ | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT, AppXSize,AppYSize,
		NULL, NULL, hInstance, NULL);

	if(!ghWnd) {
		return (FALSE);
	  }

	ShowWindow(ghWnd, nCmdShow);
	UpdateWindow(ghWnd);

	return (TRUE);
  }

//
//  FUNCTION: About(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for "About" dialog box
// 		This version allows greater flexibility over the contents of the 'About' box,
// 		by pulling out values from the 'Version' resource.
//
//  MESSAGES:
//
//	WM_INITDIALOG - initialize dialog box
//	WM_COMMAND    - Input received
//
//
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static  HFONT hfontDlg;		// Font for dialog text
	static	HFONT hFinePrint;	// Font for 'fine print' in dialog
	DWORD   dwVerInfoSize;		// Size of version information block
	LPSTR   lpVersion;			// String pointer to 'version' text
	DWORD   dwVerHnd=0;			// An 'ignored' parameter, always '0'
	UINT    uVersionLen;
	WORD    wRootLen;
	BOOL    bRetCode;
	int     i;
	char    szFullPath[256];
	char    szResult[256];
	char    szGetName[256];
	DWORD	dwVersion;
	char	szVersion[40];
	DWORD	dwResult;

	switch (message) {
    case WM_INITDIALOG:
//			ShowWindow(hDlg, SW_HIDE);
			hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
			hFinePrint = CreateFont(11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				VARIABLE_PITCH | FF_SWISS, "");
//			CenterWindow (hDlg, GetWindow (hDlg, GW_OWNER));
			GetModuleFileName(g_hinst, szFullPath, sizeof(szFullPath));

			// Now lets dive in and pull out the version information:
			dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
			if(dwVerInfoSize) {
				LPSTR   lpstrVffInfo;
				HANDLE  hMem;
				hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
				lpstrVffInfo  = GlobalLock(hMem);
				GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
				// The below 'hex' value looks a little confusing, but
				// essentially what it is, is the hexidecimal representation
				// of a couple different values that represent the language
				// and character set that we are wanting string values for.
				// 040904E4 is a very common one, because it means:
				//   US English, Windows MultiLingual characterset
				// Or to pull it all apart:
				// 04------        = SUBLANG_ENGLISH_USA
				// --09----        = LANG_ENGLISH
				// ----04E4 = 1252 = Codepage for Windows:Multilingual
				lstrcpy(szGetName, "\\StringFileInfo\\040904E4\\");	 
				wRootLen = lstrlen(szGetName); // Save this position
			
				// Set the title of the dialog:
				lstrcat (szGetName, "ProductName");
				bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
					(LPSTR)szGetName,
					(LPVOID)&lpVersion,
					(UINT *)&uVersionLen);
//				lstrcpy(szResult, "About ");
//				lstrcat(szResult, lpVersion);
//				SetWindowText (hDlg, szResult);

				// Walk through the dialog items that we want to replace:
				for(i=DLG_VERFIRST; i <= DLG_VERLAST; i++) {
					GetDlgItemText(hDlg, i, szResult, sizeof(szResult));
					szGetName[wRootLen] = (char)0;
					lstrcat (szGetName, szResult);
					uVersionLen   = 0;
					lpVersion     = NULL;
					bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
						(LPSTR)szGetName,
						(LPVOID)&lpVersion,
						(UINT *)&uVersionLen);

					if(bRetCode && uVersionLen && lpVersion) {
					// Replace dialog item text with version info
						lstrcpy(szResult, lpVersion);
						SetDlgItemText(hDlg, i, szResult);
					  }
					else {
						dwResult = GetLastError();
						wsprintf (szResult, "Error %lu", dwResult);
						SetDlgItemText (hDlg, i, szResult);
					  }
					SendMessage (GetDlgItem (hDlg, i), WM_SETFONT, 
						(UINT)((i==DLG_VERLAST)?hFinePrint:hfontDlg),TRUE);
				  } // for


				GlobalUnlock(hMem);
				GlobalFree(hMem);

			}
		else {
				// No version information available.
			} // if (dwVerInfoSize)

    SendMessage(GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
			(WPARAM)hfontDlg,(LPARAM)TRUE);

			// We are  using GetVersion rather then GetVersionEx
			// because earlier versions of Windows NT and Win32s
			// didn't include GetVersionEx:
			dwVersion = GetVersion();

			if(dwVersion < 0x80000000) {
				// Windows NT
				wsprintf (szVersion, "Microsoft Windows NT %u.%u (Build: %u)",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
          (DWORD)(HIWORD(dwVersion)) );
				}
			else
				if(LOBYTE(LOWORD(dwVersion))<4) {
					// Win32s
				wsprintf (szVersion, "Microsoft Win32s %u.%u (Build: %u)",
  				(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))),
					(DWORD)(HIWORD(dwVersion) & ~0x8000) );
				}
			else {
					// Windows 95
				wsprintf(szVersion,"Microsoft Windows 95 %u.%u",
					(DWORD)(LOBYTE(LOWORD(dwVersion))),
					(DWORD)(HIBYTE(LOWORD(dwVersion))) );
				}

			SetWindowText(GetDlgItem(hDlg, IDC_OSVERSION), szVersion);
//			SetWindowPos(hDlg,NULL,GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2,0,0,SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);
//			ShowWindow(hDlg, SW_SHOW);
			return (TRUE);

		case WM_COMMAND:
			if(wParam==IDOK || wParam==IDCANCEL) {
  		  EndDialog(hDlg,0);
			  return (TRUE);
			  }
			else if(wParam==3) {
				MessageBox(hDlg,"Se trovate utile questo programma, mandate un contributo!!\nLunt rd. 121 - L205EZ Liverpool (England)\n[Dario Greggio]","ADPM Synthesis sas",MB_OK);
			  return (TRUE);
			  }
			break;
		}

	return FALSE;
	}


BOOL PlayResource(LPSTR lpName) { 
  BOOL bRtn; 
  LPSTR lpRes; 
  HANDLE hResInfo, hRes; 

  // Find the WAVE resource.  
  hResInfo = FindResource(g_hinst, lpName, "WAVE"); 
  if(hResInfo == NULL) 
    return FALSE; 

  // Load the WAVE resource. 
  hRes = LoadResource(g_hinst, hResInfo); 
  if(hRes == NULL) 
    return FALSE; 

  // Lock the WAVE resource and play it. 
  lpRes = LockResource(hRes); 
  if(lpRes != NULL) { 
    bRtn = sndPlaySound(lpRes, SND_MEMORY | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP); 
    UnlockResource(hRes); 
		} 
  else 
    bRtn = 0; 
 
  // Free the WAVE resource and return success or failure. 
 
  FreeResource(hRes); 
  return bRtn; 
	}

int WritePrivateProfileInt(char *s, char *s1, int n, char *f) {
//  int i;
  char myBuf[16];
  
  wsprintf(myBuf,"%d",n);
  return WritePrivateProfileString(s,s1,myBuf,f);
  }

int ShowMe(void) {
	int i;
	char buffer[16];

	buffer[0]='A'^ 0x17;
	buffer[1]='D'^ 0x17;
	buffer[2]='P'^ 0x17;
	buffer[3]='M'^ 0x17;
	buffer[4]='-'^ 0x17;
	buffer[5]='G'^ 0x17;
	buffer[6]='.'^ 0x17;
	buffer[7]='D'^ 0x17;
	buffer[8]='a'^ 0x17;
	buffer[9]='r'^ 0x17;
	buffer[10]=' '^ 0x17;
	buffer[11]='2'^ 0x17;
	buffer[12]='0' ^ 0x17;
	buffer[13]=0;
	for(i=0; i<13; i++) buffer[i]^=0x17;
	return MessageBox(GetDesktopWindow(),buffer,APPNAME,MB_OK | MB_ICONEXCLAMATION);
	}


