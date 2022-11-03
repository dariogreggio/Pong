//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#define STRICT

#define _INC_OLE
#include <windows.h>
#undef  _INC_OLE

#ifndef WIN32
#include <shell.h>
#endif

#include "afxres.h"
#include "resource.h"

extern HINSTANCE g_hinst;

union POS_FRACT {
	struct {
		unsigned int fract:2;
		unsigned int whole:14;
		};
	WORD pos;
	};
struct MOB {
	HANDLE hImg,hImgAlt;
	union POS_FRACT x,y;
	SIZE s;
	unsigned char bVis;
	BYTE mirrorX,mirrorY;
	POINT speed;
	unsigned int punti;
	int res1;          // usato per computare speed
	HANDLE hImgOld;
	BYTE bUseTransparency;		// evita flicker se non serve...
  };


#define BAT_POS 12		// 
#define BAT_SIZE_Y 40		// v. bmp, racchetta grande
#define BAT_SIZE_X 8
#define BALL_SIZE 8		// v. bmp (doppia per fucile)
#define BAT2_POS 100
#define BAT_SPEED 4
#define BALL_SPEED 2
#define MAX_BALL_SPEED 10 // opp. sizeof(sinTable)
#define MIN_BALL_SPEED (32/4) // v. sinTable
#define UPPER_LIMIT 6
#define LOWER_LIMIT 6
#define UPPER_BORDER (6+8)
#define LOWER_BORDER (6+8)
#define LEFT_BORDER (6+8)
#define RIGHT_BORDER (6+8)
#define SCORE_AREA 40
#define MAX_PUNTI 15
#define MAX_TEMPO 60

enum TIPO_GIOCO {
	GIOCO_TENNIS,
	GIOCO_HOCKEY,
	GIOCO_SQUASH,
	GIOCO_PELOTA,
	GIOCO_FUCILE,
	GIOCO_FUCILE2,
	};

enum PLAY_STATE {
	PLAY_IDLE=0,
	PLAY_PAUSED=-1,
	PLAY_STARTING=1,
	PLAY_PLAY=2,
	PLAY_ENDING=3,
	PLAY_DEMO=99
	};
enum SUB_PLAY_STATE {
	SUBPLAY_NONE=0,
	SUBPLAY_HITBORDER,
	SUBPLAY_HITBAT,
	SUBPLAY_BALLOUT,
	SUBPLAY_WAITSERVE,
	SUBPLAY_WAITSERVEDONE,
	SUBPLAY_ENDGAME
	};
extern enum PLAY_STATE bPlayMode;
extern enum SUB_PLAY_STATE subPlayMode;

ATOM MyRegisterClass(CONST WNDCLASS*);
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK About(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
int WritePrivateProfileInt(char *,char *,int,char *);
BOOL PlayResource(LPSTR lpName);

int animateMobs(HWND);
int loadMobs(BYTE);
int MobErase(HDC,struct MOB *);
int MobDraw(HDC,struct MOB *);
int MobMove(HDC,struct MOB *,SIZE) ;
int MobDrawXY(HDC,struct MOB *,WORD,WORD);
int MobCollisionRect(struct MOB *,RECT *);
int MobCollisionPoint(struct MOB *,POINT);
struct MOB *initBall(struct MOB *);
WORD getBallSpeed(BYTE s,BYTE a);
DWORD isBallInBat(struct MOB *);
BYTE isFucileInBall(struct MOB *);
BYTE isBallOut(struct MOB *);
BYTE hitBorder(struct MOB *);
void handleBounce(struct MOB *,BYTE);
void normalizeBallAngle(struct MOB *,BYTE);

