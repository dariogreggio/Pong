#include <windows.h>
#include "pong.h"


extern int AppXSize,AppYSize,AppXSizeR,AppYSizeR;
extern struct MOB myMob[10];
extern BYTE doubleSize,bSuoni;
extern int score[2];
extern enum TIPO_GIOCO tipoGioco;
extern BYTE batSize,ballSpeed,ballAngle,autoServe;
extern DWORD subPlayModeTime;
extern HDC hCompDC;
extern HBRUSH hBrush,hBrush2;
extern HFONT hFont2;
extern BYTE sinTable[8];
extern BYTE lastTouched;

int MobCreate(struct MOB *mp,int img1,int img2,WORD cx,WORD cy) {

	if(mp->hImg)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt)
		DeleteObject(mp->hImgAlt);
	mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	mp->s.cx=cx; mp->s.cy=cy;
	SetBitmapDimensionEx(mp->hImg,mp->s.cx,mp->s.cy,NULL);		// 
	return 1;
	}

int MobErase(HDC hDC,struct MOB *mp) {

//	mp->bVis=0;		// mah
	SelectObject(hDC,hBrush);
	return PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,PATCOPY);
	}

int MobDraw(HDC hDC,struct MOB *mp) {

	SelectObject(hCompDC,mp->hImg); //v. space invaders, non � sempre cos�...
	if(doubleSize==2)
		return StretchBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*2,mp->s.cy*2,hCompDC,0,0,mp->s.cx,mp->s.cy,SRCCOPY);
	else
		return BitBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx,mp->s.cy,hCompDC,0,0,SRCCOPY);
	}

int MobDrawXY(HDC hDC,struct MOB *mp,WORD x,WORD y) {

	SelectObject(hCompDC,mp->hImg);
	if(doubleSize==2)
		return StretchBlt(hDC,x,y,mp->s.cx*2,mp->s.cy*2,hCompDC,0,0,mp->s.cx,mp->s.cy,SRCCOPY);
	else
		return BitBlt(hDC,x,y,mp->s.cx,mp->s.cy,hCompDC,0,0,SRCCOPY);
	}

int MobMove(HDC hDC,struct MOB *mp,SIZE s) {

	MobErase(hDC,mp);
	mp->x.pos +=s.cx;
	mp->y.pos +=s.cy;
	return MobDraw(hDC,mp);
	}

int MobCollisionRect(struct MOB *mp,RECT *rc) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy;


  return ! (rc2.left > rc->right || rc2.right < rc->left
        || rc2.top > rc->bottom || rc2.bottom < rc->top);

	}

int MobCollisionPoint(struct MOB *mp,POINT pt) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy;


  return ! (rc2.left > pt.x || rc2.right < pt.x
        || rc2.top > pt.y || rc2.bottom < pt.y);

	}


int loadMobs() {
	int i,maxBats;


	//palle
	switch(tipoGioco) {
		case GIOCO_TENNIS:
		case GIOCO_SQUASH:
			maxBats=2;
creaPalla:
			MobCreate(&myMob[0],IDB_PALLA,IDB_PALLA,8*doubleSize,8*doubleSize);
			myMob[0].x.whole=AppXSizeR/2-4*doubleSize;
			myMob[0].y.whole=AppYSizeR/2;
			myMob[0].speed.x=0; myMob[0].speed.y=0;
			myMob[0].bVis=1;
			break;
		case GIOCO_HOCKEY:
			maxBats=4;
			goto creaPalla;
			break;
		case GIOCO_PELOTA:
			maxBats=1;
			goto creaPalla;
			break;
		case GIOCO_FUCILE:
		case GIOCO_FUCILE2:
			maxBats=0;
			MobCreate(&myMob[1],IDB_PALLAGRANDE,IDB_PALLAGRANDE,16*doubleSize,16*doubleSize);
			myMob[1].x.whole=AppXSizeR/2-8*doubleSize;
			myMob[1].y.whole=AppYSizeR/2;
			myMob[1].speed.x=0; myMob[1].speed.y=0;
			myMob[1].bVis=1;
			break;
		}
	//racchette: 4&5 sono primo/secondo giocatore singolo, 6&7 il doppio
	for(i=4; i<4+maxBats; i++) {			// 
		if(batSize==1)
			MobCreate(&myMob[i],IDB_RACCHETTAPICCOLA,IDB_RACCHETTAPICCOLA,8*doubleSize,20*doubleSize);
		else
			MobCreate(&myMob[i],IDB_RACCHETTA,IDB_RACCHETTA,8*doubleSize,40*doubleSize);

		if(i & 1) {
			myMob[i].x.whole = i>=6 ? AppXSizeR-((BAT2_POS+8)*doubleSize) : AppXSizeR-((BAT_POS+8)*doubleSize);
			}
		else {
			if(i==4) {
				if(tipoGioco==GIOCO_SQUASH)
					myMob[i].x.whole = AppXSizeR-((BAT2_POS+8)*doubleSize);
				else if(tipoGioco==GIOCO_TENNIS || tipoGioco==GIOCO_HOCKEY)
					myMob[i].x.whole = ((BAT_POS)*doubleSize);
				else
					myMob[i].x.whole = AppXSizeR-((BAT_POS+8)*doubleSize);
				}
			else {
				myMob[i].x.whole = (BAT2_POS)*doubleSize;
				}
			}

		myMob[i].y.whole=AppYSizeR/2;
		myMob[i].speed.x=myMob[i].speed.y=0;
		if(tipoGioco==GIOCO_PELOTA && i>4)
			myMob[i].bVis=0;
		else
			myMob[i].bVis=1;
		}


	return 1;
	}

int animateMobs(HWND hWnd) {
	struct MOB *mp;
	register int i,j;
	HDC hDC;

	hDC=GetDC(hWnd);

	switch(tipoGioco) {
		case GIOCO_TENNIS:
			for(i=4; i<6; i++) {			// 
				mp=&myMob[i];				// racchette
//				if(mp->speed.y) {
					MobErase(hDC,mp);
					if(mp->bVis) {
						if(mp->speed.y>0) {
							if(mp->y.whole > (AppYSizeR-(LOWER_BORDER+batSize*20)*doubleSize)) {
								mp->speed.y=0;
								}
							}
						else {
							if(mp->y.whole < (UPPER_BORDER*doubleSize)) {
								mp->speed.y=0;
								}
							}
						mp->y.whole +=mp->speed.y;
						MobDraw(hDC,mp);
						}
//					}
				}
			break;
		case GIOCO_HOCKEY:
			for(i=4; i<8; i++) {			// 
				mp=&myMob[i];				// racchette
//				if(mp->speed.y) {
					MobErase(hDC,mp);
					if(mp->bVis) {
						if(mp->speed.y>0) {
							if(mp->y.whole > (AppYSizeR-(LOWER_BORDER+batSize*20)*doubleSize)) {
								mp->speed.y=0;
								}
							}
						else {
							if(mp->y.whole < (UPPER_BORDER*doubleSize)) {
								mp->speed.y=0;
								}
							}
						mp->y.whole +=mp->speed.y;
						MobDraw(hDC,mp);
						}
//					}
				}
			break;
		case GIOCO_SQUASH:
			for(i=4; i<6; i++) {			// 
				mp=&myMob[i];				// racchette
//				if(mp->speed.y) {
					MobErase(hDC,mp);
					if(mp->bVis) {
						if(mp->speed.y>0) {
							if(mp->y.whole > (AppYSizeR-(LOWER_BORDER+batSize*20)*doubleSize)) {
								mp->speed.y=0;
								}
							}
						else {
							if(mp->y.whole < (UPPER_BORDER*doubleSize)) {
								mp->speed.y=0;
								}
							}
						mp->y.whole +=mp->speed.y;
						MobDraw(hDC,mp);
						}
//					}
				}
			break;
		case GIOCO_PELOTA:
			mp=&myMob[4];				// racchetta
			MobErase(hDC,mp);
//			if(mp->speed.y) {
				if(mp->bVis) {
					if(mp->speed.y>0) {
							if(mp->y.whole > (AppYSizeR-(LOWER_BORDER+batSize*20)*doubleSize)) {
							mp->speed.y=0;
							}
						}
					else {
						if(mp->y.whole < (UPPER_BORDER*doubleSize)) {
							mp->speed.y=0;
							}
						}
					mp->y.whole +=mp->speed.y;
					MobDraw(hDC,mp);
//					}
				}
			break;
		case GIOCO_FUCILE:
			break;
		case GIOCO_FUCILE2:
			break;
		}

	switch(tipoGioco) {
		case GIOCO_TENNIS:
		case GIOCO_HOCKEY:
			mp=&myMob[0];		// palla
			MobErase(hDC,mp);
			mp->x.pos += mp->speed.x;
			mp->y.pos += mp->speed.y;
			if(i=isBallInBat(&myMob[4])) {		// left
				handleBounce(&myMob[4],i);
//				mp->x.fract=0; mp->x.whole += (BALL_SIZE/2)*doubleSize;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=1;
				goto touched;
				}
			if(i=isBallInBat(&myMob[5])) {		// right
				handleBounce(&myMob[5],i);
//				mp->x.fract=0; mp->x.whole -= (BALL_SIZE/2)*doubleSize;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=2;
				goto touched;
				}
			if(tipoGioco==GIOCO_HOCKEY) {
				if(i=isBallInBat(&myMob[6])) {
					handleBounce(&myMob[6],i);
					if(bSuoni)
						PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
					lastTouched=1;
					goto touched;
					}
				if(i=isBallInBat(&myMob[7])) {
					handleBounce(&myMob[7],i);
					if(bSuoni)
						PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
					lastTouched=2;
					goto touched;
					}
				}
			if(i=hitBorder(mp)) {
				switch(i) {
					case 1:
						mp->speed.y=-mp->speed.y;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
						goto touched;
						break;
					case 2:
						mp->speed.y=-mp->speed.y;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
						goto touched;
						break;
					case 3:
						break;
					case 4:
						break;
					}
				}
			if(i=isBallOut(mp)) {
				switch(i) {
					case 1:
						break;
					case 2:
						break;
					case 3:
						mp->bVis=0;
						score[1]++;
						subPlayMode=SUBPLAY_BALLOUT;
						break;
					case 4:
						mp->bVis=0;
						score[0]++;
						subPlayMode=SUBPLAY_BALLOUT;
						break;
					}
				}

touched:
			if(mp->bVis) 
				MobDraw(hDC,mp);
			break;

		case GIOCO_SQUASH:
		case GIOCO_PELOTA:
			mp=&myMob[0];		// palla
			MobErase(hDC,mp);
			mp->x.pos +=mp->speed.x;
			mp->y.pos +=mp->speed.y;
			if(i=isBallInBat(&myMob[4])) {
				handleBounce(&myMob[4],i);
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=1;
				goto touched2;
				}
			if(tipoGioco==GIOCO_SQUASH) {
				if(i=isBallInBat(&myMob[5])) {
					handleBounce(&myMob[5],i);
					if(bSuoni)
						PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
					lastTouched=2;
					goto touched2;
					}
				}
			if(i=hitBorder(mp)) {
				switch(i) {
					case 1:
						mp->speed.y=-mp->speed.y;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
						break;
					case 2:
						mp->speed.y=-mp->speed.y;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
						break;
					case 3:
						mp->speed.x=-mp->speed.x;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));
						break;
					case 4:
						break;
					}
				}
			if(i=isBallOut(mp)) {
				switch(i) {
					case 1:
						break;
					case 2:
						break;
					case 3:
						break;
					case 4:
						score[0]++;
						score[1]++;		// gestire ultimo tocco??
						mp->bVis=0;
						subPlayMode=SUBPLAY_BALLOUT;
						goto touched2;
						break;
					}
				}
touched2:
			if(mp->bVis) 
				MobDraw(hDC,mp);
			break;

		case GIOCO_FUCILE:
		case GIOCO_FUCILE2:
			mp=&myMob[1];		// palla
			MobErase(hDC,mp);
			mp->x.pos +=mp->speed.x;
			mp->y.pos +=mp->speed.y;
			i=isBallOut(mp);

							ballAngle;

			switch(i) {
				case 1:
					mp->speed.y=-mp->speed.y;
					break;
				case 2:
					mp->speed.y=-mp->speed.y;
					break;
				case 3:
					mp->speed.x=-mp->speed.x;
					break;
				case 4:
					mp->speed.x=-mp->speed.x;
					break;
				}
			if(i)
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_BORDER));

//				if(hit)	gestire FUCILE!!
//					subPlayMode=SUBPLAY_HITBAT;

			if(mp->bVis) 
				MobDraw(hDC,mp);
			break;
		}

	ReleaseDC(hWnd,hDC);

	return 1;
  }



DWORD isBallInBat(struct MOB *mp) {
	RECT rc1,rc2;
//	int x=myMob[0].x.whole+myMob[0].s.cx/2;
//	int y=myMob[0].y.whole+myMob[0].s.cy/2;			// centro
	rc1.left=myMob[0].x.whole;
	rc1.right=rc1.left+myMob[0].s.cx;
	rc1.top=myMob[0].y.whole;
	rc1.bottom=rc1.top+myMob[0].s.cy;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy;


//  return ! (lprcSrc2->left > lprcSrc1->right || lprcSrc2->right < lprcSrc1->left
//        || lprcSrc2->top > lprcSrc1->bottom  || lprcSrc2->bottom < lprcSrc1->top);


	if(rc2.left < rc1.right && rc2.right > rc1.left &&
		rc2.top < rc1.bottom && rc2.bottom > rc1.top) {
		int y=myMob[0].y.whole+myMob[0].s.cy/2;			// centro

		if(y>=mp->y.whole && y<(mp->y.whole+mp->s.cy/8))
			return 1;
		else if(y>=(mp->y.whole+mp->s.cy/8) && y<(mp->y.whole+(mp->s.cy*2)/8))
			return 2;
		else if(y>=(mp->y.whole+(mp->s.cy*2)/8) && y<(mp->y.whole+(mp->s.cy*3)/8))
			return 3;
		else if(y>=(mp->y.whole+(mp->s.cy*3)/8) && y<(mp->y.whole+(mp->s.cy*4)/8))
			return 4;
		else if(y>=(mp->y.whole+(mp->s.cy*4)/8) && y<(mp->y.whole+(mp->s.cy*5)/8))
			return 5;
		else if(y>=(mp->y.whole+(mp->s.cy*5)/8) && y<(mp->y.whole+(mp->s.cy*6)/8))
			return 6;
		else if(y>=(mp->y.whole+(mp->s.cy*6)/8) && y<(mp->y.whole+(mp->s.cy*7)/8))
			return 7;
		else if(y>=(mp->y.whole+(mp->s.cy*7)/8) && y<(mp->y.whole+mp->s.cy))
			return 8;
		}
	return 0;
	}

BYTE isFucileInBall(struct MOB *mp) {
	int x=myMob[1].x.whole+myMob[1].s.cx/2;
	int y=myMob[1].y.whole;			// centro, alto
//	BYTE buf[22/2 /* 4bpp */ * 16],c;		// sempre dimensioni originali qua!
	DWORD buf[22*16],c;		// 32bpp
	int x2,y2;

	if(x>=(mp->x.whole) && x<(mp->x.whole+mp->s.cx) && 
		y>=(mp->y.whole) && y<(mp->y.whole+mp->s.cy)) {

		int i=GetBitmapBits(mp->hImgAlt,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
//		i=GetObject(mp->hImgAlt,sizeof(buf),buf);		// solo propriet� bitmap...
		x2=x-mp->x.whole; x2/=doubleSize;
		y2=y-mp->y.whole; y2/=doubleSize;
		c=buf[y2*mp->s.cx/doubleSize+x2];
		if(y2>0) {
			y2--;
			c|=buf[y2*mp->s.cx/doubleSize+x2];
			if(x2<(mp->s.cx/doubleSize-2))
				c|=buf[y2*mp->s.cx/doubleSize+x2+1];
			if(x2>0)
				c|=buf[y2*mp->s.cx/doubleSize+x2-1];
			}
		if(c)
			return 1;
		}
	return 0;
	}

BYTE hitBorder(struct MOB *m1) {
	int n;
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy/2;			// centro
#define BORDER_OFFSET 2 //BALL_SIZE gi� incluso

	if(tipoGioco<GIOCO_FUCILE) {
		if(y < (UPPER_BORDER+BORDER_OFFSET)*doubleSize) {
			return 1;
			}
		if(y > AppYSizeR-(LOWER_BORDER+BORDER_OFFSET)*doubleSize) {
			return 2;
			}
		if(x < (LEFT_BORDER+BORDER_OFFSET)*doubleSize) {
			return 3;
			}
		if(x > AppXSizeR-(RIGHT_BORDER+BORDER_OFFSET)*doubleSize) {
			return 4;
			}
		}
	else {
		if(y < (UPPER_BORDER+BORDER_OFFSET)*doubleSize) {
			return 1;
			}
		if(y > AppYSizeR-(LOWER_BORDER+BORDER_OFFSET)*doubleSize) {
			return 2;
			}
		if(x < (LEFT_BORDER+BORDER_OFFSET)*doubleSize) {
			return 3;
			}
		if(x > AppXSizeR-(RIGHT_BORDER+BORDER_OFFSET)*doubleSize) {
			return 4;
			}
		}
	return 0;
	}


BYTE isBallOut(struct MOB *m1) {
	int n;
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy/2;			// centro

	if(tipoGioco<GIOCO_FUCILE) {
		if(y < (UPPER_LIMIT+BALL_SIZE)*doubleSize) {
			return 1;
			}
		if(y > AppYSizeR-(LOWER_LIMIT+BALL_SIZE)*doubleSize) {
			return 2;
			}
		if(x < (LEFT_BORDER+BALL_SIZE)*doubleSize) {
			return 3;
			}
		if(x > AppXSizeR-(RIGHT_BORDER+BALL_SIZE)*doubleSize) {
			return 4;
			}
		}
	else {
		if(y < BALL_SIZE*2) {
			return 1;
			}
		if(y > AppYSizeR-(BALL_SIZE*2)*doubleSize) {
			return 2;
			}
		if(x < (LEFT_BORDER+BALL_SIZE*2)*doubleSize) {
			return 3;
			}
		if(x > AppXSizeR-(BALL_SIZE*2)*doubleSize) {
			return 4;
			}
		}
	return 0;
	}

void handleBounce(struct MOB *m1,BYTE i) {
	struct MOB *mp=&myMob[0];

	switch(i) {
		case 1:
			mp->speed.x=-mp->speed.x/2 /*3*/;
			mp->speed.y=-mp->speed.y*2;
			break;
		case 2:
			mp->speed.x=-mp->speed.x/2;
			mp->speed.y=-(mp->speed.y*3)/2;
			break;
		case 3:
			mp->speed.x=-(mp->speed.x*3)/4;
			mp->speed.y=-mp->speed.y/2;
			break;
		case 4:
		case 5:
			mp->speed.x=-mp->speed.x;
			break;
		case 6:
			mp->speed.x=-(mp->speed.x*3)/4;
			mp->speed.y=mp->speed.y/2;
			break;
		case 7:
			mp->speed.x=-mp->speed.x/2;
			mp->speed.y=(mp->speed.y*3)/2;
			break;
		case 8:
			mp->speed.x=-mp->speed.x/2 /*3*/;
			mp->speed.y=mp->speed.y*2;
			break;
		}
	if(!mp->speed.x)		// per sicurezza!
		mp->speed.x=(1+(rand() & 1))*MIN_BALL_SPEED;
	if(!mp->speed.y)
		mp->speed.y=(1+(rand() & 1))*MIN_BALL_SPEED;
	}

struct MOB *initBall(struct MOB *m) {		// in effetti non lo uso...
	int n=getBallSpeed(ballSpeed,ballAngle);

	if(tipoGioco<GIOCO_FUCILE) {
		if(tipoGioco<GIOCO_SQUASH) {
			m=&myMob[0];
			m->x.whole=AppXSizeR/2-(BALL_SIZE/2)*doubleSize;
			m->speed.x=LOBYTE(n);
			m->speed.y=HIBYTE(n);
			}
		else {
			m=&myMob[0];
			m->x.whole=BAT_POS	/* a caso! */;
			m->speed.x=LOBYTE(n);
			m->speed.y=HIBYTE(n);
			}
		}
	else {
		m=&myMob[1];
		m->x.whole=AppXSizeR/2-(BALL_SIZE)*doubleSize;
		m->speed.x=LOBYTE(n);
		m->speed.y=HIBYTE(n);
		}
	if(rand() & 1)
		m->speed.x=-m->speed.x;
	if(rand() & 1)
		m->speed.y=-m->speed.y;
	m->y.whole=AppYSizeR/2;
	m->bVis=1;
	return m;
	}

WORD getBallSpeed(BYTE s,BYTE a) {		// 
	WORD x,y;
	int n;
	WORD maxS=((s*sizeof(sinTable))/MAX_BALL_SPEED);

	n=rand() % maxS;
//	n++;
	x=sinTable[maxS-n] / 4;
	if(!x)
		x=MIN_BALL_SPEED;
	n /= 2;
	if(a==1)
		n /= 2;
	if(!n)
		n=1;
	n=rand() % n;
	y=sinTable[n] / 4;
	if(!y)
		y=MIN_BALL_SPEED;

	return MAKEWORD(x,y);
	}

