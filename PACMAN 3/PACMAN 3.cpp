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
constexpr wchar_t snd_file[]{ L".\\res\\snd\\main.wav" };
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

float RIP_x{};
float RIP_y{};

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
std::vector<gamedll::ATOM> vHearts;

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
    vHearts.clear();

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
        for (float dum_x = 10.0f; dum_x < scr_width; dum_x += 40.0f)
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
BOOL CheckRecord()
{
    if (score < 1)return no_record;
    int result = 0;
    CheckFile(rec_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        std::wofstream rec(rec_file);
        rec << score << std::endl;
        for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
        rec.close();
        return first_record;
    }
    else
    {
        std::wifstream check(rec_file);
        check >> result;
        check.close();

        if (score > result)
        {
            std::wofstream rec(rec_file);
            rec << score << std::endl;
            for (int i = 0; i < 16; i++)rec << static_cast<int>(current_player[i]) << std::endl;
            rec.close();
            return record;
        }
    }
    return no_record;
}
void GameOver()
{
    PlaySound(NULL, NULL, NULL);

    switch (CheckRecord())
    {
    case no_record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Beige));
        Draw->DrawTextW(L"ЗАГУБИ ИГРАТА !", 16, bigText, D2D1::RectF(10.0f, 200.0f, scr_width, scr_height), TextBrush);
        Draw->EndDraw();
        if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;

    case first_record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Beige));
        Draw->DrawTextW(L"ПЪРВИ РЕКОРД !", 15, bigText, D2D1::RectF(10.0f, 200.0f, scr_width, scr_height), TextBrush);
        Draw->EndDraw();
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;

    case record:
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::Beige));
        Draw->DrawTextW(L"СВЕТОВЕН РЕКОРД !", 18, bigText, D2D1::RectF(10.0f, 200.0f, scr_width, scr_height), TextBrush);
        Draw->EndDraw();
        if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
        else Sleep(3000);
        break;
    }

    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void LevelUp()
{
    Draw->EndDraw();
    Draw->BeginDraw();
    Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
    if (bigText && InactBrush)Draw->DrawTextW(L"НИВОТО ПРЕМИНАТО !", 19, bigText, D2D1::RectF(5.0f, 200.0f, 
        scr_width, scr_height), InactBrush);
    Draw->EndDraw();
    if (sound)
    {
        PlaySound(NULL, NULL, NULL);
        PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
        PlaySound(snd_file, NULL, SND_ASYNC | SND_LOOP);
    }
    else Sleep(2500);

    ++level;
    speed = level * 0.5f;
    
    vObstacles.clear();
    vCoins.clear();
    vHearts.clear();

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
        for (float dum_x = 10.0f; dum_x < scr_width; dum_x += 40.0f)
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

            if (dummy_ok)vCoins.push_back(dummy);
        }
    }

    PacMan = gamedll::Factory(creatures::pacman, 45.0f, ground - 45.0f);
}
void ShowRecord()
{
    int result{};
    CheckFile(rec_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още няма постигнат рекорд !\n\nПостарай се повече !",
            L"Липсва файл", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    wchar_t rectext[100] = L"Най-добър играч: ";
    wchar_t saved_player[16] = L"\0";
    wchar_t add[5] = L"\0";

    std::wifstream check(rec_file);
    check >> result;
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;
        check >> letter;
        saved_player[i] = static_cast<wchar_t>(letter);
    }
    check.close();

    wcscat_s(rectext, saved_player);
    wcscat_s(rectext, L"\n\nСветовен рекорд: ");
    wsprintf(add, L"%d", result);
    wcscat_s(rectext, add);

    result = 0;
    for (int i = 0; i < 100; i++)
    {
        if (rectext[i] != '\0')result++;
        else break;
    }

    if (Draw && midText && TextBrush)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
        Draw->DrawText(rectext, result, midText, D2D1::RectF(20.0f, 150.0f, scr_width, scr_height), TextBrush);
        Draw->EndDraw();
        if (sound)mciSendString(L"play .\\res\\snd\\showrec.wav", NULL, NULL, NULL);
    }
    Sleep(2500);
}
void SaveGame()
{
    int result{};
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Съществува записана игра, която ще се загуби !\n\nНаистина ли да я презапиша ?",
            L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION) == IDNO)return;
    }

    std::wofstream save(save_file);

    save << score << std::endl;
    save << level << std::endl;
    save << speed << std::endl;
    for (int i = 0; i < 16; i++)save << static_cast<int>(current_player[i]) << std::endl;
    save << name_set << std::endl;

    save << vObstacles.size() << std::endl;
    for (int i = 0; i < vObstacles.size(); i++)
    {
        save << vObstacles[i].x << std::endl;
        save << vObstacles[i].y << std::endl;
    }

    save << vCoins.size() << std::endl;
    if (!vCoins.empty())
        for (int i = 0; i < vCoins.size(); i++)
        {
            save << vCoins[i].x << std::endl;
            save << vCoins[i].y << std::endl;
        }

    save << vHearts.size() << std::endl;
    if (!vHearts.empty())
        for (int i = 0; i < vHearts.size(); i++)
        {
            save << vHearts[i].x << std::endl;
            save << vHearts[i].y << std::endl;
        }

    save << vGhosts.size() << std::endl;
    if (!vGhosts.empty())
        for (int i = 0; i < vGhosts.size(); i++)
        {
            save << vGhosts[i]->x << std::endl;
            save << vGhosts[i]->y << std::endl;
            save << static_cast<int>(vGhosts[i]->GetType()) << std::endl;
        }

    if (!PacMan)save << 0 << std::endl;
    else
    {
        save << 1 << std::endl;
        save << PacMan->x << std::endl;
        save << PacMan->y << std::endl;
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е записана !", L"Съхранение", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
    int result{};
    CheckFile(save_file, &result);
    if (result == FILE_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
        if (MessageBox(bHwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли да я презапиша ?",
            L"Презапис", MB_YESNO | MB_APPLMODAL | MB_ICONEXCLAMATION) == IDNO)return;
    }
    else
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !",
            L"Липсва файл", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    vObstacles.clear();
    vCoins.clear();
    vHearts.clear();

    ClearMem(&PacMan);
    if (!vGhosts.empty())
        for (int i = 0; i < vGhosts.size(); i++)ClearMem(&vGhosts[i]);
    vGhosts.clear();

    result = 0;

    std::wifstream save(save_file);

    save >> score;
    save >> level;
    save >> speed;
    for (int i = 0; i < 16; i++)
    {
        int letter = 0;

        save >> letter;
        current_player[i]=static_cast<wchar_t>(letter);
    }
    save >> name_set;

    save >> result;
    for (int i = 0; i < result; i++)
    {
        float temp_x = 0;
        float temp_y = 0;

        save >> temp_x;
        save >> temp_y;

        vObstacles.push_back(gamedll::ATOM(temp_x, temp_y, 45.0f, 45.0f));
    }

    save >> result;
    if(result>0)
        for (int i = 0; i < result; i++)
        {
            float temp_x = 0;
            float temp_y = 0;

            save >> temp_x;
            save >> temp_y;

            vCoins.push_back(gamedll::ATOM(temp_x, temp_y, 15.0f, 14.0f));
        }

    save >> result;
    if (result > 0)
        for (int i = 0; i < result; i++)
        {
            float temp_x = 0;
            float temp_y = 0;

            save >> temp_x;
            save >> temp_y;

            vHearts.push_back(gamedll::ATOM(temp_x, temp_y, 20.0f, 17.0f));
        }

    save >> result;
    if (result > 0)
        for (int i = 0; i < result; i++)
        {
            float temp_x = 0;
            float temp_y = 0;
            int temp_type = -1;

            save >> temp_x;
            save >> temp_y;
            save >> temp_type;

            vGhosts.push_back(gamedll::Factory(static_cast<creatures>(temp_type), temp_x, temp_y));
            if (vGhosts.back()->GetType() == creatures::hurt)vGhosts.back()->Hurt();
        }
    
    save >> result;
    if (result == 0)GameOver();
    else 
    {
        float temp_x = 0;
        float temp_y = 0;

        save >> temp_x;
        save >> temp_y;

        PacMan = gamedll::Factory(creatures::pacman, temp_x, temp_y);
    }

    save.close();

    if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
    MessageBox(bHwnd, L"Играта е заредана !", L"Съхранение", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void ShowHelp()
{
    int result{};
    CheckFile(help_file, &result);
    if (result == FILE_NOT_EXIST)
    {
        if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
        MessageBox(bHwnd, L"Липсва помощ за играта !\n\nСвържете се с разработчика !",
            L"Липсва файл", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
        return;
    }

    wchar_t help_text[1000] = L"\0";
    
    std::wifstream help(help_file);
    help >> result;
    for (int i = 0; i < result; i++)
    {
        int letter = 0;
        help >> letter;
        help_text[i] = static_cast<wchar_t>(letter);
    }
    help.close();

    if (sound)mciSendString(L"play .\\res\\snd\\help.wav", NULL, NULL, NULL);

    if (Draw && BckgBrush && TextBrush && InactBrush && nrmText)
    {
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkBlue));
        Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), BckgBrush);
        if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, InactBrush);
        else
        {
            if (b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, HgltBrush);
            else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmText, b1Rect, TextBrush);
        }
        if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, HgltBrush);
        else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmText, b2Rect, TextBrush);
        if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, HgltBrush);
        else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, b3Rect, TextBrush);
        Draw->DrawTextW(help_text, result, midText, D2D1::RectF(10.0f, 100.0f, scr_width, scr_height), HgltBrush);
        Draw->EndDraw();
    }
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
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            if (GetDlgItemTextW(hwnd, IDC_NAME, current_player, 16) < 1)
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
        break;
    }

    return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
        
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
            pause = true;
            if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
            if (MessageBox(hwnd, L"Ако почнеш ново ниво, ще загубиш точките от това !\n\nНаистина ли минаваш на следващо ниво ?",
                L"Ново ниво !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            LevelUp();
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;

        case mSave:
            pause = true;
            SaveGame();
            pause = false;
            break;

        case mLoad:
            pause = true;
            LoadGame();
            pause = false;
            break;

        case mHoF:
            pause = true;
            ShowRecord();
            pause = false;
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

    case WM_LBUTTONDOWN:
        if (HIWORD(lParam) <= 50)
        {
            if (LOWORD(lParam) >= b1Rect.left && LOWORD(lParam) <= b1Rect.right)
            {
                if (name_set)
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                    break;
                }
                if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
                break;
            }
            if (LOWORD(lParam) >= b2Rect.left && LOWORD(lParam) <= b2Rect.right)
            {
                mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (sound)
                {
                    sound = false;
                    PlaySound(NULL, NULL, NULL);
                    break;
                }
                else
                {
                    sound = true;
                    PlaySound(snd_file, NULL, SND_ASYNC | SND_LOOP);
                    break;
                }

                break;
            }
            if (LOWORD(lParam) >= b3Rect.left && LOWORD(lParam) <= b3Rect.right)
            {
                if (sound) mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
                if (!show_help)
                {
                    show_help = true;
                    pause = true;
                    ShowHelp();
                    break;
                }
                else
                {
                    show_help = false;
                    pause = false;
                    break;
                }
                break;
            }
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
                DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 28, L"", &midText);
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
                Draw->DrawTextW(L"PACMAN IN ACTION !\n\n\n\n      dev. Daniel", 41, bigText, D2D1::RectF(5.0f, 70.0f,
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
        Draw->DrawTextW(L"PACMAN IN ACTION !\n\n\n\n      dev. Daniel", 41, bigText, D2D1::RectF(5.0f, 70.0f,
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

    PlaySound(snd_file, NULL, SND_ASYNC | SND_LOOP);

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
            
            switch (RandGenerator(0, 3))
            {
            case 0:
                temp_y = sky;
                break;

            case 1:
                temp_y = ground - 40.0f;
                break;

            case 2:
                temp_x = 2;
                temp_y = (float)(RandGenerator((int)(sky), (int)(ground - 100.0f)));
                break;

            case 3:
                temp_x = scr_width-50.0f;
                temp_y = (float)(RandGenerator((int)(sky), (int)(ground - 100.0f)));
                break;
            }
            
            vGhosts.push_back(gamedll::Factory(static_cast<creatures>(temp_type), temp_x, temp_y));
            vGhosts.back()->dir = static_cast<dirs>(temp_dir);
        }
        
        if (!vGhosts.empty() && PacMan)
        {
            if (RandGenerator(0, 100) == 6)
            {
                for (std::vector<gamedll::Creature>::iterator evil = vGhosts.begin(); evil < vGhosts.end(); ++evil)
                {
                    switch (RandGenerator(0, 1))
                    {
                    case 0:
                        if (PacMan->x < (*evil)->x && !(*evil)->GetFlag(left_flag))(*evil)->dir = dirs::left;
                        else if (!(*evil)->GetFlag(right_flag)) (*evil)->dir = dirs::right;
                        break;

                    case 1:
                        if (PacMan->y < (*evil)->y && !(*evil)->GetFlag(up_flag))(*evil)->dir = dirs::up;
                        else if (!(*evil)->GetFlag(down_flag))(*evil)->dir = dirs::down;
                        break;
                    }
                }
            }
        }
        
        if (!vGhosts.empty() && !vObstacles.empty())
        {
            gamedll::ATOMPACK ObstPack((int)(vObstacles.size()));

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

        if (PacMan && !vCoins.empty())
        {
            for (std::vector<gamedll::ATOM>::iterator coin = vCoins.begin(); coin < vCoins.end(); coin++)
            {
                if (!(PacMan->x >= coin->ex || PacMan->ex <= coin->x || PacMan->y >= coin->ey || PacMan->ey <= coin->y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\coin.wav", NULL, NULL, NULL);
                    score += 10 + level;
                    vCoins.erase(coin);
                    break;
                }
            }
        }

        if (vHearts.size() < 3 && RandGenerator(0, 500) == 66 && !vObstacles.empty())
        {
            bool place_found = false;
            while (!place_found)
            {
                place_found = true;
                
                gamedll::ATOM DummyHeart((float)(RandGenerator(0, (int)(scr_width - 25.0f))),
                    (float)(RandGenerator((int)(sky + 10.0f), (int)(ground - 20.0f))), 20.0f, 17.0f);

                for (std::vector<gamedll::ATOM>::iterator brick = vObstacles.begin(); brick < vObstacles.end(); brick++)
                {
                    if (!(DummyHeart.x >= brick->ex || DummyHeart.ex < brick->x
                        || DummyHeart.y >= brick->ey || DummyHeart.ey < brick->y))
                    {
                        place_found = false;
                        break;
                    }
                }
                if (place_found)vHearts.push_back(DummyHeart);
            }
        }

        if (PacMan && !vHearts.empty())
        {
            for (std::vector<gamedll::ATOM>::iterator heart = vHearts.begin(); heart < vHearts.end(); heart++)
            {
                if (!(PacMan->x >= heart->ex || PacMan->ex <= heart->x || PacMan->y >= heart->ey || PacMan->ey <= heart->y))
                {
                    if (sound)mciSendString(L"play .\\res\\snd\\life.wav", NULL, NULL, NULL);
                    score += 20 + level;
                    vHearts.erase(heart);
                    if (!vGhosts.empty())
                        for (int i = 0; i < vGhosts.size(); i++)
                        {
                            if (!vGhosts[i]->panic)vGhosts[i]->Hurt();
                        }

                    break;
                }
            }
        }

        if (!vGhosts.empty())
        {
            for (int i = 0; i < vGhosts.size(); i++)
            {
                if (vGhosts[i]->panic)vGhosts[i]->Hurt();
            }
        }
        if (!vGhosts.empty() && PacMan)
        {
            for (std::vector<gamedll::Creature>::iterator evil = vGhosts.begin(); evil < vGhosts.end(); evil++)
            {
                if (!(PacMan->x > (*evil)->ex || PacMan->ex<(*evil)->x || PacMan->y >(*evil)->ey || PacMan->ey < (*evil)->y))
                {
                    if ((*evil)->GetType() != creatures::hurt)
                    {

                        RIP_x = PacMan->x;
                        RIP_y = PacMan->y;
                        ClearMem(&PacMan);
                        break;
                    }
                    else
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\eaten.wav", NULL, NULL, NULL);
                        score += 50 + level;
                        (*evil)->Release();
                        vGhosts.erase(evil);
                        break;
                    }
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
        else LevelUp();

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

                case creatures::hurt:
                    Draw->DrawBitmap(bmpHurtEvil[(*evil)->GetFrame()], D2D1::RectF((*evil)->x, (*evil)->y,
                        (*evil)->ex, (*evil)->ey));
                    break;
                }
            }
        }

        if (!vHearts.empty())
        {
            for (std::vector<gamedll::ATOM>::iterator heart = vHearts.begin(); heart < vHearts.end(); heart++)
                Draw->DrawBitmap(bmpLife, D2D1::RectF(heart->x, heart->y, heart->ex, heart->ey));
        }
        
        if (nrmText && InactBrush)
        {
            wchar_t stat_text[100] = L"играч: ";
            wchar_t add[10] = L"\0";
            int size = 0;

            wcscat_s(stat_text, current_player);

            wcscat_s(stat_text, L", резултат: ");
            wsprintf(add, L"%d", score);
            wcscat_s(stat_text, add);

            wcscat_s(stat_text, L", ниво: ");
            wsprintf(add, L"%d", level);
            wcscat_s(stat_text, add);

            for (int i = 0; i < 100; i++)
            {
                if (stat_text[i] != '\0')size++;
                else break;
            }

            Draw->DrawTextW(stat_text, size, nrmText, D2D1::RectF(10.0f, ground + 10.0f, scr_width, scr_height), InactBrush);
        }

        if (RIP_x > 0 && RIP_y > 0)
        {
            Draw->DrawBitmap(bmpRIP, D2D1::RectF(RIP_x, RIP_y, RIP_x + 80.0f, RIP_y + 94.0f));
            Draw->EndDraw();
            if (sound)
            {
                PlaySound(NULL, NULL, NULL);
                PlaySound(L".\\res\\snd\\killed.wav", NULL, SND_SYNC);
            }
            else(Sleep(2000));
            GameOver();
        }

        
        /////////////////////////////////
        Draw->EndDraw();
    }

    ReleaseResources();
    std::remove(tmp_file);
    return (int) bMsg.wParam;
}