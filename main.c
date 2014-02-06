/**************************************************************/
/*              ȫ�ʹ��б� �ð�ǥ�ۼ� �����                    */
/*															  */
/*                          HONGSI                            */
/*															  */
/*                                         github.com/hongsi  */
/**************************************************************/

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "resource.h"

int unit, unit_list[32]; //unit�� ����.
int cnt = 0, next_chk = 0;	// cnt�� Date_list[cnt] �� ī���ͷ� ���̴ٰ� read()������ �Ѱ������ ���� 
char table[16][8];	// �ð�ǥ(2���� �迭)
char name[64] = {0,}, old_name[64] = {0,}, time[32];
char name_list[32][64];	// 32����, ����� 32�ڱ��� ����
char prof_name[32] = {0,}, prof_list[16][32];
char croom_name[32] = {0,}, croom_list[16][32];
char state_str[256];

typedef struct Data *list_pointer;
struct Data
{
	char time[32];			// ��23,��5 ���� ������ ����
	int	 time_list[32];		// ��23,��5�� ��ȯ�� 203052 ���� ������ ����
	list_pointer next;
};

list_pointer Data_list[16],curr,ptr; // curr�� ���� ���, ptr�� ���� �Ҵ�� ���

void start();
int read(FILE *);	// ���Ͽ��� �о����
void analyse();		// �о���� ������� ���ǽð� ������ ���������� ����
int what_day(char*);	// �ѱ� ���ڸ� �о� ���� �������� �Ǵ�
void state();		// �о���� ����鿡 ���� ���� ���
int make_table(FILE *);	// ������ ������ ������ ����ϴ� �Լ�
int insert_table(list_pointer, int);	// �ð�ǥ �迭�� ������ ����
void restore(list_pointer);	// ������ ������ �ð�ǥ �迭���� ����
void init_table();	// �ð�ǥ �迭 �ʱ�ȭ
void print_table_head(FILE *);
int ishangul2nd(byte *, int);  // �ѱ� �ι�° ����Ʈ���� üũ (�Ѷ�����, http://kldp.org/node/75688)
void print_table(FILE *,int);	// �ð�ǥ ���
void free();		// �޸� ��ȯ <- �۾� �ʿ�

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPSTR lpszClass = "ȫ�� 3.2 - ȫ�ʹ� �ð�ǥ �ۼ����α׷�";
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
	WndClass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON2));
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
		
		/*SetBkMode(hdc,TRANSPARENT);
		//SetTextAlign(hdc,TA_CENTER);
		TextOut(hdc,20,20,"1. �޸��忡 ������ ���ϴ� ������ �ٿ��ְ�",41);
		TextOut(hdc,20,40,"2. ���ϸ��� class.txt �� �Ͽ�",29);
		TextOut(hdc,20,60,"3. hongsi.exe�� �ִ� ������ �����ϼ���.",39);
		TextOut(hdc,20,90,"4. HONGSI�� ������ ������ ������ �ð�ǥ��",42);
		TextOut(hdc,20,110,"5. ���� ������ �ð�ǥ.html �� ����ǹǷ�",40);
		TextOut(hdc,20,130,"6. �޸������� ��� �μ��� �� �ֽ��ϴ�.",41);*/	

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
			160,150,50,25,hWnd,(HMENU)1,g_hInst,NULL);	
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
		
		/*SetBkMode(hdc,TRANSPARENT);
		SetTextAlign(hdc,TA_CENTER);
		TextOut(hdc,200,20,"HongIk Univ. Timetable Planer",29);
		TextOut(hdc,200,50,"HongSi",6);
		TextOut(hdc,200,170,"LMN Programming Society",23);*/

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
	fp_r = fopen("class.txt","r");
	fp_w = fopen("�ð�ǥ.html","w");	
	
	unit = 0;
	for (i = 0;i<32;i++)
		unit_list[i] = 0;

	cnt = 0, next_chk = 0; 

	for (i = 0;i<16;i++)
		for (j = 0;j<8;j++)
			table[i][j] = '\0';

	for (i = 0;i<32;i++)
	{
		name[i] = '\0';
		old_name[i] = '\0';
	}

	for (i = 0;i<32;i++)
		time[i] = '\0';

	for (i = 0;i<32;i++)
		for (j = 0;j<64;j++)
			name_list[i][j] = '\0';
		
	for (i = 0;i<256;i++)
		state_str[i] = '\0';
	
	if (read(fp_r))	return;
	fclose(fp_r);	

	if (make_table(fp_w))	return;	
	fclose(fp_w);

	GetCurrentDirectory(256, buffer);
	sprintf(buffer, "%s\\%s", buffer, "�ð�ǥ.html");
	ShellExecute(NULL, "open", "iexplore.exe", buffer, NULL, SW_SHOWNORMAL);
}

int read(FILE *fp_r)
{
    int i, strcnt;
	char c, str[32];
	
	if (fp_r == NULL)
	{
		MessageBox(hWnd,"class.txt ���Ͽ� ���ϴ� ������� �ٿ� ��������.","�˸�",MB_OK);
		fp_r = fopen("class.txt","w");
		fputs("�� ���Ͽ� Ŭ�������� ������� �ܾ���̼���.",fp_r);
		fclose(fp_r);
		return -1;
	}
	
	for (i = 1;c = getc(fp_r);i++)
	{
		if (i == 1)	
			time[0] = '\0';
		
		while (c == ' ' || c == '\t')	
			c = getc(fp_r);	// ������ ������ ���
		
		if (i == 1 && c == EOF && Data_list[0] == NULL)
		{
			MessageBox(hWnd,"class.txt �� ����ֽ��ϴ�.","�˸�",MB_OK);
			return -1;
		}
		
		strcnt = 0;
		while (c != ' ' && c != '\t' && c != '\n' && c != EOF)
		{
			str[strcnt++] = c;
			c = getc(fp_r);		
		}
		str[strcnt] = '\0';	// ���ڿ� ���� �ι��� ����		
	
		if (i == 1 && str[0] == '#')	// ���� ù ���ڰ� '#'�̸� ���� ����
		{
			while (c != '\n')	
				c = getc(fp_r);
			
			i = 0;
			continue;
		}
		else if (i == 1 && str[0]<0)	// �г��� ������ ���
		{
			i = 2;
		}
		else if (i == 4 && str[0]<0)	// ���翵�� �׸��� ������ ī��Ʈ ���� ���� (���뱳��)
		{
			i--;
			continue;
		}
		else if (i == 5)	// ������� name�� ����
		{
			strcpy(name,str);
		}
		else if (i == 6)	// ������ unit�� ����
		{
			if (!isdigit(str[0]))		// ������� ���� �� ��� (��: ADVANCED CAD)
			{
				//sprintf(name, "%s %s", name, str);
				i--;
				continue;
			}

			unit = atoi(str);
		}
		else if (i == 11)
		{
			if (what_day(str) >= 0)	// �������� �� ���Ͻð��� ������ ���
			{
				strcpy(prof_name," ");
				strcpy(time,str);
				i++;
			}
			else
				strcpy(prof_name,str);
		}
		else if (i == 12 && str[0]>0)	// �������� �����ִ� ����(���� �̸�) �ذ�
		{
			strcat(prof_name," ");
			strcat(prof_name,str);
			i--;		
		}
		else if (i == 12 && what_day(str) >= 0)	// ���Ϲ׽ð��� time�� ����
		{
			strcpy(time,str);
			//analyse();
			
			//printf("%s %s\n",name,time);	// �Է¹��� ���� Ȯ�ο�		
		}
		else if (i == 13)
		{
			strcpy(croom_name,str);
			analyse();
		}
		
		if (c == '\n')	 // ���ǽ� ����, ��� ���� ���� �ذ�
		{					
			/*if (i >= 10 && time[0] == '\0')	// ���Ϲ׽ð��� ����ִ� ���		
				printf(" <�˸�> ���Ϲ׽ð��� ����ִ� ���Ǵ� ���õ˴ϴ� : %s\n",name);*/

			i = 0;
			continue;
		}
		else if (c == EOF)	// EOF �� ������ ����
		{
			if (Data_list[0] == NULL || Data_list[0]->time_list[0] == 0)	// �ƹ� �ҵ���� ���� ���
			{
				MessageBox(hWnd,"class.txt �� �ùٸ� ������û ������ �����ϴ�.","�˸�",MB_OK);
				return -1;
			}
			else			// ���� ����.
				return 0;
		}
	}
	return 0;
}

void analyse()
{
	int i = 0, j = 0, k, temp;
	int old_cnt = 0, exist = 0;
	
	ptr = (list_pointer) malloc (sizeof(struct Data));
	ptr->next = NULL;
	strcpy(ptr->time,time);			// ��23,��7 ���� ������ ptr->time�� ����
		
	for (k = 0;k<cnt;k++)	// �̹� �����ϴ� �������� name_list���� �˻�
	{		
		if (!strcmp(name_list[k],name))
		{
			exist = 1;
			old_cnt = k;
			break;
		}
	}
	if (!exist)	// ���ο� ���� �߰��̸�
	{		
		strcpy(name_list[cnt],name);	// ����� ����Ʈ�� �߰�
		strcpy(prof_list[cnt],prof_name);	// ������ ����
		strcpy(croom_list[cnt],croom_name);	// ���ǽ�
		unit_list[cnt] = unit;	// ���� ����Ʈ�� �߰�
		Data_list[cnt++] = curr = ptr;		// Data_list�� ���ο� ��� ��忬��
	}
	else		// �̹� �����ϴ� �����̸�
	{		
		curr = Data_list[old_cnt];
		while (1)				// ��� ������ ����
		{
			if (!strcmp(curr->time,time))	// ���� ����, ���� �ð��̸� �������
			{
				free(ptr);
				return;			
			}
			if (curr->next == NULL)	// ��� ���� ������ loop ����
				break;	
			else					// �ƴϸ� ���� ���� �̵�
				curr = curr->next;
		}
		curr = curr->next = ptr;	// ��� ���� �� ���� ����
	}

	while (time[i])	// ��23,��7 ���� ������ ���� 020327 �� ���� data ����
	{
		if (time[i]<0)	// time[0]�� �ѱ��̸�
		{
			curr->time_list[j++] = what_day(&time[i]);
			i += 2;
		}
		else
		{
			if (j%2 == 0)
				curr->time_list[j++] = curr->time_list[j-2];

			curr->time_list[j++] = (int)time[i++]-48;
			if (curr->time_list[j-1] == 1 && (int)time[i] == 48)	 // 10����
			{
				curr->time_list[j-1] = 10;
				i++;
			}
			else if (curr->time_list[j-1] == 1 && (int)time[i] == 49)	 // 11����
			{
				curr->time_list[j-1] = 11;
				i++;
			}
			else if (curr->time_list[j-3] + 1 == curr->time_list[j-1] * 10 + ((int)time[i] - 48))  // 12, 13, .. ����
			{
				curr->time_list[j-1] = curr->time_list[j-1] * 10 + ((int)time[i] - 48);
				i++;
			}
		}		
		if (time[i] == ',')	i++;		
	}
	curr->time_list[j] = -1;	// ���ڿ� time_list�� ��ǥ�÷� -1 ������
	
	for (i = 0;curr->time_list[i] != -1;i += 2)	// ��23,��7 ���� 020327 �� �ٲ� ������ 203072 �� ��ȯ
	{
		temp = curr->time_list[i];
		curr->time_list[i] = curr->time_list[i+1];
		curr->time_list[i+1] = temp;		
	}
	strcpy(old_name,name);
}

int what_day(char* str)	// ���ڿ��� ù���ڰ� ������������ �Ǻ�
{
	if (!strncmp(str,"��",2))	return 0;
	else if (!strncmp(str,"ȭ",2))	 return 1;
	else if (!strncmp(str,"��",2))	 return 2;
	else if (!strncmp(str,"��",2))	 return 3;
	else if (!strncmp(str,"��",2))	 return 4;
	else if (!strncmp(str,"��",2))	 return 5;
	else return -1;
}

void state()
{	
	int i,j,k;
	char str[32];
		
	for (i = 0,j = 0;i<cnt;i++)
		j += unit_list[i];

	sprintf(state_str,"\n��� %d����, %d�����Դϴ�.\n",cnt,j);

	for (i = 0,j = 0;i<cnt;i++)	// ���� �̸��� ����
	{
		//putchar('\n');
		strcat(state_str,"\n");
		//putchar(' ');
		//strcat(state_str," ");
		for (j = 0;name_list[i][j];j++)
		{
			sprintf(str,"%c",name_list[i][j]);
			strcat(state_str,str);
		}
		for (k = 0;(unsigned)k<2-(strlen(name_list[i])+1)/10;k++)
			//putchar('\t');
			strcat(state_str,"\t");
		sprintf(str,"(%d����)   ",unit_list[i]);
		strcat(state_str,str);
	}
	//puts("\n");
	//strcat(state_str,"\n");
	
	/*list_pointer now;		
	for (i = 0;i<cnt;i++)	// ���ǽð� ����
	{
		now = Data_list[i];
		do{
			while (now->time_list[j] != -1)
			{
				printf("%d ",now->time_list[j++]);
			}
			j = 0;
			putchar('\n');
			now = now->next;
		}while (now != NULL);

		putchar('\n');
	}*/	
}

int make_table(FILE *fp_w)
{	
	int i, num_table = 1;	
	
	list_pointer lec_ptr[24];	// �� ������� ����Ű�� ������
	
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
			if (insert_table(lec_ptr[i],i+1))	// 1�� ���ϵǸ� �浹�� �ִٴ� ��
			{
				//printf("%d�� ���� �浹\n",i+1);
				lec_ptr[i] = lec_ptr[i]->next;
				
				while (lec_ptr[i] == NULL)
				{
					lec_ptr[i] = Data_list[i];	// �浹�� �ִ� ������ �ٽ� ù��带 ����Ű�� �ϰ�					
					if (--i<0)
					{
						if (--num_table)
						{
							state();
							MessageBox(hWnd,state_str,"�˸�",MB_OK);
							return 0;
						}
						else
						{
							state();
							MessageBox(hWnd,"��� ������ �����ϴ� �ð�ǥ�� �����ϴ�.","�˸�",MB_OK);
							return -1;
						}
					}
					restore(lec_ptr[i]);	 // �հ��� �ð�ǥ ������ �ʱ�ȭ�ϰ�
					lec_ptr[i] = lec_ptr[i]->next;	 // �հ��� ��� �������̵� �� �հ������ �ٽ� ����
				}				
				i--;
			}
			//print_table();	// ���� ���� ���
		}	// ���� : for���� ��� ���� i�� cnt�� ����
		
		//printf("\t\t\t\t\t�ϼ� - %2d�� �ð�ǥ\n",num_table++);
		print_table(fp_w,num_table++);

		i--;
		lec_ptr[i] = lec_ptr[i]->next;	// �ð�ǥ�ϳ� �ϼ��ϸ� �������������� �������� �̵��ϰ� ���
		while (lec_ptr[i] == NULL)
		{
			lec_ptr[i] = Data_list[i];	// �浹�� �ִ� ������ �ٽ� ù��带 ����Ű�� �ϰ�					
			if (--i<0)
			{
				if (--num_table)
				{
					state();					
					MessageBox(hWnd,state_str,"�˸�",MB_OK);
					return 0;
				}
				else
				{
					state();				
					MessageBox(hWnd,"��� ������ �����ϴ� �ð�ǥ�� �����ϴ�.","�˸�",MB_OK);
					return -1;
				}
			}			
			lec_ptr[i] = lec_ptr[i]->next;	 // �հ��� ��� �������̵� �� �հ������ �ٽ� ����
		}
		
	}

}

int insert_table(list_pointer now, int lec)
{
	int i;	

	for (i = 0;now->time_list[i] != -1;i += 2)	// �浹�� �ϳ��� �ִ��� Ȯ��
	{
		if ((table[now->time_list[i]][now->time_list[i+1]]))
			return 1;		
	}
	
	for (i = 0;now->time_list[i] != -1;i += 2)	// �浹�� �ϳ��� ������ �Է�
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

void print_table_head(FILE *fp_w)	// IE security warning ���� (http://msdn.microsoft.com/en-us/library/ms537628(v=vs.85).ASPX)
{
	fprintf(fp_w,"<HTML>\n<HEAD><TITLE> �ð�ǥ </TITLE></HEAD>\n\n<style type = \"text/css\">\n\nBODY,TD,DIV,LI	{ FONT-FAMILY : APPLEGOTHIC,����,SANS-SERIF; FONT-SIZE : 9PT; COLOR : #333; TEXT-ALIGN : CENTER; LINE-HEIGHT : 150%%;}\nTABLE { border-collapse: collapse; }\nTD { WIDTH : 70; HEIGHT : 20; border: 1px solid #E8E8E8; }\nTD.DAY { BACKGROUND-COLOR : #CCFFFF; }\nTD.CLASSNUM { BACKGROUND-COLOR : #FFF4FF; }\nTD.CLASS01 { BACKGROUND-COLOR : #C3D2F1; }\nTD.CLASS02 { BACKGROUND-COLOR : #FFEDBB; }\nTD.CLASS03 { BACKGROUND-COLOR : #DEE6D0; }\nTD.CLASS04 { BACKGROUND-COLOR : #F6CCCD; }\nTD.CLASS05 { BACKGROUND-COLOR : #D9CDEE; }\nTD.CLASS06 { BACKGROUND-COLOR : #CADED8; }\nTD.CLASS07 { BACKGROUND-COLOR : #FAD9BE; }\nTD.CLASS08 { BACKGROUND-COLOR : #E9D0DC; }\nTD.CLASS09 { BACKGROUND-COLOR : #FBFCBD; }\nTD.CLASS10 { BACKGROUND-COLOR : #CFDEEA; }\nTD.CLASS11 { BACKGROUND-COLOR : #D7CFDC; }\nTD.CLASS12 { BACKGROUND-COLOR : #F3EAD1; }\n\n#hintbox{ position:absolute; top: 0; background-color: lightyellow; width: 150px; \npadding: 3px; border:1px solid black; font:normal 11px Verdana; line-height:18px;\nz-index:100; visibility: hidden; }\n\n</style>\n\n");
	fprintf(fp_w,"<!-- saved from url=(0014)about:internet -->\n\n<script type = \"text/javascript\">\n\nvar horizontal_offset = \"5px\"\nvar vertical_offset = \"0\"\nvar ie = document.all\nvar ns6 = document.getElementById&&!document.all\nvar tipwidth = 130\n\nfunction getposOffset(what, offsettype){\nvar totaloffset = (offsettype == \"left\")? what.offsetLeft : what.offsetTop;\nvar parentEl = what.offsetParent;\nwhile (parentEl != null){\ntotaloffset = (offsettype == \"left\")? totaloffset+parentEl.offsetLeft : totaloffset+parentEl.offsetTop;\nparentEl = parentEl.offsetParent;\n}\nreturn totaloffset;\n}\n\nfunction iecompattest(){\nreturn (document.compatMode && document.compatMode != \"BackCompat\")? document.documentElement : document.body\n}\n\nfunction clearbrowseredge(obj, whichedge){\nvar edgeoffset = (whichedge == \"rightedge\")? parseInt(horizontal_offset)*-1 : parseInt(vertical_offset)*-1\nif (whichedge == \"rightedge\"){\nvar windowedge = ie && !window.opera? iecompattest().scrollLeft+iecompattest().clientWidth-30 : window.pageXOffset+window.innerWidth-40\ndropmenuobj.contentmeasure = dropmenuobj.offsetWidth\nif (windowedge-dropmenuobj.x < dropmenuobj.contentmeasure)\nedgeoffset = dropmenuobj.contentmeasure+obj.offsetWidth+parseInt(horizontal_offset)\n}\nelse{\nvar windowedge = ie && !window.opera? iecompattest().scrollTop+iecompattest().clientHeight-15 : window.pageYOffset+window.innerHeight-18\ndropmenuobj.contentmeasure = dropmenuobj.offsetHeight\nif (windowedge-dropmenuobj.y < dropmenuobj.contentmeasure)\nedgeoffset = dropmenuobj.contentmeasure-obj.offsetHeight\n}\nreturn edgeoffset\n}\n\n");
	fprintf(fp_w,"function showhint(menucontents, obj, e){\nif ((ie||ns6) && document.getElementById(\"hintbox\")){\ndropmenuobj = document.getElementById(\"hintbox\")\ndropmenuobj.innerHTML = menucontents\ndropmenuobj.style.left = dropmenuobj.style.top = -500\nif (tipwidth != \"\"){\ndropmenuobj.widthobj = dropmenuobj.style\ndropmenuobj.widthobj.width = tipwidth\n}\ndropmenuobj.x = getposOffset(obj, \"left\")\ndropmenuobj.y = getposOffset(obj, \"top\")\ndropmenuobj.style.left = dropmenuobj.x-clearbrowseredge(obj, \"rightedge\")+obj.offsetWidth+\"px\"\ndropmenuobj.style.top = dropmenuobj.y-clearbrowseredge(obj, \"bottomedge\")+\"px\"\ndropmenuobj.style.visibility = \"visible\"\nobj.onmouseout = hidetip\n}\n}\n\nfunction hidetip(e){\ndropmenuobj.style.visibility = \"hidden\"\ndropmenuobj.style.left = \"-500px\"\n}\n\nfunction createhintbox(){\nvar divblock = document.createElement(\"div\")\ndivblock.setAttribute(\"id\", \"hintbox\")\ndocument.body.appendChild(divblock)\n}\n\nif (window.addEventListener)\nwindow.addEventListener(\"load\", createhintbox, false)\nelse if (window.attachEvent)\nwindow.attachEvent(\"onload\", createhintbox)\nelse if (document.getElementById)\nwindow.onload = createhintbox\n\n</script>\n\n<BODY>\n<center>\n");
	//fprintf(fp_w,"����� ���� ���콺�� �ø��� ������ ���԰� ���ǽ��� ��Ÿ���ϴ�.\n");	// TODO: ������ ���԰� ���ǽ� ���̵��� ����
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

	fprintf(fp_w,"<BR><BR><TABLE>\n<TR><TD>[ %03d ]</TD><TD class = DAY>������</TD><TD class = DAY>ȭ����</TD><TD class = DAY>������</TD><TD class = DAY>�����</TD><TD class = DAY>�ݿ���</TD><TD class = DAY>�����</TD></TR>\n",num_table);
	for (i = 1;i<14;i++)	
	{
		if (i>10 && i>lastclass) break;
		fprintf(fp_w, "<TR>");
		fprintf(fp_w,"<TD class = CLASSNUM>%2d����</TD>",i);
		for (j = 0;j<6;j++)
		{
			if (table[i][j])
			{
				for (k = 0;k<cnt;k++)  // ���� ���򱸺�
				{		
					if (!strcmp(name_list[k],name_list[table[i][j]-1]))
					{
						colornum  =  k+1;
						break;
					}
				}

				//fprintf(fp_w,"<TD class = CLASS%02d onMouseover = \"showhint('%s<br>%s<br>%s', this, event)\">",colornum,name_list[table[i][j]-1],prof_list[table[i][j]-1],croom_list[table[i][j]-1]);
				fprintf(fp_w,"<TD class = CLASS%02d onMouseover = \"showhint('%s', this, event)\">",colornum,name_list[table[i][j]-1]);	// TODO: ������ ���԰� ���ǽ� ���̵��� ����
				for (k = 0;k<10;k++)	// ������� ù 5���ڸ� �����ֱ�
				{
					fprintf(fp_w,"%c",name_list[table[i][j]-1][k]);
				}
				if (ishangul2nd(name_list[table[i][j]-1],10))	// �ѱ� ©������ �ѹ���Ʈ �� ����
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