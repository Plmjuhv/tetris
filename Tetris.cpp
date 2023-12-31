#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <windows.h>

#define UNICODE
#define _UNICODE

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int nScreenWidth = 120;
int nScreenHeight = 30;

int Rotate(int px, int py, int r) {
    switch (r % 4) {
        case 0: return py * 4 + px;             // 0 degree rotation
        case 1: return 12 + py - (px * 4);      // 90 degree rotation
        case 2: return 15 - (py * 4) - px;      // 180 degree rotation
        case 3: return 3 - py + (px * 4);       // 270 degree rotation
    }

    return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            // Get index into piece
            int pieceIndex = Rotate(px, py, nRotation);

            // Get index into field
            int fieldIndex = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pieceIndex] == L'X' && pField[fieldIndex] != 0) {
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

int main() {
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".X..");
    tetromino[3].append(L".X..");

    tetromino[4].append(L"....");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"..X.");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"....");

    tetromino[6].append(L"..X.");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"....");

    pField = new unsigned char[nFieldWidth * nFieldHeight];
    for (int x = 0; x < nFieldWidth; x++) {
        for (int y = 0; y < nFieldHeight; y++) {
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
        }
    }

    wchar_t *screen  = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) {
        screen[i] = L' ';
    }

    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    bool bGameOver = false;

    srand((unsigned int)time(NULL));
    int nCurrentPiece = rand() % 7;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth/2 - 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;

    int nScore = 0;
    int nTetrises = 0;

    vector<int> vLines;

    while (!bGameOver) {
        // Game Timing =================
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeed == nSpeedCounter);

        // Input =======================
        for (int k = 0; k < 4; k++) {
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
        }

        // Game Logic ==================
        if (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
            nCurrentX++;
        }

        if (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
            nCurrentX--;
        }

        if (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
            nCurrentY++;
        }

        if (bKey[3]) {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY) && !bRotateHold) {
                nCurrentRotation++;
            }

            bRotateHold = true;
        } else {
            bRotateHold = false;
        }

        if (bForceDown) {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                nCurrentY++;
            } else {
                // Lock piece onto field
                for (int px = 0; px < 4; px++) {
                    for (int py = 0; py < 4; py++) {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                            pField[(nCurrentY + py) * nFieldWidth + nCurrentX + px] = nCurrentPiece + 1;
                        }
                    }
                }

                nPieceCount++;
                if (nPieceCount % 10 == 0) {
                    if (nSpeed >= 5) {
                        nSpeed--;
                    }
                }

                // Check have we made horizontal lines
                for (int py = 0; py < 4; py++) {
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++) {
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;
                        }

                        if (bLine) {
                            for (int px = 1; px < nFieldWidth - 1; px++) {
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            }

                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }

                nScore += 25 * (1 + 0.2 * (20 - nSpeed));
                if (!vLines.empty()) {
                    nScore += (1 << vLines.size()) * 100* (1 + 0.2 * (20 - nSpeed));
                    nTetrises += (vLines.size() == 4) ? 1 : 0;
                }

                //  Choose next piece
                nCurrentX = nFieldWidth/2 - 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // If next piece doesnt fit
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }

            nSpeedCounter = 0;
        }

        // Render Output ===============

        // Draw Field
        for (int x = 0; x < nFieldWidth; x++) {
            for (int y = 0; y < nFieldHeight; y++) {
                screen[(y + 2) * nScreenWidth + (x + 5)] =  L" ZMNOEHX=\u2588"[pField[y * nFieldWidth + x]];
            }
        }

        // Draw Current Piece
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 5)] = L"ZMNOEHX"[nCurrentPiece];
                }
            }
        }

        // Display Score
        swprintf_s(&screen[1 * nScreenWidth + nFieldHeight + 6], 16, L"LEVEL: %8d", 20 - nSpeed);
        swprintf_s(&screen[3 * nScreenWidth + nFieldHeight + 6], 16, L"SCORE: %8d", nScore);
        swprintf_s(&screen[4 * nScreenWidth + nFieldHeight + 5], 17, L"==================");
        swprintf_s(&screen[5 * nScreenWidth + nFieldHeight + 6], 16, L"TETRISES: %5d", nTetrises);


        if(!vLines.empty()) {
            WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : vLines) {
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--) {
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    }

                    pField[px] = 0;
                }
            }

            vLines.clear();
        }

        // Display Frame
        WriteConsoleOutputCharacterW(hConsole, screen, nScreenWidth * nScreenHeight, {0, 0}, &dwBytesWritten);
    }

    CloseHandle(hConsole);
    cout << "Game Over!! Score:" << nScore << endl;
    system("pause");

    return 0;
}