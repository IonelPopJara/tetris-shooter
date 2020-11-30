#include <iostream>
#include <thread>
#include <vector>
#include <list>
#include <stdio.h>
#include <Windows.h>

/*
* To Do:
* arreglar linea 300 y pico
* rotación random no sirve, crear assets para todas las piezas
*/

using namespace std;

wstring tetromino[10];
wstring playerModel[3];

int nFieldWidth = 20;
int nFieldHeight = 20;
unsigned char* pField = nullptr;

int nScreenWidth = 80;
int nScreenHeight = 30;

int nFieldOffset = 2;

struct sObject
{
	int x;
	int y;
	int vx;
	int vy;
	bool bRemove;
};

list<sObject> listObjects;

int Rotate(int px, int py, int r)
{
	switch (r % 4)
	{
	case 0: return py * 4 + px;			
	case 1: return 12 + py - (px * 4);	
	case 2: return 15 - (py * 4) - px;	
	case 3: return 3 - py + (px * 4);	
	}
	return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			// Get index into piece
			int pi = Rotate(px, py, nRotation);

			// Get index into field
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosY + py < nFieldHeight)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

bool CanPlayerMove(int nPlayerModel, int nPosX, int nPosY)
{
	for (int px = 0; px < 5; px++)
	{
		for (int py = 0; py < 5; py++)
		{
			int pi = py * 5 + px;
			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (nPosX + px >= 0 && nPosY + py < nFieldHeight)
			{
				if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
				{
					if (playerModel[nPlayerModel][pi] == L'X' && pField[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

bool PieceDeleted(int nTetromino, int nRotation, int nPosX, int nPosY)
{
	for (int px = 0; px < 4; px++)
	{
		for (int py = 0; py < 4; py++)
		{
			int pi = Rotate(px, py, nRotation);

			int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

			if (tetromino[nTetromino][pi] == L'X')
				return false;
		}
	}
	return true;
}

int main()
{
	// Create assets
	tetromino[0] = (L"..X...X...X...X.");
	tetromino[1] = (L"..X..XX..X......");
	tetromino[2] = (L".X...XX...X.....");
	tetromino[3] = (L".....XX..XX.....");
	tetromino[4] = (L"..X..XX...X.....");
	tetromino[5] = (L".....XX...X...X.");
	tetromino[6] = (L".....XX..X...X..");
	tetromino[7] = (L"....XXXX........");
	tetromino[8] = (L"....XX....XX....");
	tetromino[9] = (L"..X.X..X.XX.X..X");


	// Player model
	playerModel[0].append(L"..X..");
	playerModel[0].append(L"YYXYY");
	playerModel[0].append(L".....");
	playerModel[0].append(L".....");
	playerModel[0].append(L".....");

	pField = new unsigned char[nFieldWidth * nFieldHeight];
	for (int x = 0; x < nFieldWidth; x++)
	{
		for (int y = 0; y < nFieldHeight; y++)
		{
			pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;
		}
	}

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	// Game Logic
	bool bGameOver = false;

	int nCurrentPiece = 1;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;

	int nCurrentPlayerX = nFieldWidth / 2;
	int nCurrentPlayerY = 5;

	bool bKey[4];
	bool bShootHold = false;
	bool bPieceDeleted = false;

	int nSpeed = 20;
	int nSpeedCounter = 0;
	bool bForceDown = false;
	int nPieceCount = 0;
	int nScore = 0;

	vector<int> vLines;

	while (!bGameOver)
	{
		// GAME TIMING ==============================================
		this_thread::sleep_for(50ms);
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		// INPUT ====================================================
		for (int k = 0; k < 4; k++)								//R L D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		// GAME LOGIC ===============================================
		nCurrentPlayerX += (bKey[0] && CanPlayerMove(0, nCurrentPlayerX + 1, nCurrentPlayerY)) ? 1 : 0;
		nCurrentPlayerX -= (bKey[1] && CanPlayerMove(0, nCurrentPlayerX - 1, nCurrentPlayerY)) ? 1 : 0;
		//nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		//nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		bPieceDeleted = PieceDeleted(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

		/*if (bKey[3])
		{
			nCurrentRotation += (!bShootHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bShootHold = true;
		}
		else
		{
			bShootHold = false;
		}*/

		if (bKey[3])
		{
			if (!bShootHold)
			{
				// Instantiate a bullet
				sObject o;
				o.x = nCurrentPlayerX;
				o.y = nFieldHeight;
				o.vy = 2;
				o.bRemove = false;

				listObjects.push_back(o);

				bShootHold = true;
			}
		}
		else
		{
			bShootHold = false;
		}

		if (bForceDown)
		{
			if (bPieceDeleted)
			{
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				// Reset assets
				tetromino[0] = (L"..X...X...X...X.");
				tetromino[1] = (L"..X..XX..X......");
				tetromino[2] = (L".X...XX...X.....");
				tetromino[3] = (L".....XX..XX.....");
				tetromino[4] = (L"..X..XX...X.....");
				tetromino[5] = (L".....XX...X...X.");
				tetromino[6] = (L".....XX..X...X..");

				// Choose next piece
				nCurrentX = nCurrentX = rand() % (nFieldWidth - 4);
				//nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;
			}

			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				nCurrentY++; // It can, so do it!
			}
			else if(!DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
			{
				// Lock the current piece in the field
				for (int px = 0; px < 4; px++)
				{
					for (int py = 0; py < 4; py++)
					{
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
					}
				}

				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				// Check have we got any lines
				for (int py = 0; py < 4; py++)
				{
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}
				}

				nScore += 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				// Create assets
				tetromino[0] = (L"..X...X...X...X.");
				tetromino[1] = (L"..X..XX..X......");
				tetromino[2] = (L".X...XX...X.....");
				tetromino[3] = (L".....XX..XX.....");
				tetromino[4] = (L"..X..XX...X.....");
				tetromino[5] = (L".....XX...X...X.");
				tetromino[6] = (L".....XX..X...X..");

				// Choose next piece
				nCurrentX = nCurrentX = rand() % (nFieldWidth - 4);
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				// if piece does not fit
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}

			nSpeedCounter = 0;
		}


		// RENDER OUTPUT ============================================


		// Draw field
		for (int x = 0; x < nFieldWidth; x++)
		{
			for (int y = 0; y < nFieldHeight; y++)
			{
				screen[(y + nFieldOffset) * nScreenWidth + (x + nFieldOffset)] = L" QWERTYU-#"[pField[y * nFieldWidth + x]];
			}
		}

		// Empty player and draw player
		for (int x = 0; x < nFieldWidth + 3; x++)
		{
			for (int y = 0; y < 5; y++)
			{
				screen[(nFieldHeight + nFieldOffset + y) * nScreenWidth + (x + nFieldOffset)] = L' ';
			}
		}

		for (int px = 0; px < 5; px++)
		{
			for (int py = 0; py < 5; py++)
			{
				if (playerModel[0][py * 5 + px] == L'X')
					screen[(nFieldHeight + nFieldOffset + py) * nScreenWidth + (nCurrentPlayerX + px + nFieldOffset)] = L'*';
				else if (playerModel[0][py * 5 + px] == L'Y')
					screen[(nFieldHeight + nFieldOffset + py) * nScreenWidth + (nCurrentPlayerX + px + nFieldOffset)] = L'|';
			}
		}

		// Draw bullet
		for (auto& object : listObjects)
		{
			object.y -= object.vy;

			/*
			* Mejorar la colision si alcanzo
			*/
			//Remove bullet if collides with the current piece
			if (screen[(object.y + nFieldOffset - 1) * nScreenWidth + (object.x + nFieldOffset + 2)] == '+')
			{
				object.bRemove = true;
			}

			screen[(object.y + nFieldOffset) * nScreenWidth + (object.x + nFieldOffset + 2)] = L'O';

			// Collision detection
			if (object.y <= nFieldOffset - 2)
			{
				object.bRemove = true;
			}
		}

		// Draw current piece
		for (int px = 0; px < 4; px++)
		{
			for (int py = 0; py < 4; py++)
			{
				// Editar la pieza por acá
				if (screen[(nCurrentY + py + 4) * nScreenWidth + (nCurrentX + px + nFieldOffset)] == 'O')
				{
					tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] = L'.';
				}

				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py + nFieldOffset) * nScreenWidth + (nCurrentX + px + nFieldOffset)] = '+';
			}
		}

		// Remove dead bullets from object list
		listObjects.remove_if([](sObject& o) {return o.bRemove; });

		// Draw score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		if (!vLines.empty())
		{
			// Displat frame (cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // Delay a bit

			for (auto& v : vLines)
			{
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
					{
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					}
					pField[px] = 0;
				}
			}

			vLines.clear();
		}
		// Display frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	// Oh dear
	CloseHandle(hConsole);
	std::cout << "Game Over !! Score: " << nScore << endl;
	std::system("pause");

	return 0;
}