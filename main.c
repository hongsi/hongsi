/**************************************************************/
/*              홍익대학교 시간표작성 도우미                    */
/*                                                            */
/*                          HONGSI                            */
/*                                                            */
/*                                         github.com/hongsi  */
/**************************************************************/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "resource.h"

int unit, unit_list[32]; //unit은 학점.
int cnt = 0, next_chk = 0;	// cnt는 Date_list[cnt] 의 카운터로 쓰이다가 read()끝나면 총과목수로 쓰임 
char table[16][8];	// 시간표(2차원 배열)
char name[64] = {0,}, old_name[64] = {0,}, time[32];
char name_list[32][64];	// 32과목, 과목명 32자까지 가능
char prof_name[32] = {0,}, prof_list[32][32];
char croom_name[32] = {0,}, croom_list[32][32];
char state_str[512];

typedef struct Data *list_pointer;
struct Data
{
	char time[32];			// 월23,수5 같은 정보가 저장
	int	 time_list[32];		// 월23,수5가 변환된 203052 같은 정보가 저장
	list_pointer next;
};

list_pointer Data_list[32],curr,ptr; // curr은 현재 노드, ptr은 새로 할당된 노드

void start();
int read(FILE *);	// 파일에서 읽어들임
int analyse();		// 읽어들인 과목명과 강의시간 정보를 연속적으로 저장
int what_day(char*);	// 한글 문자를 읽어 무슨 요일인지 판단
void state(int num_table);		// 읽어들인 과목들에 대한 정보 출력
int make_table(FILE *);	// 가능한 수업의 조합을 계산하는 함수
int insert_table(list_pointer, int);	// 시간표 배열에 수업을 삽입
void restore(list_pointer);	// 지정한 수업을 시간표 배열에서 삭제
void init_table();	// 시간표 배열 초기화
void print_table_head(FILE *);
int ishangul2nd(byte *, int);  // 한글 두번째 바이트인지 체크 (한라프로, http://kldp.org/node/75688)
void print_table(FILE *,int);	// 시간표 출력
void free();		// TODO: 메모리 반환

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPSTR lpszClass = "홍시 3.2 - 홍익대 시간표 작성프로그램";
HWND hWnd;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevIntance, LPSTR lpCmdLine, int nCmdShow)
{	
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	WndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	WndClass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_POPUPWINDOW | WS_MINIMIZEBOX, 
		310, 200, 400, 300, NULL, (HMENU)NULL, hInstance,NULL);
	ShowWindow(hWnd,nCmdShow);

	while (GetMessage(&Message,0,0,0)){
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

BOOL CALLBACK HelpDlgProc(HWND hDlg,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	HDC hdc,MemDC;
	PAINTSTRUCT ps;
	HBITMAP MyBitmap, OldBitmap;

	switch(iMessage)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_PAINT:
		hdc = BeginPaint(hDlg,&ps);
		MemDC = CreateCompatibleDC(hdc);
		MyBitmap = LoadBitmap(g_hInst,MAKEINTRESOURCE(IDB_BITMAP5));
		OldBitmap = (HBITMAP)SelectObject(MemDC, MyBitmap);
		BitBlt(hdc,0,0,400,300,MemDC,0,0,SRCCOPY);
		
		SelectObject(MemDC,OldBitmap);
		DeleteObject(MyBitmap);
		DeleteDC(MemDC);
		EndPaint(hWnd,&ps);
		return 0;
	case WM_COMMAND:
		switch(wParam)
		{		
		case IDCANCEL:		
			EndDialog(hDlg,0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{	
	HDC hdc,MemDC;
	PAINTSTRUCT ps;
	HBITMAP MyBitmap, OldBitmap;
	switch(iMessage){	
	case WM_CREATE:
		CreateWindow("button","?",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			10,10,15,15,hWnd,(HMENU)0,g_hInst,NULL);
		CreateWindow("button","x",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			370,10,20,20,hWnd,(HMENU)2,g_hInst,NULL);
		CreateWindow("button","Start !",WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			190,142,50,25,hWnd,(HMENU)1,g_hInst,NULL);	
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd,&ps);
		MemDC = CreateCompatibleDC(hdc);
		MyBitmap = LoadBitmap(g_hInst,MAKEINTRESOURCE(IDB_BITMAP3));
		OldBitmap = (HBITMAP)SelectObject(MemDC, MyBitmap);
		BitBlt(hdc,0,0,400,300,MemDC,0,0,SRCCOPY);
		
		SelectObject(MemDC,OldBitmap);
		DeleteObject(MyBitmap);
		DeleteDC(MemDC);
		EndPaint(hWnd,&ps);
		return 0;
	case WM_LBUTTONDOWN:
		DefWindowProc(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam); 
		return 0;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 0:
			DialogBox(g_hInst,MAKEINTRESOURCE(IDD_DIALOG1),hWnd,HelpDlgProc);
			break;
		case 1:
			start();			
			break;
		case 2:
			exit(0);
		}
		return 0;
	}
	return(DefWindowProc(hWnd,iMessage,wParam,lParam));
}

void start()
{	
	int i,j;
	char buffer[256] = {0,};

	FILE *fp_r,*fp_w;
	
	unit = 0;
	for (i = 0;i<32;i++)
		unit_list[i] = 0;

	for (i = 0; i<cnt; i++)
		free(Data_list[i]);

	cnt = 0, next_chk = 0; 

	for (i = 0;i<16;i++)
		for (j = 0;j<8;j++)
			table[i][j] = '\0';

	for (i = 0;i<64;i++)
	{
		name[i] = '\0';
		old_name[i] = '\0';
	}

	for (i = 0;i<32;i++)
		time[i] = '\0';

	for (i = 0;i<32;i++)
		for (j = 0;j<64;j++)
			name_list[i][j] = '\0';
		
	for (i = 0;i<512;i++)
		state_str[i] = '\0';
	
	fp_r = fopen("class.txt","r");
	
	if (fp_r == NULL)
	{
		MessageBox(hWnd,"class.txt 파일에 원하는 과목들을 붙여 넣으세요.","알림",MB_OK);

		fp_w = fopen("class.txt","w");
		fputs("이 파일에 클래스넷의 과목들을 긁어붙이세요.\n",fp_w);
		fclose(fp_w);

		return -1;
	}

	if (read(fp_r))
	{
		fclose(fp_r);
		return;
	}
	fclose(fp_r);	

	fp_w = fopen("시간표.html","w");	

	if (make_table(fp_w))
	{
		fclose(fp_w);
		return;
	}
	fclose(fp_w);

	GetCurrentDirectory(256, buffer);
	sprintf(buffer, "%s\\%s", buffer, "시간표.html");
	ShellExecute(NULL, "open", "iexplore.exe", buffer, NULL, SW_SHOWNORMAL);
}

int read(FILE *fp_r)
{
    int i, strcnt;
	char c, str[64];
	BOOL class_time_found = FALSE;
	
	for (i = 1;c = getc(fp_r);i++)
	{
		if (i == 1)	
			time[0] = '\0';
		
		while (c == ' ' || c == '\t')	
			c = getc(fp_r);	// 공백이 연속인 경우
		
		if (i == 1 && c == EOF && cnt == 0)
		{
			MessageBox(hWnd,"class.txt 가 비어있습니다.","알림",MB_OK);
			return -1;
		}
		
		strcnt = 0;
		while (c != ' ' && c != '\t' && c != '\n' && c != EOF)
		{
			str[strcnt++] = c;
			c = getc(fp_r);		
		}
		str[strcnt] = '\0';	// 문자열 끝에 널문자 삽입		

		if (i == 1 && (str[0] == '#' || str[0] == '`' || str[0] == '('))	// 라인 첫 문자가 '#', '`', '(' 이면 라인 무시. '(' 는 사이버강좌
		{
			while (c != '\n' && c != EOF)	
				c = getc(fp_r);
			
			i = 0;
			continue;
		}
		else if (i == 1 && str[0]<0)	// 학년이 생략된 경우
		{
			i = 2;
		}
		else if (i == 4 && str[0]<0)	// 교양영역 항목을 만나면 카운트 하지 않음 (공통교양)
		{
			i--;
		}
		else if (i == 5)	// 과목명을 name에 저장
		{
			if (strlen(str) >= 3 && !strcmp(&str[strlen(str)-3],"(*)"))	// 과목명 끝에 붙는 '(*)' 제거. (영어진행 수업)
			{
				str[strlen(str)-3] = '\0';
			}

			strcpy(name,str);
		}
		else if (i == 6)	// 학점을 unit에 저장
		{
			if (str[0] < 48 || 57 < str[0])	// 숫자가 아니라면 과목명에 띄어쓰기 있는 경우임 (예: ADVANCED CAD, 중국어로 배우는)
			{
				strcat(name," ");
				strcat(name,str);
				i--;
			}

			unit = atoi(str);
		}
		else if (i == 11)
		{
			if (what_day(str) >= 0)	// 교수명이 비어서 요일시간이 나오는 경우
			{
				strcpy(prof_name," ");

				strcpy(time,str);
				class_time_found = TRUE;

				i++;
			}
			else
				strcpy(prof_name,str);
		}
		else if (i == 12 && str[0]>0)	// 교수명에 공백있는 문제(영어 이름) 해결
		{
			strcat(prof_name," ");
			strcat(prof_name,str);
			i--;		
		}
		else if (i == 12 && what_day(str) >= 0)	// 요일및시간을 time에 저장
		{
			strcpy(time,str);
			class_time_found = TRUE;
		}
		else if (i == 13)
		{
			strcpy(croom_name,str);
		}
	
		if (c == '\n')
		{					
			if (class_time_found && analyse())	// 읽은 라인을 analyse()에서 강의데이터로 변형
			{
				MessageBox(hWnd,"class.txt 에 수강정보가 비현실적으로 많습니다. (32과목 이상)","알림",MB_OK);
				return -1;
			}

			i = 0;
			class_time_found = FALSE;
		}
		else if (c == EOF)	// EOF 를 만나면 끝냄
		{
			if (class_time_found && analyse())	// 읽은 라인을 analyse()에서 강의데이터로 변형
			{
				MessageBox(hWnd,"class.txt 에 수강정보가 비현실적으로 많습니다. (32과목 이상)","알림",MB_OK);
				return -1;
			}

			if (cnt == 0)	// 아무 소득없이 끝날 경우
			{
				MessageBox(hWnd,"class.txt 에 올바른 수강신청 정보가 없습니다.","알림",MB_OK);
				return -1;
			}
			else			// 정상 종료.
				return 0;
		}
		
	}
	return 0;
}

int analyse()
{
	int i = 0, j = 0, k, temp;
	int old_cnt = 0, exist = 0;
	
	ptr = (list_pointer) malloc (sizeof(struct Data));
	ptr->next = NULL;
	strcpy(ptr->time,time);			// 월23,수7 같은 정보를 ptr->time에 저장
		
	for (k = 0;k<cnt;k++)	// 이미 존재하는 과목인지 name_list에서 검색
	{		
		if (!strcmp(name_list[k],name))
		{
			exist = 1;
			old_cnt = k;
			break;
		}
	}

	if (!exist)	// 새로운 과목 발견이면
	{		
		strcpy(name_list[cnt],name);	// 과목명 리스트에 추가
		strcpy(prof_list[cnt],prof_name);	// 교수님 성함
		strcpy(croom_list[cnt],croom_name);	// 강의실
		unit_list[cnt] = unit;	// 학점 리스트에 추가
		Data_list[cnt++] = curr = ptr;		// Data_list의 새로운 방과 노드연결

		if (cnt >= 32)
		{
			return -1;	// 32과목 이상은 무시
		}
	}
	else		// 이미 존재하는 과목이면
	{		
		curr = Data_list[old_cnt];
		while (1)				// 노드 끝까지 가서
		{
			if (!strcmp(curr->time,time))	// 같은 과목, 같은 시간이면 저장취소
			{
				free(ptr);
				return 0;			
			}
			if (curr->next == NULL)	// 노드 끝을 만나면 loop 중지
				break;	
			else					// 아니면 다음 노드로 이동
				curr = curr->next;
		}
		curr = curr->next = ptr;	// 노드 끝에 새 정보 연결
	}

	while (time[i])	// 월23,수7 같은 정보로 먼저 020327 과 같은 data 형성
	{
		if (time[i]<0)	// time[0]이 한글이면
		{
			curr->time_list[j++] = what_day(&time[i]);
			i += 2;
		}
		else
		{
			if (j%2 == 0)
				curr->time_list[j++] = curr->time_list[j-2];

			curr->time_list[j++] = (int)time[i++]-48;
			if (curr->time_list[j-1] == 1 && (int)time[i] == 48)	 // 10교시
			{
				curr->time_list[j-1] = 10;
				i++;
			}
			else if (curr->time_list[j-1] == 1 && (int)time[i] == 49)	 // 11교시
			{
				curr->time_list[j-1] = 11;
				i++;
			}
			else if (curr->time_list[j-1] == 1 && (int)time[i] == 50 && ((int)time[i-2] >= 48 || (int)time[i+1] == 49))	 // 12교시
			{
				curr->time_list[j-1] = 12;
				i++;
			}
			else if (curr->time_list[j-1] == 1 && (int)time[i] == 51)	 // 13교시
			{
				curr->time_list[j-1] = 13;
				i++;
			}
			/*else if (curr->time_list[j-3] + 1 == curr->time_list[j-1] * 10 + ((int)time[i] - 48))  // 12, 13, .. 교시
			{
				curr->time_list[j-1] = curr->time_list[j-1] * 10 + ((int)time[i] - 48);
				i++;
			}*/
		}		
		if (time[i] == ',')	i++;		
	}
	curr->time_list[j] = -1;	// 문자열 time_list에 끝표시로 -1 덧붙임
	
	for (i = 0;curr->time_list[i] != -1;i += 2)	// 월23,수7 에서 020327 로 바뀐 정보를 203072 로 변환
	{
		temp = curr->time_list[i];
		curr->time_list[i] = curr->time_list[i+1];
		curr->time_list[i+1] = temp;		
	}
	strcpy(old_name,name);

	return 0;
}

int what_day(char* str)	// 문자열의 첫글자가 무슨요일인지 판별
{
	if (!strncmp(str,"월",2))	return 0;
	else if (!strncmp(str,"화",2))	 return 1;
	else if (!strncmp(str,"수",2))	 return 2;
	else if (!strncmp(str,"목",2))	 return 3;
	else if (!strncmp(str,"금",2))	 return 4;
	else if (!strncmp(str,"토",2))	 return 5;
	else return -1;
}

void state(int num_table)
{	
	int i, j, k, tab_length;
	char str[64];
		
	for (i = 0,j = 0;i<cnt;i++)
		j += unit_list[i];

	sprintf(state_str,"\n[시간표 %d개] 총 %d과목 %d학점\n",num_table,cnt,j);

	for (i = 0,j = 0;i<cnt;i++)	// 과목 이름과 학점
	{
		strcat(state_str,"\n");

		for (j = 0;name_list[i][j];j++)
		{
			sprintf(str,"%c",name_list[i][j]);
			strcat(state_str,str);
		}

		tab_length = 3 - (strlen(name_list[i])+1) / 10;
		for (k = 0;k < tab_length;k++)
		{
			strcat(state_str,"\t");
		}

		sprintf(str," (%d학점)   ",unit_list[i]);
		strcat(state_str,str);
	}
}

int make_table(FILE *fp_w)
{	
	int i, num_table = 1;	
	
	list_pointer lec_ptr[32];	// 각 과목들을 가리키는 포인터
	
	for (i = 0;i<cnt;i++)
	{
		lec_ptr[i] = Data_list[i];
	}

	print_table_head(fp_w);

	while (1)
	{
		init_table();

		for (i = 0;i<cnt;i++)
		{			
			if (insert_table(lec_ptr[i],i+1))	// 1이 리턴되면 충돌이 있다는 뜻
			{
				//printf("%d번 과목 충돌\n",i+1);
				lec_ptr[i] = lec_ptr[i]->next;
				
				while (lec_ptr[i] == NULL)
				{
					lec_ptr[i] = Data_list[i];	// 충돌이 있는 과목은 다시 첫노드를 가리키게 하고					
					if (--i<0)
					{
						if (--num_table)
						{
							state(num_table);
							MessageBox(hWnd,state_str,"알림",MB_OK);
							return 0;
						}
						else
						{
							//state();
							MessageBox(hWnd,"모든 조건을 만족하는 시간표가 없습니다.","알림",MB_OK);
							return -1;
						}
					}
					restore(lec_ptr[i]);	 // 앞과목 시간표 배정을 초기화하고
					lec_ptr[i] = lec_ptr[i]->next;	 // 앞과목 노드 오른쪽이동 후 앞과목부터 다시 시작
				}				
				i--;
			}
			//print_table();	// 연산 과정 출력
		}	// 참고 : for문을 벗어난 현재 i는 cnt와 같음
		
		//printf("\t\t\t\t\t완성 - %2d번 시간표\n",num_table++);
		print_table(fp_w,num_table++);

		i--;
		lec_ptr[i] = lec_ptr[i]->next;	// 시간표하나 완성하면 마지막번과목을 다음노드로 이동하고 계속
		while (lec_ptr[i] == NULL)
		{
			lec_ptr[i] = Data_list[i];	// 충돌이 있는 과목은 다시 첫노드를 가리키게 하고					
			if (--i<0)
			{
				if (--num_table)
				{
					state(num_table);					
					MessageBox(hWnd,state_str,"알림",MB_OK);
					return 0;
				}
				else
				{
					//state();				
					MessageBox(hWnd,"모든 조건을 만족하는 시간표가 없습니다.","알림",MB_OK);
					return -1;
				}
			}			
			lec_ptr[i] = lec_ptr[i]->next;	 // 앞과목 노드 오른쪽이동 후 앞과목부터 다시 시작
		}
		
	}

}

int insert_table(list_pointer now, int lec)
{
	int i;	

	for (i = 0;now->time_list[i] != -1;i += 2)	// 충돌이 하나라도 있는지 확인
	{
		if ((table[now->time_list[i]][now->time_list[i+1]]))
			return 1;		
	}
	
	for (i = 0;now->time_list[i] != -1;i += 2)	// 충돌이 하나도 없으면 입력
	{		
		table[now->time_list[i]][now->time_list[i+1]] = lec;		
	}
	return 0;
}

void restore(list_pointer now)
{
	int i = 0;	

	while (now->time_list[i] != -1)
	{
		table[now->time_list[i]][now->time_list[i+1]] = 0;
		
		i += 2;
	}
}

void init_table()
{
	int i,j;

	for (i = 1;i < 16;i++)				
		for (j = 0;j < 8;j++)
			table[i][j] = 0;
}

void print_table_head(FILE *fp_w)	// IE security warning 제거 (http://msdn.microsoft.com/en-us/library/ms537628(v=vs.85).ASPX)
{
	fprintf(fp_w,"<HTML>\n<HEAD><TITLE> 시간표 </TITLE></HEAD>\n\n<style type = \"text/css\">\n\nBODY,TD,DIV,LI	{ FONT-FAMILY : APPLEGOTHIC,굴림,SANS-SERIF; FONT-SIZE : 9PT; COLOR : #333; TEXT-ALIGN : CENTER; LINE-HEIGHT : 150%%;}\nTABLE { border-collapse: collapse; }\nTD { WIDTH : 70; HEIGHT : 20; border: 1px solid #E8E8E8; }\nTD.DAY { BACKGROUND-COLOR : #CCFFFF; }\nTD.CLASSNUM { BACKGROUND-COLOR : #FFF4FF; }\nTD.CLASS01 { BACKGROUND-COLOR : #C3D2F1; }\nTD.CLASS02 { BACKGROUND-COLOR : #FFEDBB; }\nTD.CLASS03 { BACKGROUND-COLOR : #DEE6D0; }\nTD.CLASS04 { BACKGROUND-COLOR : #F6CCCD; }\nTD.CLASS05 { BACKGROUND-COLOR : #D9CDEE; }\nTD.CLASS06 { BACKGROUND-COLOR : #CADED8; }\nTD.CLASS07 { BACKGROUND-COLOR : #FAD9BE; }\nTD.CLASS08 { BACKGROUND-COLOR : #E9D0DC; }\nTD.CLASS09 { BACKGROUND-COLOR : #FBFCBD; }\nTD.CLASS10 { BACKGROUND-COLOR : #F3DCF2; }\nTD.CLASS11 { BACKGROUND-COLOR : #D9EFB9; }\nTD.CLASS12 { BACKGROUND-COLOR : #F3EAD1; }\n\n#hintbox{ position:absolute; top: 0; background-color: lightyellow; width: 150px; \npadding: 3px; border:1px solid black; font:normal 11px Verdana; line-height:18px;\nz-index:100; visibility: hidden; }\n\n</style>\n\n");
	fprintf(fp_w,"<!-- saved from url=(0014)about:internet -->\n\n<script type = \"text/javascript\">\n\nvar horizontal_offset = \"5px\"\nvar vertical_offset = \"0\"\nvar ie = document.all\nvar ns6 = document.getElementById&&!document.all\nvar tipwidth = 130\n\nfunction getposOffset(what, offsettype){\nvar totaloffset = (offsettype == \"left\")? what.offsetLeft : what.offsetTop;\nvar parentEl = what.offsetParent;\nwhile (parentEl != null){\ntotaloffset = (offsettype == \"left\")? totaloffset+parentEl.offsetLeft : totaloffset+parentEl.offsetTop;\nparentEl = parentEl.offsetParent;\n}\nreturn totaloffset;\n}\n\nfunction iecompattest(){\nreturn (document.compatMode && document.compatMode != \"BackCompat\")? document.documentElement : document.body\n}\n\nfunction clearbrowseredge(obj, whichedge){\nvar edgeoffset = (whichedge == \"rightedge\")? parseInt(horizontal_offset)*-1 : parseInt(vertical_offset)*-1\nif (whichedge == \"rightedge\"){\nvar windowedge = ie && !window.opera? iecompattest().scrollLeft+iecompattest().clientWidth-30 : window.pageXOffset+window.innerWidth-40\ndropmenuobj.contentmeasure = dropmenuobj.offsetWidth\nif (windowedge-dropmenuobj.x < dropmenuobj.contentmeasure)\nedgeoffset = dropmenuobj.contentmeasure+obj.offsetWidth+parseInt(horizontal_offset)\n}\nelse{\nvar windowedge = ie && !window.opera? iecompattest().scrollTop+iecompattest().clientHeight-15 : window.pageYOffset+window.innerHeight-18\ndropmenuobj.contentmeasure = dropmenuobj.offsetHeight\nif (windowedge-dropmenuobj.y < dropmenuobj.contentmeasure)\nedgeoffset = dropmenuobj.contentmeasure-obj.offsetHeight\n}\nreturn edgeoffset\n}\n\n");
	fprintf(fp_w,"function showhint(menucontents, obj, e){\nif ((ie||ns6) && document.getElementById(\"hintbox\")){\ndropmenuobj = document.getElementById(\"hintbox\")\ndropmenuobj.innerHTML = menucontents\ndropmenuobj.style.left = dropmenuobj.style.top = -500\nif (tipwidth != \"\"){\ndropmenuobj.widthobj = dropmenuobj.style\ndropmenuobj.widthobj.width = tipwidth\n}\ndropmenuobj.x = getposOffset(obj, \"left\")\ndropmenuobj.y = getposOffset(obj, \"top\")\ndropmenuobj.style.left = dropmenuobj.x-clearbrowseredge(obj, \"rightedge\")+obj.offsetWidth+\"px\"\ndropmenuobj.style.top = dropmenuobj.y-clearbrowseredge(obj, \"bottomedge\")+\"px\"\ndropmenuobj.style.visibility = \"visible\"\nobj.onmouseout = hidetip\n}\n}\n\nfunction hidetip(e){\ndropmenuobj.style.visibility = \"hidden\"\ndropmenuobj.style.left = \"-500px\"\n}\n\nfunction createhintbox(){\nvar divblock = document.createElement(\"div\")\ndivblock.setAttribute(\"id\", \"hintbox\")\ndocument.body.appendChild(divblock)\n}\n\nif (window.addEventListener)\nwindow.addEventListener(\"load\", createhintbox, false)\nelse if (window.attachEvent)\nwindow.attachEvent(\"onload\", createhintbox)\nelse if (document.getElementById)\nwindow.onload = createhintbox\n\n</script>\n\n<BODY>\n<center>\n");
	//fprintf(fp_w,"과목명 위에 마우스를 올리면 교수님 성함과 강의실이 나타납니다.\n");	// TODO: 교수님 성함과 강의실 보이도록 변경
}

int ishangul2nd(byte *s, int pos)
{
    int i;

    if (pos < 1 || pos > (signed) strlen(s) - 1 || s[pos - 1] < 128) return 0;

    for (i = 0; i < pos; )
        if (isascii(s[i])) i++;
        else i += 2;

    return ((i == pos + 1) ? 1 : 0);
}

void print_table(FILE *fp_w,int num_table)
{
	int i,j,k;
	int colornum, lastclass = 0;
	
	for (i = 1;i<16;i++)
		for (j = 0;j<6;j++)
			if (table[i][j])	lastclass = i;

	fprintf(fp_w,"<BR><BR><TABLE>\n<TR><TD>[ %03d ]</TD><TD class = DAY>월요일</TD><TD class = DAY>화요일</TD><TD class = DAY>수요일</TD><TD class = DAY>목요일</TD><TD class = DAY>금요일</TD><TD class = DAY>토요일</TD></TR>\n",num_table);
	for (i = 1;i<14;i++)	
	{
		if (i>10 && i>lastclass) break;
		fprintf(fp_w, "<TR>");
		fprintf(fp_w,"<TD class = CLASSNUM>%2d교시</TD>",i);
		for (j = 0;j<6;j++)
		{
			if (table[i][j])
			{
				for (k = 0;k<cnt;k++)  // 과목별 색깔구분
				{		
					if (!strcmp(name_list[k],name_list[table[i][j]-1]))
					{
						colornum  =  k+1;
						break;
					}
				}

				//fprintf(fp_w,"<TD class = CLASS%02d onMouseover = \"showhint('%s<br>%s<br>%s', this, event)\">",colornum,name_list[table[i][j]-1],prof_list[table[i][j]-1],croom_list[table[i][j]-1]);
				fprintf(fp_w,"<TD class = CLASS%02d onMouseover = \"showhint('%s', this, event)\">",colornum,name_list[table[i][j]-1]);	// TODO: 교수님 성함과 강의실 보이도록 변경
				for (k = 0;k<10;k++)	// 과목명은 첫 5글자만 보여주기
				{
					fprintf(fp_w,"%c",name_list[table[i][j]-1][k]);
				}
				if (ishangul2nd(name_list[table[i][j]-1],10))	// 한글 짤렸으면 한바이트 더 붙임
					fprintf(fp_w,"%c",name_list[table[i][j]-1][10]);
				fprintf(fp_w,"</TD>");
			}
			else
				fprintf(fp_w,"<TD></TD>");
		}
		fprintf(fp_w, "</TR>\n");
	}
	fprintf(fp_w, "</TABLE><BR>\n");
}
