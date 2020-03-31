// Tetris.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <fstream>
#include <Windows.h>
using namespace std;

std::wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* playingField = nullptr; //store elements of field as array of unsigned chars(dynamically)

// Console screen (Windows command prompt default)
int nScreenWidth = 80;
int nScreenHeight = 30;

//index after rotating
int Rotate(int px, int py, int r) {
    switch (r % 4) {
    case 0: return py * 4 + px; //0 degrees
    case 1: return 12 + py - (px * 4); //90 degrees
    case 2: return 15 - (py * 4) - px; //180 degrees
    case 3: return 3 - py + (px * 4); //270 degrees
    }

    return 0;
}
bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY) {

    for(int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++) {
            // Get index into piece
            int pi = Rotate(px, py, nRotation);

            // Get index into field
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pi] == L'X' && playingField[fi] != 0)
                        return false; // fail if piece ix equal to X and playing field != 0
                }
            }
        }



    return true;
}

int main()
{
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    //Create assets
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
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");


    playingField = new unsigned char[nFieldWidth * nFieldHeight]; // Create play field buffer
    for (int x = 0; x < nFieldWidth; x++) //Board Boundary
        for (int y = 0; y < nFieldHeight; y++)
            playingField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

    

    // Game Logic Stuff
    bool bGameOver = false;

    int nCurrentPiece = 0;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0; // when speedcoutner = speed then force piece down
    bool bForceDown = false;
    int nPieceCount = 0;

    int nScore = 0;
    int highScore;
    std::vector<int> vLines;

    std::fstream scoreFile("score.txt");
    if (!scoreFile) {
        std::cout << "Error displaying high score.\n";
    }
    scoreFile >> highScore;
    scoreFile.close();
    remove("score.txt");
    
    while (!bGameOver) {

        // GAME TIMING ==================================
        std::this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed); //if counter == speed then forcedown

        // INPUT =======================================
        for (int k = 0; k < 4; k++)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;


        // GAME LOGIC ==================================
        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        // Rotate
        if (bKey[3]) {
            nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else
            bRotateHold = false;

        if (bForceDown) {

            // Test if piece can be moved down
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++; //it can, so do it
            else {
                // If cant, lock the current piece into the field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            playingField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                //Check have we created any full horizonatal lines
                for (int py = 0; py < 4; py++) 
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (playingField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine) {
                            // Remove Line, set to =
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                playingField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            vLines.push_back(nCurrentY + py);
                        }
                    }
                nScore += 25;
                if (!vLines.empty())
                    nScore += (1 << vLines.size()) * 100;

                // Choose next piece. reset position and choose random piece
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                // if piece doesnt fit then game over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }
        

        // RENDER OUTPUT ================================
        // Draw Field
        for (int x = 0; x < nFieldWidth; x++)
            for (int y = 0; y < nFieldHeight; y++)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[playingField[y * nFieldWidth + x]]; //offset by 2 to move game to right a bit out of left hand corner

        // Draw Current Piece
        for (int px = 0; px < 4; px++)
            for (int py = 0; py < 4; py++)
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        if (!vLines.empty()) {
            // Display Frame
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms); // Delay a bit

            for(auto &v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--)
                        playingField[py * nFieldWidth + px] = playingField[(py - 1) * nFieldWidth + px];
                    playingField[px] = 0;
                }
            vLines.clear();
        }

        // Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
    }

    CloseHandle(hConsole);
    if (nScore > highScore) {
        highScore = nScore;
        std::cout << "NEW HIGH SCORE!!!\n";
    }
    std::ofstream toFile("score.txt");
    toFile << highScore << std::endl;
    std::cout << "***HIGH SCORE: " << highScore << "***\n\n";
    std::cout << "YOUR SCORE: " << nScore << std::endl;
    std::cout << "Game over, you suck ass.\n";
    system("pause");

    return 0;

}

