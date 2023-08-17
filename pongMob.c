#include <windows.h>
#include "pong.h"


extern int AppXSize,AppYSize,AppXSizeR,AppYSizeR;
extern struct MOB myMob[10];
extern BYTE doubleSize,bSuoni;
extern WORD score[2];
extern enum TIPO_GIOCO tipoGioco;
extern BYTE batSize,ballSpeed,ballAngle,autoServe;
extern DWORD subPlayModeTime;
extern HDC hCompDC;
extern HBRUSH hBrush,hBrush2;
extern HFONT hFont2;
extern BYTE sinTable[8];
extern BYTE lastTouched;
extern HWND hStatusWnd;

int MobCreate(struct MOB *mp,int img1,int img2,WORD cx,WORD cy,BYTE bSaveImage) {

	if(mp->hImg)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt)
		DeleteObject(mp->hImgAlt);
	mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	mp->s.cx=cx; mp->s.cy=cy;
	mp->mirrorX=mp->mirrorY=0;
	SetBitmapDimensionEx(mp->hImg,mp->s.cx,mp->s.cy,NULL);		// 
	mp->hImgOld=NULL;
	mp->magnify=1;
	mp->bSaveImage=bSaveImage;
	mp->bTransparent=0;
	return 1;
	}

int MobErase(HDC hDC,struct MOB *mp) {

//	mp->bVis=0;		// mah
	if(mp->hImgOld) {
		SelectObject(hCompDC,mp->hImgOld);
		return BitBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx/**doubleSize*/,mp->s.cy/**doubleSize*/,hCompDC,0,0,SRCCOPY);
		}
	else {
		SelectObject(hDC,hBrush);
	/*	if(doubleSize==2)
			return PatBlt(hDC,mp->x,mp->y,mp->s.cx*2,mp->s.cy*2,PATCOPY);
		else*/
		return PatBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*mp->magnify,mp->s.cy*mp->magnify,PATCOPY);
		}
	}

int MobDraw(HDC hDC,struct MOB *mp) {
	HANDLE hMyImg;

	if(mp->bSaveImage) {
		if(!mp->hImgOld)
			mp->hImgOld=CreateCompatibleBitmap(hDC,mp->s.cx*mp->magnify /**doubleSize*/,mp->s.cy*mp->magnify /**doubleSize*/);
		hMyImg=SelectObject(hCompDC,mp->hImgOld);
		BitBlt(hCompDC,0,0,mp->s.cx*mp->magnify,mp->s.cy*mp->magnify,hDC,mp->x.whole,mp->y.whole,SRCCOPY);
		SelectObject(hCompDC,hMyImg);
		}

	SelectObject(hCompDC,mp->hImg); //v. space invaders, non è sempre così...
	return StretchBlt(hDC,mp->x.whole,mp->y.whole,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobDrawXY(HDC hDC,struct MOB *mp,WORD x,WORD y) {

	if(mp->bSaveImage) {
		if(!mp->hImgOld)
			mp->hImgOld=CreateCompatibleBitmap(hDC,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify);
		SelectObject(hCompDC,mp->hImgOld);
		BitBlt(hCompDC,0,0,mp->s.cx*mp->magnify,mp->s.cy*mp->magnify,hDC,mp->x.whole,mp->y.whole,SRCCOPY);
		}

	SelectObject(hCompDC,mp->hImg);
	return StretchBlt(hDC,x,y,mp->s.cx*doubleSize*mp->magnify,mp->s.cy*doubleSize*mp->magnify,
		hCompDC,mp->mirrorX ? mp->s.cx-1 : 0,mp->mirrorY ? mp->s.cy-1 : 0,mp->mirrorX ? -mp->s.cx : mp->s.cx,mp->mirrorY ? -mp->s.cy : mp->s.cy,mp->bTransparent ? SRCPAINT : SRCCOPY);
	}

int MobMove(HDC hDC,struct MOB *mp,SIZE s) {

	MobErase(hDC,mp);
	// qua NON uso fract... (2022)
	mp->x.whole += s.cx;
	mp->y.whole += s.cy;
	return MobDraw(hDC,mp);
	}

int MobCollisionRect(struct MOB *mp,RECT *rc) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;

  return ! (rc2.left > rc->right || rc2.right < rc->left
        || rc2.top > rc->bottom || rc2.bottom < rc->top);
	}

int MobCollisionPoint(struct MOB *mp,POINT pt) {
	RECT rc2;
	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;

  return ! (rc2.left > pt.x || rc2.right < pt.x
        || rc2.top > pt.y || rc2.bottom < pt.y);
	}

BYTE MobCollisionColor(struct MOB *mp,DWORD color,BYTE mode) {
	RECT rc2;
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int x,y;
	int i;

	rc2.left=mp->x.whole;
	rc2.right=rc2.left+mp->s.cx*mp->magnify;
	rc2.top=mp->y.whole;
	rc2.bottom=rc2.top+mp->s.cy*mp->magnify;
	//serve??

	if(!mp->bSaveImage)
		return 0;

	i=GetBitmapBits(mp->hImgOld,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
	if(mode) {			// cerco il colore dato
//		i=GetObject(mp->hImgOld,sizeof(buf),buf);		// solo proprietà bitmap...
		for(y=0; y<mp->y.whole; y++) {
			for(x=0; x<mp->x.whole; x++) {
				c=*p++;
				if(c == color)
					return 1;
				}
			}
		}
	else {			// cerco qualsiasi colore diverso da quello dato
		for(y=0; y<mp->y.whole; y++) {
			for(x=0; x<mp->x.whole; x++) {
				c=*p++;
				if(c != color)
					return 1;
				}
			}
		}
	return 0;
	}

int MobSetColor(struct MOB *mp,BYTE w,COLORREF forecolor,COLORREF backcolor) {
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int x,y;
	int i;

	i=GetBitmapBits(w ? mp->hImg : mp->hImgAlt,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
//		i=GetObject(mp->hImgOld,sizeof(buf),buf);		// solo proprietà bitmap...
	for(y=0; y<mp->s.cy; y++) {
		for(x=0; x<mp->s.cx; x++) {
			c=*p;
			if(c)		// tutto quel che non è nero! (finire...?)
			//	mp->bTransparent;//e usare ??
				*p=RGB(GetBValue(forecolor),GetGValue(forecolor),GetRValue(forecolor));
			p++;
			}
		}

	return SetBitmapBits(w ? mp->hImg : mp->hImgAlt,sizeof(buf),buf);		// 
	}

COLORREF MobGetColor(struct MOB *mp,DWORD where) {
	DWORD buf[128*64 /*FARE DINAMICO! mp->s.cx*mp->s.cy*/],c,*p;		// 32bpp
	int i,n;

	i=GetBitmapBits(mp->hImg,sizeof(buf),buf);		// questa restituisce 32bpp, vecchia storia...
	p=&buf;
	if(where != 0xffffffff) {
		POINT pt={ LOWORD(where),HIWORD(where) };
		n=pt.y*mp->s.cx+pt.x;
		if(n<sizeof(buf)/sizeof(DWORD))
			p += n;
		}
	else {
		n=sizeof(buf)/sizeof(DWORD);
		while(*p == RGB(0,0,0) && n>=0) {
			p++; n--;
			}
		}

	return RGB(GetBValue(*p),GetGValue(*p),GetRValue(*p));
	}

int MobSetImage(struct MOB *mp,int img1,int img2) {

	if(mp->hImg && img1)
		DeleteObject(mp->hImg);
	if(mp->hImgAlt && img2)
		DeleteObject(mp->hImgAlt);
	if(img1)
		mp->hImg=LoadBitmap(g_hinst,MAKEINTRESOURCE(img1));
	if(img2)
		mp->hImgAlt=LoadBitmap(g_hinst,MAKEINTRESOURCE(img2));
	return 1;
	}



int loadMobs(BYTE q) {
	int i,maxBats;


	if(q & 1) {
		//palle
		switch(tipoGioco) {
			case GIOCO_TENNIS:
			case GIOCO_SQUASH:
				maxBats=2;
	creaPalla:
				MobCreate(&myMob[0],IDB_PALLA,IDB_PALLA,BALL_SIZE*doubleSize,BALL_SIZE*doubleSize,FALSE);
				myMob[0].x.whole=AppXSizeR/2-(BALL_SIZE/2)*doubleSize;
				myMob[0].y.whole=AppYSizeR/2-myMob[0].s.cy/2;
				myMob[0].speed.x=0; myMob[0].speed.y=0;
				myMob[0].bVis=1;
				myMob[0].bSaveImage=1;
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
				MobCreate(&myMob[1],IDB_PALLAGRANDE,IDB_PALLAGRANDE,BALL_SIZE*2*doubleSize,BALL_SIZE*2*doubleSize,FALSE);
				myMob[1].x.whole=AppXSizeR/2-BALL_SIZE*doubleSize;
				myMob[1].y.whole=AppYSizeR/2-myMob[1].s.cy/2;
				myMob[1].speed.x=0; myMob[1].speed.y=0;
				myMob[1].bVis=1;
				myMob[1].bSaveImage=1;
				break;
			}
		}
	if(q & 2) {
		//racchette: 4&5 sono primo/secondo giocatore singolo, 6&7 il doppio
		for(i=4; i<4+maxBats; i++) {			// 
			if(batSize==1)
				MobCreate(&myMob[i],IDB_RACCHETTAPICCOLA,IDB_RACCHETTAPICCOLA,BAT_SIZE_X*doubleSize,(BAT_SIZE_Y/2)*doubleSize,FALSE);
			else
				MobCreate(&myMob[i],IDB_RACCHETTA,IDB_RACCHETTA,BAT_SIZE_X*doubleSize,BAT_SIZE_Y*doubleSize,FALSE);

			if(i & 1) {
				myMob[i].x.whole = i>=6 ? AppXSizeR-((BAT2_POS+BAT_SIZE_X)*doubleSize) : AppXSizeR-((BAT_POS+BAT_SIZE_X)*doubleSize);
				}
			else {
				if(i==4) {
					if(tipoGioco==GIOCO_SQUASH)
						myMob[i].x.whole = AppXSizeR-((BAT2_POS+BAT_SIZE_X)*doubleSize);
					else if(tipoGioco==GIOCO_TENNIS || tipoGioco==GIOCO_HOCKEY)
						myMob[i].x.whole = ((BAT_POS)*doubleSize);
					else
						myMob[i].x.whole = AppXSizeR-((BAT_POS+BAT_SIZE_X)*doubleSize);
					}
				else {
					myMob[i].x.whole = (BAT2_POS)*doubleSize;
					}
				}

			myMob[i].y.whole=AppYSizeR/2-myMob[i].s.cy/2;
			myMob[i].speed.x=myMob[i].speed.y=0;
			if(tipoGioco==GIOCO_PELOTA && i>4)
				myMob[i].bVis=0;
			else
				myMob[i].bVis=1;
			myMob[i].bSaveImage=0;
			}
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
			// metto controllo direzione per migliorare collisione...(e evito anche contro-rimbalzo hockey, VERIFICARE!)
			if(mp->speed.x<0 &&    (i=isBallInBat(&myMob[4]))) {		// left
				handleBounce(&myMob[4],i);
//				mp->x.fract=0; mp->x.whole += (BALL_SIZE/2)*doubleSize;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=1;
				goto touched;
				}
			if(mp->speed.x>0 &&    (i=isBallInBat(&myMob[5]))) {		// right
				handleBounce(&myMob[5],i);
//				mp->x.fract=0; mp->x.whole -= (BALL_SIZE/2)*doubleSize;
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=2;
				goto touched;
				}
			if(tipoGioco==GIOCO_HOCKEY) {
				if(mp->speed.x<0 &&    (i=isBallInBat(&myMob[6]))) {
					handleBounce(&myMob[6],i);
					if(bSuoni)
						PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
					lastTouched=1;
					goto touched;
					}
				if(mp->speed.x>0 &&    (i=isBallInBat(&myMob[7]))) {
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
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_OUT));
						break;
					case 4:
						mp->bVis=0;
						score[0]++;
						subPlayMode=SUBPLAY_BALLOUT;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_OUT));
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
			if(mp->speed.x>0 &&    (i=isBallInBat(&myMob[4]))) {
				handleBounce(&myMob[4],i);
				if(bSuoni)
					PlayResource(MAKEINTRESOURCE(IDR_WAVE_PADDLE));
				lastTouched=1;
				goto touched2;
				}
			if(tipoGioco==GIOCO_SQUASH) {
				if(mp->speed.x>0 &&    (i=isBallInBat(&myMob[5]))) {
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
						if(tipoGioco==GIOCO_SQUASH)
							score[1]++;		// opp gestire ultimo tocco??
						mp->bVis=0;
						subPlayMode=SUBPLAY_BALLOUT;
						if(bSuoni)
							PlayResource(MAKEINTRESOURCE(IDR_WAVE_OUT));
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


	if(rc2.left <= rc1.right && rc2.right >= rc1.left &&
		rc2.top <= rc1.bottom && rc2.bottom >= rc1.top) {
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
		else if(y>=(mp->y.whole+(mp->s.cy*7)/8) && y<=(mp->y.whole+mp->s.cy))
			return 8;
		}
	return 0;
	}

BYTE hitBorder(struct MOB *m1) {
	int n;
	int x=m1->x.whole+m1->s.cx/2;
	int y=m1->y.whole+m1->s.cy/2;			// centro
#define BORDER_OFFSET 0 //BALL_SIZE già incluso

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
#define OUT_OFFSET 0 //(BALL_SIZE/2) //

	if(tipoGioco<GIOCO_FUCILE) {
		if(y < (UPPER_LIMIT+OUT_OFFSET)*doubleSize) {
			return 1;
			}
		if(y > AppYSizeR-(LOWER_LIMIT+OUT_OFFSET)*doubleSize) {
			return 2;
			}
		if(x < (LEFT_BORDER+OUT_OFFSET)*doubleSize) {
			return 3;
			}
		if(x > AppXSizeR-(RIGHT_BORDER+OUT_OFFSET)*doubleSize) {
			return 4;
			}
		}
	else {
		if(y < (BALL_SIZE*2)*doubleSize) {
			return 1;
			}
		if(y > AppYSizeR-(BALL_SIZE*2)*doubleSize) {
			return 2;
			}
		if(x < (BALL_SIZE*2)*doubleSize) {
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
	int v=abs(mp->speed.x)+abs(mp->speed.y);		// vettore complessivo velocità precedente
	signed char dx=sgn(mp->speed.x),dy=sgn(mp->speed.y);


	// si potrebbe inserire un minimo di casualità...?
	switch(i) {
		case 1:		// alto; svirgolo
			mp->speed.x=-mp->speed.x/2 /*3*/;
			if(!mp->speed.x)
				mp->speed.x=-dx*MIN_BALL_SPEED;
			mp->speed.y=-(v-abs(mp->speed.x));
			if(!mp->speed.y)
				mp->speed.y=-MIN_BALL_SPEED;
			break;
		case 2:		// svirgolo un po' meno
			mp->speed.x=-(mp->speed.x*3)/4;
			if(!mp->speed.x)
				mp->speed.x=-dx*MIN_BALL_SPEED;
			mp->speed.y=-(v-abs(mp->speed.x));
			if(!mp->speed.y)
				mp->speed.y=-MIN_BALL_SPEED;
			break;
		case 3:		// appiattisco, e aggiungo un po' casuale per compensare arrotondamenti consecutivi..
			mp->speed.y=-(mp->speed.y)/2;
			if(!mp->speed.y)
				mp->speed.y=-dy*MIN_BALL_SPEED;
			mp->speed.x=-dx*(v  + 2*(rand() & 1)  -abs(mp->speed.y));
			break;
		case 4:		// centro; rimbalzo puro
		case 5:
			mp->speed.x=-mp->speed.x;
			mp->speed.x -= dx*(rand() & 1);		// idem
			break;
		case 6:
			mp->speed.y=-(mp->speed.y)/2;
			if(!mp->speed.y)
				mp->speed.y=-dy*MIN_BALL_SPEED;
			mp->speed.x=dx*(v  + 2*(rand() & 1)  -abs(mp->speed.y));
			break;
		case 7:
			mp->speed.x=-(mp->speed.x*3)/4;
			if(!mp->speed.x)
				mp->speed.x=-dx*MIN_BALL_SPEED;
			mp->speed.y=v-abs(mp->speed.x);
			if(!mp->speed.y)
				mp->speed.y=MIN_BALL_SPEED;
			break;
		case 8:		// basso
			mp->speed.x=-mp->speed.x/2 /*3*/;
			if(!mp->speed.x)
				mp->speed.x=-dx*MIN_BALL_SPEED;
			mp->speed.y=v-abs(mp->speed.x);
			if(!mp->speed.y)
				mp->speed.y=MIN_BALL_SPEED;
			break;
		}

	mp->speed.x=min(mp->speed.x,128/4 /* v. sintable */);
	normalizeBallAngle(mp,ballAngle);
/*	dx=sgn(mp->speed.x); dy=sgn(mp->speed.y);
	if(ballAngle==2) {
		while(abs(mp->speed.y)>=abs(mp->speed.x)) {
			mp->speed.y-=dy;
			mp->speed.x+=dx;
			}
		}
	else {
		while(abs(mp->speed.y)>=abs(mp->speed.x)/2) {
			mp->speed.y-=dy;
			mp->speed.x+=dx;
			}
		}*/

	dx=sgn(mp->speed.x);
	mp->x.whole+=(dx*BALL_SIZE*3)/2;			// evito rinculi

/*	if(!mp->speed.x)		// per sicurezza!
		mp->speed.x=(1+(rand() & 1))*MIN_BALL_SPEED;
	if(!mp->speed.y)
		mp->speed.y=(1+(rand() & 1))*MIN_BALL_SPEED;*/
#ifdef _DEBUG
	{
	char myBuf[64];
	wsprintf(myBuf,"xs=%d,ys=%d",mp->speed.x,mp->speed.y);
	SetWindowText(hStatusWnd,myBuf);
	}
#endif
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
			m->x.whole=BAT_POS*3	/* a caso! */;
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
	m->y.whole=AppYSizeR/2-m->s.cy/2;
	m->bVis=1;
	return m;
	}

WORD getBallSpeed(BYTE s,BYTE ba) {		// 
	WORD x,y;
	int n;
	WORD maxS=((s*sizeof(sinTable))/MAX_BALL_SPEED);

	n=rand() % maxS;
//	n++;
	x=sinTable[maxS-n] / 4;
	if(!x)
		x=MIN_BALL_SPEED;
	n /= 2;
	if(ba==1)
		n /= 2;
	if(!n)
		n=1;
	n=rand() % n;
	y=sinTable[n] / 4;
	if(!y)
		y=MIN_BALL_SPEED;

	//v. anche bounce e normalizzazione ballAngle... usare, nel caso
#ifdef _DEBUG
	{
	char myBuf[64];
	wsprintf(myBuf,"xs=%d,ys=%d",x,y);
	SetWindowText(hStatusWnd,myBuf);
	}
#endif

	return MAKEWORD(x,y);
	}

void normalizeBallAngle(struct MOB *mp,BYTE ba) {
	signed char dx,dy;

	dx=sgn(mp->speed.x); dy=sgn(mp->speed.y);
	if(ba==2) {
		while(abs(mp->speed.y)>=abs(mp->speed.x)) {
			mp->speed.y-=dy;
			mp->speed.x+=dx;
			}
		}
	else {
		while(abs(mp->speed.y)>=abs(mp->speed.x)/2) {
			mp->speed.y-=dy;
			mp->speed.x+=dx;
			}
		}
	}

