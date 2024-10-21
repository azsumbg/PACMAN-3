#include "framework.h"
#include "PACMAN 3.h"
#include <mmsystem.h>
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "pacengine.h"
#include <d2d1.h>
#include <dwrite.h>
#include <vector>
#include <fstream>
#include <chrono>
#include <random>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")
#pragma comment (lib, "pacengine.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")

constexpr wchar_t bWinClassName[]{ L"MyNewPacMan" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t snd_file[]{ L".\\res\\snd\\main.dat" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t rec_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int no_record{ 2003 };

WNDCLASS bWin{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HICON Icon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
HDC PaintDC{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
MSG bMsg{};
BOOL bRet{ -1 };
PAINTSTRUCT bPaint{};
UINT bTimer = -1 ;
POINT cur_pos{};

D2D1_RECT_F b1Rect{ 10.0f, 10.0f, scr_width / 3 - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3 + 10.0f, 10.0f, scr_width * 2 / 3 - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2 / 3 + 10.0f, 10.0f, scr_width, 50.0f };

bool in_client = true;
bool pause = false;
bool sound = true;
bool show_help = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;
bool name_set = false;

wchar_t current_player[16]{ L"ONE PACMAN" };

int score = 0;
int level = 1;
float speed{};

int field_frame = 0;

////////////////////////////////

gamedll::RANDENGINE RandGenerator{};

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

ID2D1RadialGradientBrush* BckgBrush = nullptr;
ID2D1SolidColorBrush* TextBrush = nullptr;
ID2D1SolidColorBrush* HgltBrush = nullptr;
ID2D1SolidColorBrush* InactBrush = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* midText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1Bitmap* bmpField[51]{ nullptr };
ID2D1Bitmap* bmpBrick = nullptr;
ID2D1Bitmap* bmpCoin = nullptr;
ID2D1Bitmap* bmpLife = nullptr;
ID2D1Bitmap* bmpIntro = nullptr;
ID2D1Bitmap* bmpRIP = nullptr;

ID2D1Bitmap* bmpPackManD[2]{ nullptr };
ID2D1Bitmap* bmpPackManU[2]{ nullptr };
ID2D1Bitmap* bmpPackManL[2]{ nullptr };
ID2D1Bitmap* bmpPackManR[2]{ nullptr };

ID2D1Bitmap* bmpBlueEvil[2]{ nullptr };
ID2D1Bitmap* bmpRedEvil[2]{ nullptr };
ID2D1Bitmap* bmpOrangeEvil[2]{ nullptr };
ID2D1Bitmap* bmpPinkEvil[2]{ nullptr };
ID2D1Bitmap* bmpHurtEvil[11]{ nullptr };

////////////////////////////////

std::vector<gamedll::ATOM>vObstacles;
std::vector<gamedll::ATOM>vCoins;

gamedll::Creature PacMan{ nullptr };
std::vector<gamedll::Creature>vGhosts;

////////////////////////////////

template<typename T> concept CanBeReleased = requires (T what)
{
    what.Release();
};
template<CanBeReleased T> bool ClearMem(T** what)
{
    if (*what)
    {
        (*what)->Release();
        (*what) = nullptr;
        return true;
    }
    return false;
}
void LogError(LPCWSTR what)
{
    std::wofstream err(L".\\res\\data\\error.log", std::ios::app);
    err << what << L" Time stamp: " << std::chrono::system_clock::now() << std::endl;
    err.close();
}
void ReleaseResources()
{
    if (!ClearMem(&iFactory))LogError(L"Error releasing iFactory !");
    if (!ClearMem(&iWriteFactory))LogError(L"Error releasing iWriteFactory !");
    if (!ClearMem(&BckgBrush))LogError(L"Error releasing BckgBrush !");
    if (!ClearMem(&TextBrush))LogError(L"Error releasing TextBrush !");
    if (!ClearMem(&HgltBrush))LogError(L"Error releasing HgltBrush !");
    if (!ClearMem(&InactBrush))LogError(L"Error releasing InactBrush !");
    if (!ClearMem(&nrmText))LogError(L"Error releasing nrmText !");
    if (!ClearMem(&midText))LogError(L"Error releasing midText !");
    if (!ClearMem(&bigText))LogError(L"Error releasing bigText !");
    if (!ClearMem(&Draw))LogError(L"Error releasing HwndRenderTarget !");

    if (!ClearMem(&bmpBrick))LogError(L"Error releasing bmpBrick !");
    if (!ClearMem(&bmpCoin))LogError(L"Error releasing bmpCoin !");
    if (!ClearMem(&bmpLife))LogError(L"Error releasing bmpLife !");
    if (!ClearMem(&bmpIntro))LogError(L"Error releasing bmpIntro !");
    if (!ClearMem(&bmpRIP))LogError(L"Error releasing bmpRIP !");

    for (int i = 0; i < 51; i++)if (!ClearMem(&bmpField[i]))LogError(L"Error releasing bmpField !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpPackManD[i]))LogError(L"Error releasing bmpPackManD !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpPackManU[i]))LogError(L"Error releasing bmpPackManU !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpPackManL[i]))LogError(L"Error releasing bmpPackManL !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpPackManR[i]))LogError(L"Error releasing bmpPackManR !");

    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpBlueEvil[i]))LogError(L"Error releasing bmpBlueEvil !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpRedEvil[i]))LogError(L"Error releasing bmpRedEvil !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpOrangeEvil[i]))LogError(L"Error releasing bmpOrangeEvil !");
    for (int i = 0; i < 2; i++)if (!ClearMem(&bmpPinkEvil[i]))LogError(L"Error releasing bmpPinkEvil !");
    for (int i = 0; i < 11; i++)if (!ClearMem(&bmpHurtEvil[i]))LogError(L"Error releasing bmpHurtEvil !");
}
void ErrExit(int what)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(what), L"КРИТИЧНА ГРЕШКА !", MB_OK | MB_APPLMODAL | MB_ICONERROR);

    ReleaseResources();
    std::remove(tmp_file);
    exit(1);
}
void InitGame()
{
    level = 1;
    speed = level * 0.5f;
    score = 0;
    wcscpy_s(current_player, L"ONE PACMAN");
    name_set = false;

    vObstacles.clear();
    vCoins.clear();

    ClearMem(&PacMan);
    if (!vGhosts.empty())
        for (int i = 0; i < vGhosts.size(); i++)ClearMem(&vGhosts[i]);
    vGhosts.clear();

    for (float dum_x = 50.0f; dum_x < scr_width - 50.0f; dum_x += 50.0f)
    {
        for (float dum_y = 100.0f; dum_y < ground - 50.0f; dum_y += 100.0f)
        {
            if (RandGenerator(0, 1) == 1)vObstacles.push_back(gamedll::ATOM(dum_x, dum_y, 45.0f, 45.0f));
        }
    }
    for (float dum_y = 70.0f; dum_y < ground - 50.0f; dum_y += 50.0f)
    {
        for (float dum_x = 10.0f; dum_x < scr_width; dum_x += 20.0f)
        {
            gamedll::ATOM dummy(dum_x, dum_y, 15.0f, 14.0f);
            bool dummy_ok = true;

            for (int i = 0; i < vObstacles.size(); i++)
            {
                if (!(dummy.x >= vObstacles[i].ex || dummy.ex <= vObstacles[i].x
                    || dummy.y >= vObstacles[i].ey || dummy.ey <= vObstacles[i].y))
                {
                    dummy_ok = false;
                    break;
                }
            }
            
            if(dummy_ok)vCoins.push_back(dummy);
        }
    }

    PacMan = gamedll::Factory(creatures::pacman, 45.0f, ground - 45.0f);
}

void GameOver()
{
    KillTimer(bHwnd, bTimer);
    PlaySound(NULL, NULL, NULL);

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(Icon));
        return true;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
        {
            wcscpy_s(current_player, L"ONE PACMAN");
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            MessageBox(bHwnd, L"Името си ли забрави ?", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
            EndDialog(hwnd, IDCANCEL);
            break;
        }
        EndDialog(hwnd, IDOK);
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        SetTimer(hwnd, bTimer, 1000, NULL);
        
        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();
        
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bMain, L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bStore, L"Меню за данни");
        
        AppendMenu(bMain, MF_POPUP, mNew, L"Нова игра");
        AppendMenu(bMain, MF_POPUP, mLvl, L"Следващо ниво"); 
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_POPUP, mExit, L"Изход");

        AppendMenu(bStore, MF_POPUP, mSave, L"Запази игра");
        AppendMenu(bStore, MF_POPUP, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_POPUP, mHoF, L"Зала на славата");
        SetMenu(hwnd, bBar);
        InitGame();
        break;

    case WM_CLOSE:
        pause = true;
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(hwnd, L"Ако излезеш, тази игра ще бъде загубена !\n\nНаистина ли излизаш ?",
            L"Изход ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(50, 50, 50)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cur_pos);
        ScreenToClient(hwnd, &cur_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }
            if (cur_pos.y <= 50)
            {
                if (cur_pos.x >= b1Rect.left && cur_pos.x <= b1Rect.right)
                {
                    if (!b1Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b1Hglt = true;
                        b2Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b2Rect.left && cur_pos.x <= b2Rect.right)
                {
                    if (!b2Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b2Hglt = true;
                        b1Hglt = false;
                        b3Hglt = false;
                    }
                }
                if (cur_pos.x >= b3Rect.left && cur_pos.x <= b3Rect.right)
                {
                    if (!b3Hglt)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                        b3Hglt = true;
                        b1Hglt = false;
                        b2Hglt = false;
                    }
                }
                SetCursor(outCur);
                return true;
            }
            else if (b1Hglt || b2Hglt || b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(mainCur);
            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }
            if (b1Hglt || b2Hglt|| b3Hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1Hglt = false;
                b2Hglt = false;
                b3Hglt = false;
            }
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            return true;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case mNew:
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако рестартираш, тази игра ще бъде загубена !\n\nНаистина ли рестартираш ?",
                L"Рестарт ?", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            break;

        case mLvl:

            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        }
        break;

    case WM_KEYDOWN:
        if (!PacMan)break;
        switch (LOWORD(wParam))
        {
        case VK_LEFT:
            PacMan->dir = dirs::left;
            break;

        case VK_RIGHT:
            PacMan->dir = dirs::right;
            break;

        case VK_UP:
            PacMan->dir = dirs::up;
            break;

        case VK_DOWN:
            PacMan->dir = dirs::down;
            break;
        }
        break;

    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)(FALSE);
}

void CreateResources()
{
    std::wifstream check(Ltmp_file);
    if (check.good())
    {
        check.close();
        ErrExit(eStarted);
    }
    else
    {
        check.close();
        std::wofstream start(Ltmp_file);
        start << L"Game started at: " << std::chrono::system_clock::now();
        start.close();
    }
    
    int w_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2);
    if (GetSystemMetrics(SM_CXSCREEN) < w_x + scr_width || GetSystemMetrics(SM_CYSCREEN) < scr_height + 20)ErrExit(eScreen);

    Icon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", ICON_BIG, 128, 128, LR_LOADFROMFILE));
    if (!Icon)ErrExit(eIcon);
    mainCur = LoadCursorFromFile(L".\\res\\main.ani");
    outCur = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCur || !outCur)ErrExit(eCursor);

    bWin.lpszClassName = bWinClassName;
    bWin.hInstance = bIns;
    bWin.lpfnWndProc = &WinProc;
    bWin.style = CS_DROPSHADOW;
    bWin.hbrBackground = CreateSolidBrush(RGB(50, 50, 50));
    bWin.hIcon = Icon;
    bWin.hCursor = mainCur;

    if (!RegisterClass(&bWin))ErrExit(eClass);

    bHwnd = CreateWindowW(bWinClassName, L"ГЛАДНИК В ДЕЙСТВИЕ", WS_CAPTION | WS_SYSMENU, w_x, 20, (int)(scr_width),
        (int)(scr_height), NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else
    {
        ShowWindow(bHwnd, SW_SHOWDEFAULT);

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
        if (hr != S_OK)
        {
            LogError(L"Error creating iFactory !");
            ErrExit(eD2D);
        }

        if (iFactory)
        {
            hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
                D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
            if (hr != S_OK)
            {
                LogError(L"Error creating HwndRenderTarget !");
                ErrExit(eD2D);
            }

            if (Draw)
            {
                D2D1_GRADIENT_STOP Stops[2]{};
                ID2D1GradientStopCollection* Coll = nullptr;

                Stops[0].position = 0;
                Stops[0].color = D2D1::ColorF(D2D1::ColorF::DarkViolet);
                Stops[1].position = 1.0f;
                Stops[1].color = D2D1::ColorF(D2D1::ColorF::Firebrick);

                hr = Draw->CreateGradientStopCollection(Stops, 2, &Coll);
                if (hr != S_OK)
                {
                    LogError(L"Error creating GradientStopCollection !");
                    ErrExit(eD2D);
                }

                if (Coll)
                {
                    hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(scr_width / 2, 25.0f),
                        D2D1::Point2F(0, 0), scr_width / 2, 25.0f), Coll, &BckgBrush);
                    if (hr != S_OK)
                    {
                        LogError(L"Error creating Background Brush !");
                        ErrExit(eD2D);
                    }
                    ClearMem(&Coll);
                }

                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &TextBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating TextBrush !");
                    ErrExit(eD2D);
                }
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::YellowGreen), &HgltBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating HgltTextBrush !");
                    ErrExit(eD2D);
                }
                hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::FloralWhite), &InactBrush);
                if (hr != S_OK)
                {
                    LogError(L"Error creating InactTextBrush !");
                    ErrExit(eD2D);
                }

                bmpBrick = Load(L".\\res\\img\\field\\brick.png", Draw);
                if (!bmpBrick)
                {
                    LogError(L"Error creating bmpBrick !");
                    ErrExit(eD2D);
                }

                bmpCoin = Load(L".\\res\\img\\field\\coin.png", Draw);
                if (!bmpCoin)
                {
                    LogError(L"Error creating bmpCoin !");
                    ErrExit(eD2D);
                }

                bmpLife = Load(L".\\res\\img\\field\\life.png", Draw);
                if (!bmpLife)
                {
                    LogError(L"Error creating bmpLife !");
                    ErrExit(eD2D);
                }

                bmpRIP = Load(L".\\res\\img\\pacman\\rip.png", Draw);
                if (!bmpRIP)
                {
                    LogError(L"Error creating bmpRIP !");
                    ErrExit(eD2D);
                }

                bmpIntro = Load(L".\\res\\img\\field\\intro.png", Draw);
                if (!bmpIntro)
                {
                    LogError(L"Error creating bmpIntro !");
                    ErrExit(eD2D);
                }

                for (int i = 0; i < 51; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\field\\background\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpField[i] = Load(name, Draw);
                    if (!bmpField[i])
                    {
                        LogError(L"Error creating bmpField !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\pacman\\down\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpPackManD[i] = Load(name, Draw);
                    if (!bmpPackManD[i])
                    {
                        LogError(L"Error creating bmpPacManD !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\pacman\\up\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpPackManU[i] = Load(name, Draw);
                    if (!bmpPackManU[i])
                    {
                        LogError(L"Error creating bmpPacManU !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\pacman\\left\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpPackManL[i] = Load(name, Draw);
                    if (!bmpPackManL[i])
                    {
                        LogError(L"Error creating bmpPacManL !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\pacman\\right\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpPackManR[i] = Load(name, Draw);
                    if (!bmpPackManR[i])
                    {
                        LogError(L"Error creating bmpPacManR !");
                        ErrExit(eD2D);
                    }
                }

                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\evils\\1\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpBlueEvil[i] = Load(name, Draw);
                    if (!bmpBlueEvil[i])
                    {
                        LogError(L"Error creating bmpBlueEvil !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\evils\\2\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpRedEvil[i] = Load(name, Draw);
                    if (!bmpRedEvil[i])
                    {
                        LogError(L"Error creating bmpRedEvil !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\evils\\3\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpPinkEvil[i] = Load(name, Draw);
                    if (!bmpPinkEvil[i])
                    {
                        LogError(L"Error creating bmpPinkEvil !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 2; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\evils\\4\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpOrangeEvil[i] = Load(name, Draw);
                    if (!bmpOrangeEvil[i])
                    {
                        LogError(L"Error creating bmpOrangeEvil !");
                        ErrExit(eD2D);
                    }
                }
                for (int i = 0; i < 11; i++)
                {
                    wchar_t name[100] = L".\\res\\img\\evils\\hurt\\";
                    wchar_t add[5] = L"\0";
                    wsprintf(add, L"%d", i);
                    wcscat_s(name, add);
                    wcscat_s(name, L".png");
                    bmpHurtEvil[i] = Load(name, Draw);
                    if (!bmpHurtEvil[i])
                    {
                        LogError(L"Error creating bmpHurtEvil !");
                        ErrExit(eD2D);
                    }
                }
            }
        }

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
        if (hr != S_OK)
        {
            LogError(L"Error creating iWriteFactory !");
            ErrExit(eD2D);
        }

        if (iWriteFactory)
        {
            hr = iWriteFactory->CreateTextFormat(L"GNABRI", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 18, L"", &nrmText);
            if (hr != S_OK)
            {
                LogError(L"Error creating nrmTextFormat !");
                ErrExit(eD2D);
            }

            hr = iWriteFactory->CreateTextFormat(L"GNABRI", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 36, L"", &midText);
            if (hr != S_OK)
            {
                LogError(L"Error creating midTextFormat !");
                ErrExit(eD2D);
            }

            hr = iWriteFactory->CreateTextFormat(L"GNABRI", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 72, L"", &bigText);
            if (hr != S_OK)
            {
                LogError(L"Error creating bigTextFormat !");
                ErrExit(eD2D);
            }
        }
    }

    if (Draw && HgltBrush && bigText)
    {
        PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_ASYNC);
        for (int i = 0; i <= 200; i++)
        {
            Draw->BeginDraw();
            Draw->DrawBitmap(bmpIntro, D2D1::RectF(0, 0, scr_width, scr_height));
            if (RandGenerator(0, 10) == 5)
            {
                Draw->DrawTextW(L"PACMAN IN ACTION !\n\n\n\n\ndev. Daniel", 35, bigText, D2D1::RectF(5.0f, 70.0f,
                    scr_width, scr_height), HgltBrush);
                mciSendString(L"play .\\res\\snd\\buzz.wav", NULL, NULL, NULL);
                Draw->EndDraw();
                Sleep(100);
            }
            else Draw->EndDraw();
        }
    }
    PlaySound(L".\\res\\snd\\boom.wav", NULL, SND_ASYNC);
    if (Draw && HgltBrush && bigText)
    {
        Draw->BeginDraw();
        Draw->DrawBitmap(bmpIntro, D2D1::RectF(0, 0, scr_width, scr_height));
        Draw->DrawTextW(L"PACMAN IN ACTION !\n\n\n\n\ndev. Daniel", 35, bigText, D2D1::RectF(5.0f, 70.0f,
            scr_width, scr_height), HgltBrush);
        Draw->EndDraw();
    }
    Sleep(1500);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)ErrExit(eClass);
    CreateResources();

    while (bMsg.message != WM_QUIT)
    {
        if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
        {
            if (bRet == -1)ErrExit(eMsg);
            TranslateMessage(&bMsg);
            DispatchMessage(&bMsg);
        }
        if (pause)
        {
            if (show_help)continue;
            if (Draw && TextBrush && bigText)
            {
                Draw->BeginDraw();
                Draw->Clear(D2D1::ColorF(D2D1::ColorF::FloralWhite));
                Draw->DrawText(L"ПАУЗА", 6, bigText, D2D1::RectF(scr_width / 2 - 100.0f, scr_height / 2 - 50.0f, scr_width, scr_height), 
                    TextBrush);
                Draw->EndDraw();
                continue;
            }
        }
        ///////////////////////////////

        if (PacMan && !vObstacles.empty())
        {
            gamedll::ATOMPACK LevelObstacles((int)(vObstacles.size()));
            for (int i = 0; i < (int)(vObstacles.size()); i++) LevelObstacles.push_back(vObstacles[i]);

            PacMan->Move(speed, PacMan->dir, dirs::stop, LevelObstacles);
        }
        
        if (vGhosts.size() < 4 + level)
        {
            float temp_x = (float)(RandGenerator(200, (int)(scr_width - 50.0f)));
            float temp_y{};
            int temp_type = RandGenerator(0, 3);
            int temp_dir = RandGenerator(0, 3);
            
            if (RandGenerator(0, 2) == 1)temp_y = sky;
            else temp_y = ground - 40.0f;
           
            vGhosts.push_back(gamedll::Factory(static_cast<creatures>(temp_type), temp_x, temp_y));
            vGhosts.back()->dir = static_cast<dirs>(temp_dir);
        }
        
        if (!vGhosts.empty() && !vObstacles.empty())
        {
            gamedll::ATOMPACK ObstPack(vObstacles.size());

            for (int i = 0; i < vObstacles.size(); i++)ObstPack.push_back(vObstacles[i]);

            for (std::vector<gamedll::Creature>::iterator evil = vGhosts.begin(); evil < vGhosts.end(); evil++)
            {
                int temp_dir = RandGenerator(0, 2);

                switch ((*evil)->dir)
                {
                case dirs::up:
                    if (temp_dir == 0) (*evil)->Move(speed, dirs::up, dirs::down, ObstPack);
                    else if (temp_dir == 1) (*evil)->Move(speed, dirs::up, dirs::left, ObstPack);
                    else (*evil)->Move(speed, dirs::up, dirs::right, ObstPack);
                    break;

                case dirs::down:
                    if (temp_dir == 0) (*evil)->Move(speed, dirs::down, dirs::up, ObstPack);
                    else if (temp_dir == 1) (*evil)->Move(speed, dirs::down, dirs::left, ObstPack);
                    else (*evil)->Move(speed, dirs::down, dirs::right, ObstPack);
                    break;

                case dirs::left:
                    if (temp_dir == 0) (*evil)->Move(speed, dirs::left, dirs::right, ObstPack);
                    else if (temp_dir == 1) (*evil)->Move(speed, dirs::left, dirs::up, ObstPack);
                    else (*evil)->Move(speed, dirs::left, dirs::down, ObstPack);
                    break;

                case dirs::right:
                    if (temp_dir == 0) (*evil)->Move(speed, dirs::right, dirs::left, ObstPack);
                    else if (temp_dir == 1) (*evil)->Move(speed, dirs::right, dirs::up, ObstPack);
                    else (*evil)->Move(speed, dirs::right, dirs::down, ObstPack);
                    break;
                }
            }
        }


        //DRAW THINGS *****************

        if (Draw && BckgBrush && TextBrush && InactBrush && nrmText)
        {
            Draw->BeginDraw();
            Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), BckgBrush);
            if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, InactBrush);
            else
            {
                if(b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, HgltBrush);
                else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, TextBrush);
            }
            if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, HgltBrush);
            else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, TextBrush);
            if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, HgltBrush);
            else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, TextBrush);
            Draw->DrawBitmap(bmpField[field_frame], D2D1::RectF(0, 50.0f, scr_width, scr_height));
            ++field_frame;
            if (field_frame > 50)field_frame = 0;
        }
        if (!vObstacles.empty())
        {
            for (int i = 0; i < vObstacles.size(); i++)
                Draw->DrawBitmap(bmpBrick, D2D1::RectF(vObstacles[i].x, vObstacles[i].y, vObstacles[i].ex, vObstacles[i].ey));
        }
        if (!vCoins.empty())
        {
            for (int i = 0; i < vCoins.size(); i++)
                Draw->DrawBitmap(bmpCoin, D2D1::RectF(vCoins[i].x, vCoins[i].y, vCoins[i].ex, vCoins[i].ey));
        }

        if (PacMan)
        {
            switch (PacMan->dir)
            {
            case dirs::left:
                Draw->DrawBitmap(bmpPackManL[PacMan->GetFrame()], D2D1::RectF(PacMan->x, PacMan->y, PacMan->ex, PacMan->ey));
                break;

            case dirs::right:
                Draw->DrawBitmap(bmpPackManR[PacMan->GetFrame()], D2D1::RectF(PacMan->x, PacMan->y, PacMan->ex, PacMan->ey));
                break;

            case dirs::up:
                Draw->DrawBitmap(bmpPackManU[PacMan->GetFrame()], D2D1::RectF(PacMan->x, PacMan->y, PacMan->ex, PacMan->ey));
                break;

            case dirs::down:
                Draw->DrawBitmap(bmpPackManD[PacMan->GetFrame()], D2D1::RectF(PacMan->x, PacMan->y, PacMan->ex, PacMan->ey));
                break;

            case dirs::stop:
                Draw->DrawBitmap(bmpPackManR[PacMan->GetFrame()], D2D1::RectF(PacMan->x, PacMan->y, PacMan->ex, PacMan->ey));
                break;
            }
        }
        if (!vGhosts.empty())
        {
            for (std::vector<gamedll::Creature>::iterator evil = vGhosts.begin(); evil < vGhosts.end(); evil++)
            {
                switch ((*evil)->GetType())
                {
                case creatures::blue:
                    Draw->DrawBitmap(bmpBlueEvil[(*evil)->GetFrame()], D2D1::RectF((*evil)->x, (*evil)->y,
                        (*evil)->ex, (*evil)->ey));
                    break;

                case creatures::red:
                    Draw->DrawBitmap(bmpRedEvil[(*evil)->GetFrame()], D2D1::RectF((*evil)->x, (*evil)->y,
                        (*evil)->ex, (*evil)->ey));
                    break;

                case creatures::orange:
                    Draw->DrawBitmap(bmpOrangeEvil[(*evil)->GetFrame()], D2D1::RectF((*evil)->x, (*evil)->y,
                        (*evil)->ex, (*evil)->ey));
                    break;

                case creatures::pink:
                    Draw->DrawBitmap(bmpPinkEvil[(*evil)->GetFrame()], D2D1::RectF((*evil)->x, (*evil)->y,
                        (*evil)->ex, (*evil)->ey));
                    break;
                }
            }
        }


        /////////////////////////////////
        Draw->EndDraw();
    }

    ReleaseResources();
    std::remove(tmp_file);
    return (int) bMsg.wParam;
}