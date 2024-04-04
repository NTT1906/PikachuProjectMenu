#include <vector>
#include <thread>
#include "main.h"

using namespace std;

int SCREEN_WIDTH = 0;
int SCREEN_HEIGHT = 0;

COORD getScreenSize() {
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo);
	const auto newScreenWidth = bufferInfo.srWindow.Right - bufferInfo.srWindow.Left + 1;
	const auto newscreenHeight = bufferInfo.srWindow.Bottom - bufferInfo.srWindow.Top + 1;

	return COORD{static_cast<short>(newScreenWidth), static_cast<short>(newscreenHeight)};
}

void syncScrSize() {
	COORD scrSize = getScreenSize();
	SCREEN_WIDTH = scrSize.X;
	SCREEN_HEIGHT = scrSize.Y;
}

string getFileContent(const string &fileName) {
	ifstream file(fileName);
	if (!file.is_open()) return "";
	ostringstream sstr;
	sstr << file.rdbuf();
	file.close();
	return sstr.str();
}

void DisableMinimizeButton() {
	HWND hWnd = GetConsoleWindow();
	HMENU hMenu = GetSystemMenu(hWnd, false);

	DeleteMenu(hMenu, SC_MINIMIZE, MF_BYCOMMAND);
}

void window_init() {
	SetConsoleTitleW(L"Pikachu Game");
	ShowWindow(GetConsoleWindow(),SW_MAXIMIZE);
}

void hideCursor(bool visible = false){
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(handle, &cursorInfo);
	cursorInfo.bVisible = visible; // set the cursor visibility
	SetConsoleCursorInfo(handle, &cursorInfo);
}

void project_init() {
	window_init();
	DWORD dwMode = 0;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	///--------------------- Enable ansi support
	GetConsoleMode(handle, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(handle, dwMode);

	hideCursor();

//	system("mode con COLS=700");
//	SendMessage(GetConsoleWindow(),WM_SYSKEYDOWN,VK_RETURN,0x20000000); Full screen
	HWND hWnd = GetConsoleWindow();
//	ShowScrollBar(hWnd, SB_BOTH, false);

//	DisableResizeWindow(); //auto resize
	DisableMinimizeButton();
//	DeleteMenu(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND);

	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
	SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE);
}

string makeAnsiRGBBG(const string &s = " ", int r = 0, int g = 0, int b = 0) {
	return ("\x1B[48;2;" + to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m" + s);
}

void WINAPI SetConsoleColors(WORD attribs) {
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFOEX cbi;
	cbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	GetConsoleScreenBufferInfoEx(hOutput, &cbi);
	cbi.wAttributes = attribs;
	SetConsoleScreenBufferInfoEx(hOutput, &cbi);
}

void FlashConsoleBackgroundColor(int cntFlashes, int flashInterval_ms, COLORREF color, COLORREF color2) {
	CONSOLE_SCREEN_BUFFER_INFOEX sbInfoEx;
	sbInfoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

	HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfoEx(consoleOut, &sbInfoEx);

	COLORREF storedBG = sbInfoEx.ColorTable[0];

	for (int i = 0; i < 2 * cntFlashes; ++i) {
		if (i % 2 == 0) {
			sbInfoEx.ColorTable[0] = color;
		} else {
			sbInfoEx.ColorTable[0] = color2;
		}
		SetConsoleScreenBufferInfoEx(consoleOut, &sbInfoEx);
		Sleep(flashInterval_ms);
	}
}

COLORREF GetDefaultColor() {
	CONSOLE_SCREEN_BUFFER_INFOEX sbInfoEx;
	HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfoEx(consoleOut, &sbInfoEx);
	return sbInfoEx.ColorTable[0];
}

void FillConsoleBackground(COLORREF color) {
	CONSOLE_SCREEN_BUFFER_INFOEX sbInfoEx;
	sbInfoEx.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	HANDLE consoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfoEx(consoleOut, &sbInfoEx);
	sbInfoEx.ColorTable[0] = color;
	SetConsoleScreenBufferInfoEx(consoleOut, &sbInfoEx);
}

void MoveCursorTo(COORD coord) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hStdout, coord);
}

void drawAtPos(COORD coord, const string& s) {
	istringstream sstr(s);
	string temp;
	while (!sstr.eof()) {
		MoveCursorTo(coord);
		getline(sstr, temp, '\n');
		printf("%s", temp.c_str());
		coord.Y++;
	}
}

void drawAtPosTransparent(COORD coord, const string& s) {
	istringstream sstr(s);
	string temp;	
	while (!sstr.eof()) {
		MoveCursorTo(coord);
		getline(sstr, temp, '\n');
		printf("%s", temp.c_str());
		coord.Y++;
	}
}

void moveMouse(int x, int y) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(handle, {static_cast<SHORT>(x), static_cast<SHORT>(y)});
}

vector<string> pokeballs;

int pokeball_id = 0;

void drawPocketBall() {
	drawAtPos({static_cast<SHORT>(SCREEN_WIDTH / 2 - 23), static_cast<SHORT>(SCREEN_HEIGHT / 4 - 15)}, pokeballs[pokeball_id]);
}


void drawDoubleLayerRectangle(COORD coord, int width, int height) {
	MoveCursorTo(coord);
	string s = "╔";
	string space;
	string temp;
	for (int i = 0; i < width; i++) {
		s += "═";
	}
	cout << s << "╗";

	for (int i = 0; i < height / 2; i++) {
		MoveCursorTo({coord.X, static_cast<SHORT>(coord.Y + i + 1)});

//		cout << "\033[48;2;" << rand() % 255 <<  rand() % 255
//		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), i);
//		printf(("║\033[48;2;%d;%d;%dm" + string(width, ' ') + "\033[39m\033[49m║").c_str(), rand() % 256, rand() % 256, rand() % 256);
		cout << "║\033[48;2;255;255;255m" + string(width, ' ') + "\033[49m║";
//		printf("\033[39m\033[49m");
	}
	s = "╚";
	for (int i = 0; i < width; i++)
		s += "═";
	MoveCursorTo({coord.X, static_cast<SHORT>(coord.Y + height / 2 + 1)});
	s += "╝";
	cout << s;
	MoveCursorTo({0, 50});
}

void drawThinRectangle(COORD coord, int width, int height) {
	MoveCursorTo(coord);
	string s = "┌";
	string space;
	string temp;
	for (int i = 0; i < width; i++) {
		s += "─";
	}
	cout << s << "┐";

	for (int i = 0; i < height / 2; i++) {
		MoveCursorTo({coord.X, static_cast<SHORT>(coord.Y + i + 1)});
		cout << "│" + string(width, ' ') + "│";
	}
	s = "└";
	for (int i = 0; i < width; i++)
		s += "─";
	MoveCursorTo({coord.X, static_cast<SHORT>(coord.Y + height / 2 + 1)});
	s += "┘";
	cout << s;
	MoveCursorTo({0, 50});
}

const int ARROW_UP = 0x48;
const int ARROW_DOWN = 0x50;
const int ESC_KEY = 0x1B;
const int ENTER_KEY = 0x0D;

void control(string* alphabetInputs, string* passInputs, bool* inputPass, bool* entered) {
	while (entered) {
		char input = getch();
		switch (input) {
			case 8:
				if (!*inputPass) {
					if (alphabetInputs->empty()) break;
					alphabetInputs->pop_back();
				} else{
					if (passInputs->empty()) break;
					passInputs->pop_back();
				}
				break;
			case ARROW_UP:
				*inputPass = false;
				break;
			case ARROW_DOWN:
				*inputPass = true;
				break;
			case ENTER_KEY:
				if (!*inputPass) {
					*inputPass = true;
				} else {
					*entered = true;
					system(("start cmd /k echo Hello " + *alphabetInputs + " with pass: " + *passInputs).c_str());
				}
				break;
			default:
				if ((input >= 'A' && input <= 'Z' || (input >= 'a' && input <= 'z') || (input >= '0' && input <= '9'))) {
					if (!*inputPass) alphabetInputs->push_back(input);
					else passInputs->push_back(input);
				}
		}
	}
}

int main() {
	SetConsoleOutputCP(65001);

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hStdin, &mode);
//	SetConsoleColors()
	SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

//	HWND hw = GetConsoleWindow();
//	HFONT hf = CreateFont(30, 1, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Segue UI"));
//	SendMessage(hw, WM_SETFONT, reinterpret_cast<WPARAM>(hf), true);
	project_init();
	COORD c = getScreenSize();
	SCREEN_WIDTH = c.X;
	SCREEN_HEIGHT = c.Y;
	FillConsoleBackground(RGB(229,205,174));

	pokeballs.push_back(getFileContent("pokeball_default.txt"));
	pokeballs.push_back(getFileContent("pokeball_left.txt"));
	pokeballs.push_back(getFileContent("pokeball_default.txt"));
	pokeballs.push_back(getFileContent("pokeball_right.txt"));
	pokeballs.push_back(getFileContent("pokeball_success_0.txt"));
	pokeballs.push_back(getFileContent("pokeball_success_1.txt"));
	pokeballs.push_back(getFileContent("pokeball_success_2.txt"));

	string alphabetInputs;
	string passInputs;
	cout << "\033[38;2;0;0;0m";
	drawDoubleLayerRectangle({static_cast<SHORT>(SCREEN_WIDTH / 3), static_cast<SHORT>(SCREEN_HEIGHT / 1.25)}, SCREEN_WIDTH / 3, 20);
	MoveCursorTo({static_cast<SHORT>(SCREEN_WIDTH / 3 + 10), static_cast<SHORT>(SCREEN_HEIGHT / 1.25 + 2)});
	cout << "\033[38;2;0;0;0m\033[48;2;255;255;255mUser name: " << alphabetInputs;
	MoveCursorTo({static_cast<SHORT>(SCREEN_WIDTH / 3 + 10), static_cast<SHORT>(SCREEN_HEIGHT / 1.25 + 3)});
	cout << "\033[38;2;0;0;0m\033[48;2;255;255;255mPassword: " << alphabetInputs;

	bool inputPass = false;

	bool entered = false;

	thread t(control, &alphabetInputs, &passInputs, &inputPass, &entered);
	t.detach();

//	control(&alphabetInputs, &passInputs, &inputPass, &entered);

	while (true) {
		drawPocketBall();
		if (pokeball_id == 0) pokeball_id = 1;
		else if (pokeball_id == 1) pokeball_id = 2;
		else if (pokeball_id == 2) pokeball_id = 3;
		else if (pokeball_id == 3) pokeball_id = 0;
		if (entered && pokeball_id == 0) pokeball_id = 4;
		if (pokeball_id == 4) pokeball_id = 5;
		else if (pokeball_id == 5) pokeball_id = 6;

		MoveCursorTo({static_cast<SHORT>(SCREEN_WIDTH / 3 + 10), static_cast<SHORT>(SCREEN_HEIGHT / 1.25 + 2)});
		cout << "\033[38;2;0;0;0m\033[48;2;255;255;255mUser name: " << alphabetInputs << "_" + string(49 - alphabetInputs.length(), ' ');
		MoveCursorTo({static_cast<SHORT>(SCREEN_WIDTH / 3 + 10), static_cast<SHORT>(SCREEN_HEIGHT / 1.25 + 3)});
		cout << "\033[38;2;0;0;0m\033[48;2;255;255;255mPassword: " << string(passInputs.length(), '*') << passInputs.back()+ string(49 - passInputs.length(), ' ');
		Sleep(50);
	}
	system("pause");
	return 0;
}
