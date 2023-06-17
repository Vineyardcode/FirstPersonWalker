

#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;
using namespace chrono;

#include <stdio.h>

#include <Windows.h>

int windowWidth = 150;
int windowHeight = 50;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.141559 / 4.0;
float fDepth = 16.0f;

int main()
{

	wchar_t* screen = new wchar_t[windowWidth * windowHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..##..........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..........##..#";
	map += L"#..........##..#";
	map += L"#..............#";
	map += L"################";



	auto tp1 = system_clock::now();
	auto tp2 = system_clock::now();



	while (1)
	{

		tp2 = system_clock::now();
		duration<float> elapsedTime = tp2 - tp1;

		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();


		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
 			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}



		for (int x = 0; x < windowWidth; x++)
		{
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)windowWidth) * fFOV;
		
		
			float fDistanceToWall = 0.0f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall  && fDistanceToWall < fDepth) 
			{
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);


				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					if (map[nTestY * nMapWidth + nTestX] == '#')
					{
						bHitWall = true;

						vector<pair<float, float>> p; // distance, dot

						for (int tx = 0; tx < 2; tx++)
						{
							for (int ty = 0; ty < 2; ty++)
							{
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}

						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float,float> & right) {return left.first < right.first; });
					
					
						float fBound = 0.05;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;
					}
				}
			}

			// distance player <---> ceiling/floor 
			int nCeiling = (float)(windowHeight / 2.0) - windowHeight / ((float)fDistanceToWall);
			int nFloor = windowHeight - nCeiling;


			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f)       nShade = 0x2588; //close
			else if (fDistanceToWall < fDepth / 3.0f)	nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)	nShade = 0x2592;
			else if (fDistanceToWall < fDepth)			nShade = 0x2591;
			else										nShade = ' ';

			if (bBoundary)			nShade = 'B';


			for (int y = 0; y < windowHeight; y++)
			{
				if (y <= nCeiling)
					screen[y * windowWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * windowWidth + x] = nShade;
				else
				{
					float b = 1.0f - (((float)y - windowHeight / 2.0f) / ((float)windowHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)   nShade = 'x';
					else if (b < 0.75)  nShade = '.';
					else if (b < 0.9)   nShade = '-';
					else				nShade = ' ';
					screen[y * windowWidth + x] = nShade;
				}
			}
		}

		// display map
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				screen[(ny + 1) * windowWidth + nx] = map[ny * nMapWidth + nx];
			}
		screen[((int)fPlayerX + 1) * windowWidth + (int)fPlayerY] = 'P';

		screen[windowWidth * windowHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, windowWidth * windowHeight, { 0,0 }, &dwBytesWritten);

	}

	return 0;

}

