#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include "Shapes.h"


#define ID_TIMER  1
#define TIME_INTERVAL 1000   // Falling Interval 1 second

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
VOID    CALLBACK TimerProc(HWND, UINT, UINT, DWORD);

// constants 
#define BOARD_WIDTH 180
#define BOARD_HEIGHT 400
#define LONG_SLEEP 300

#define COLS 15  // number of columns
#define ROWS 30  // number of rows
#define EXTENDED_COLS 23
#define EXTENDED_ROWS 34

#define BOARD_LEFT 4
#define BOARD_RIGHT 18
#define BOARD_TOP 0
#define BOARD_BOTTOM 29



//Parameters Declaration

static int shape[4][4];
static int score = 0;

static int shape_row = 0;  // The current shape is in this row
static int shape_col = EXTENDED_COLS / 2 - 2; // The current shape is in this column

static int **gBoard;

static int lattices_top = 40;   // White space on top
static int lattices_left = 20;  // Left side white
static int width = BOARD_WIDTH / COLS;                    //Width of each cell
static int height = (BOARD_HEIGHT - lattices_top) / ROWS; //Height of each cell

static HBRUSH grey_brush = CreateSolidBrush(RGB(210, 210, 210));
static HBRUSH white_brush = CreateSolidBrush(RGB(130, 130, 130));
static HPEN hPen = CreatePen(PS_SOLID, 1, RGB(147, 155, 166));

static bool gIsPause = false;  // Determine whether to pause


//Function Declarations
void InitGame(HWND);
void InitData();

void TypeInstruction(HWND);

void RandShape();  // Randomly select a shape

void AddScore();   // Add 100 points for a blank line

void UpdateShapeRect(HWND hwnd); 
void UpdateAllBoard(HWND hwnd);  

void FallToGround();
void MoveDown(HWND hwnd);  
void RePaintBoard(HDC hdc); // Redrawing the Game Interface
void PaintCell(HDC hdc, int x, int y, int color); // Draw a specified grid
void ClearFullLine();      

void RotateShape(HWND hwnd);  
void MoveHori(HWND hwnd, int direction);  // Horizontal movement
void RotateMatrix();         
void ReRotateMatrix();        
bool IsLegel();               // Detects if the graph is out of range

void RespondKey(HWND hwnd, WPARAM wParam); 

void PauseGame(HWND hwnd); 
void WakeGame(HWND hwnd); 


bool JudgeLose();         
void LoseGame(HWND hwnd);  
void ExitGame(HWND hwnd);  




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("TetrisGame_Zjy");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	// Here the window is set to non-adjustable size and not maximized.
	hwnd = CreateWindow(szAppName, TEXT("Tetris Game"),
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		BOARD_WIDTH + 220, BOARD_HEIGHT + 70,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	//Print Instructions
	TypeInstruction(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC hdc;
	static HDC hdcBuffer;
	static HBITMAP hBitMap;
	static PAINTSTRUCT ps;

	switch (message)
	{
	case WM_CREATE:
		SetTimer(hwnd, ID_TIMER, TIME_INTERVAL, (TIMERPROC)TimerProc);
		InitGame(hwnd);
		TypeInstruction(hwnd);
		return 0;

	
	case WM_SIZE:
		TypeInstruction(hwnd);
		return 0;

	case WM_KEYDOWN:
		RespondKey(hwnd, wParam);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		RePaintBoard(hdc);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

// Timer response events
VOID CALLBACK TimerProc(HWND hWnd, UINT uMessage, UINT uEventId, DWORD dwTime)
{	
	// Timer moves down every second
	MoveDown(hWnd);
}


void InitGame(HWND hwnd) {

	gBoard = new int* [EXTENDED_ROWS];
	for (int i = 0; i < EXTENDED_ROWS; i++) {
		gBoard[i] = new int[EXTENDED_COLS];
	}

	srand(time(0));
	
	InitData();
	
	UpdateAllBoard(hwnd);
}

void InitData() {

	// Zeroing out the game panel
	for (int i = 0; i < EXTENDED_ROWS; i++) {
		for (int j = 0; j < EXTENDED_COLS; j++) {
			gBoard[i][j] = 0;
		}
	}

	// Fill the periphery with 1, in order to determine if it is out of range.
	for (int i = 0; i < EXTENDED_ROWS; i++) {
		for (int j = 0; j < BOARD_LEFT; j++) {
			gBoard[i][j] = 1;
		}
	}
	for (int i = 0; i < EXTENDED_ROWS; i++) {
		for (int j = BOARD_RIGHT + 1; j < EXTENDED_COLS; j++) {
			gBoard[i][j] = 1;
		}
	}
	for (int i = BOARD_BOTTOM + 1; i < EXTENDED_ROWS; i++) {
		for (int j = 0; j < EXTENDED_COLS; j++) {
			gBoard[i][j] = 1;
		}
	}

	gIsPause = false;


	score = 0;
	
	
	RandShape();
	
	return;
}

void TypeInstruction(HWND hwnd) {

	TEXTMETRIC  tm;
	int cxChar, cxCaps, cyChar, cxClient, cyClient, iMaxWidth;

	HDC hdc = GetDC(hwnd);

	// Save line height and width information
	GetTextMetrics(hdc, &tm);
	cxChar = tm.tmAveCharWidth;
	cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
	cyChar = tm.tmHeight + tm.tmExternalLeading;

	int startX = 180;
	int startY = 40;

	TCHAR Instruction[100];

	wsprintf(Instruction, TEXT("INSTRUCTION "));
	TextOut(hdc, startX + 40, startY, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("↑       Change Shape"));
	TextOut(hdc, startX + 40, startY + cyChar * 3, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("←       Move Left"));
	TextOut(hdc, startX + 40, startY + cyChar * 5, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("→       Move Right"));
	TextOut(hdc, startX + 40, startY + cyChar * 7, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("↓       Move Down"));
	TextOut(hdc, startX + 40, startY + cyChar * 9, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("Space Pause the game"));
	TextOut(hdc, startX + 40, startY + cyChar * 11, Instruction, lstrlen(Instruction));

	wsprintf(Instruction, TEXT("Esc     Exit the game"));
	TextOut(hdc, startX + 40, startY + cyChar * 13, Instruction, lstrlen(Instruction));

	ReleaseDC(hwnd, hdc);
}


void RandShape() {

	int shape_num = rand() % 7;

	for (int i = 0; i < 4; i++) 
		for (int j = 0; j < 4; j++) 
			shape[i][j] = shapes[shape_num][i][j];

}

// Update the entire game interface
void UpdateAllBoard(HWND hwnd) {

	static RECT rect;

	rect.left = lattices_left;
	rect.right = lattices_left + COLS * width + width;
	rect.top = lattices_top - 30;
	rect.bottom = lattices_top + ROWS * height;

	InvalidateRect(hwnd, &rect, false);

}

// Update the rectangle in which the drop shape is located.
void UpdateShapeRect(HWND hwnd) {

	static RECT rect;

	rect.left = lattices_left;
	rect.right = lattices_left + COLS * width + width;
	rect.top = lattices_top + (shape_row - 1) * height;
	rect.bottom = lattices_top + (shape_row + 4) * height;
	
	InvalidateRect(hwnd, &rect, false);
}

void RePaintBoard(HDC hdc) {

	SetBkColor(hdc, RGB(255, 255, 255));
	SelectObject(hdc, hPen);   //Selected brushes
	TCHAR score_str[50];

	// Plotting the current score
	wsprintf(score_str, TEXT("Score: %6d     "), score);
	TextOut(hdc, 20, 15, score_str, lstrlen(score_str));

	// Drawing Game Interface Backgrounds
	for (int i = BOARD_TOP; i <= BOARD_BOTTOM; i++) {
		for (int j = BOARD_LEFT; j <= BOARD_RIGHT; j++) {
			PaintCell(hdc, i, j, gBoard[i][j]);
		}
	}

	
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j<4; j++) {
			if (shape[i][j] == 1)
				PaintCell(hdc, shape_row + i, shape_col + j, shape[i][j]);
		}
	}
}

// Prints a grid of the specified color in the specified position.
void PaintCell(HDC hdc, int x, int y, int color) {

	// Out of Range Direct End
	if (x < BOARD_TOP || x > BOARD_BOTTOM || 
		y < BOARD_LEFT || y > BOARD_RIGHT) {
		return;
	}

	x -= BOARD_TOP;
	y -= BOARD_LEFT;

	// Converting Coordinates to Actual Pixels
	int _left = lattices_left + y * width;
	int _right = lattices_left + y * width + width;
	int _top = lattices_top + x * height;
	int _bottom = lattices_top + x * height + height;

	// Drawing Borders
	MoveToEx(hdc, _left, _top, NULL);
	LineTo(hdc, _right, _top);
	MoveToEx(hdc, _left, _top, NULL);
	LineTo(hdc, _left, _bottom);
	MoveToEx(hdc, _left, _bottom, NULL);
	LineTo(hdc, _right, _bottom);
	MoveToEx(hdc, _right, _top, NULL);
	LineTo(hdc, _right, _bottom);

	// If color is 1 then use the gray brush
	SelectObject(hdc, grey_brush);
	if (color == 0) {
		SelectObject(hdc, white_brush);
	}

	// Filling
	Rectangle(hdc, _left, _top, _right, _bottom);
}

void RespondKey(HWND hwnd, WPARAM wParam) {

	if (wParam == VK_ESCAPE) {
		ExitGame(hwnd);
		return;
	}
	if (wParam == VK_SPACE) {
		gIsPause = !gIsPause;
		if (gIsPause == true) {
			PauseGame(hwnd);
			return;
		}
		else if (gIsPause == false) {
			WakeGame(hwnd);
			return;
		}
	}


	
	if (!gIsPause) {
		if (wParam == VK_UP) {
			RotateShape(hwnd);
			return;
		}
		if (wParam == VK_DOWN) {
			MoveDown(hwnd);
			return;
		}
		if (wParam == VK_LEFT) {
			MoveHori(hwnd, 0);
			return;
		}
		if (wParam == VK_RIGHT) {
			MoveHori(hwnd, 1);
			return;
		}
	}
}

void PauseGame(HWND hwnd) {
	KillTimer(hwnd, ID_TIMER);
}


void WakeGame(HWND hwnd) {
	SetTimer(hwnd, ID_TIMER, TIME_INTERVAL, (TIMERPROC)TimerProc);
}

void ExitGame(HWND hwnd) {
	
	// Pause the game first
	SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, 0);
	
	// Withdrawal or Not
	int flag = MessageBox(NULL, TEXT("Do you want exit?"), TEXT("EXIT"), MB_YESNO);

	if (flag == IDYES) {
		SendMessage(hwnd, WM_DESTROY, NULL, 0);
	}
	else if (flag == IDNO) {
		return;
	}

}


void RotateShape(HWND hwnd) {
	
	RotateMatrix();
	
	if (!IsLegel()) {
		ReRotateMatrix();
	}

	UpdateShapeRect(hwnd);

	return;
}

// Determine if the shape is in the game borders
bool IsLegel() {

	for (int i = 0; i<4; i++)
		for (int j = 0; j<4; j++)
			if (shape[i][j] == 1 && gBoard[shape_row + i][shape_col + j] == 1)
				return false;
	
	return true;
}

// Rotate the shape of the current drop counterclockwise
void RotateMatrix() {

	int (*a)[4] = shape;

	int s = 0;
	
	for (int n = 4; n >= 1; n -= 2) {
		for (int i = 0; i < n - 1; i++) {
			int t = a[s + i][s];
			a[s + i][s] = a[s][s + n - i - 1];
			a[s][s + n - i - 1] = a[s + n - i - 1][s + n - 1];
			a[s + n - i - 1][s + n - 1] = a[s + n - 1][s + i];
			a[s + n - 1][s + i] = t;
		}
		s++;
	}

}


void ReRotateMatrix() {
	int (*a)[4] = shape;
	int s = 0;
	for (int n = 4; n >= 1; n -= 2) {
		for (int i = 0; i<n - 1; i++) {
			int t = a[s + i][s];
			a[s + i][s] = a[s + n - 1][s + i];
			a[s + n - 1][s + i] = a[s + n - i - 1][s + n - 1];
			a[s + n - i - 1][s + n - 1] = a[s][s + n - i - 1];
			a[s][s + n - i - 1] = t;
		}
		s++;
	}
}

// Falling shape down one cell
void MoveDown(HWND hwnd) {
	
	shape_row++;
	
	if (!IsLegel()) {
		shape_row--;

		if (JudgeLose()) {
			LoseGame(hwnd);
			return;
		}
		FallToGround();
		ClearFullLine();
		UpdateAllBoard(hwnd);

		// Resetting the drop shape position
		shape_row = 0;
		shape_col = EXTENDED_COLS / 2 - 2;
		
		RandShape();
	}
	
	UpdateShapeRect(hwnd);
}

// Determine if you've lost
bool JudgeLose() {

	if (shape_row == 0)
		return true;
	
	return false;

}

// Game Over
void LoseGame(HWND hwnd) {

	SendMessage(hwnd, WM_KEYDOWN, VK_SPACE, 0);

	TCHAR words[100];
	wsprintf(words, TEXT("You lose the Game. Your score is %d. \nDo you want try again?"), score);

	int flag = MessageBox(NULL, words, TEXT("EXIT"), MB_YESNO);

	if (flag == IDYES) {
		SendMessage(hwnd, WM_CREATE, NULL, 0);
		return;
	}
	else if (flag == IDNO) {
		SendMessage(hwnd, WM_DESTROY, NULL, 0);
		return;
	}

}

// Graphics on the ground, updating the background array
void FallToGround() {
	for (int i = 0; i<4; i++) {
		for (int j = 0; j<4; j++) {
			gBoard[shape_row + i][shape_col + j] = shape[i][j] == 1 ? 1 : gBoard[shape_row + i][shape_col + j];
		}
	}
}

void ClearFullLine() {
	for (int i = shape_row; i <= shape_row + 3; i++) {
		if (i > BOARD_BOTTOM)continue;
		bool there_is_blank = false;

		// Determine if a line has spaces
		for (int j = BOARD_LEFT; j <= BOARD_RIGHT; j++) {
			if (gBoard[i][j] == 0) {
				there_is_blank = true;
				break;
			}
		}
		if (!there_is_blank) {
			AddScore();
			for (int r = i; r >= 1; r--) {
				for (int c = BOARD_LEFT; c <= BOARD_RIGHT; c++) {
					gBoard[r][c] = gBoard[r - 1][c];
				}
			}
		}
	}
}

// Clear a line plus 100 points
void AddScore() {
	score += 100;
}


void MoveHori(HWND hwnd, int direction) {

	int temp = shape_col;

	// If the direction is 0, move left, otherwise move right.
	if (direction == 0) 
		shape_col--;
	else
		shape_col++;
	
	// If the post-movement position exceeds the boundary
	if (!IsLegel()) {
		shape_col = temp;
	}

	UpdateShapeRect(hwnd);

	return;
}
