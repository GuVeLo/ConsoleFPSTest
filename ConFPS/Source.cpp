//Test para FPS en consola por GuVeLo
//Siguiendo el sgte. tutorial https://www.youtube.com/watch?v=xW8skO7MFYw

#include <iostream>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

#include <stdio.h>
#include <Windows.h>


using namespace std;

//Dimensiones de pantalla
int nScreenWidth = 120;
int nScreenHeight = 40;

//Variables del jugador
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

//Variables del Mapa
int nMapHeight = 16;
int nMapWidth = 16;

//Campo de Vision
float fFOV = 3.14159f / 4.0f;

//Profundidad de vista
float fDepth = 16.0f;

int main()
{
	//Crea Buffer de Pantalla
	//ARRAY hecho con las dimensiones de la pantalla
	wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Mapa
	wstring map;

	map += L"#########.......";
	map += L"#...............";
	map += L"#.......########";
	map += L"#..............#";
	map += L"#......##......#";
	map += L"#......##......#";
	map += L"#..............#";
	map += L"###............#";
	map += L"##.............#";
	map += L"#......####..###";
	map += L"#......#.......#";
	map += L"#......#.......#";
	map += L"#..............#";
	map += L"#......#########";
	map += L"#..............#";
	map += L"################";

	//Tiempo
	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	//Game Loop
	while (1)
	{

		//tiempo al estar ejecutandose
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//Controles
		//Rotacion CCW o Counter Clock Wise
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (1.0f) * fElapsedTime;
		//Rotacion CW
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (1.0f) * fElapsedTime;
		//Movimiento adelante
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
		{
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			//Deteccion de colision
			if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}
		//Movimiento atras
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
		{
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			//Deteccion de colision
			if (map[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
			{
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;;
			}
		}
		
		
		for (int x = 0; x < nScreenWidth; x++)
		{
			//Para cada columna se calcula el angulo del rayo proyectado al mundo
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			//Distancia hacia el muro
			float fDistanceToWall = 0.0f;

			//flag que comprueba si se toco el muro (raycast)
			bool bHitWall = false;
			bool bBoundary = false; //es el borde de la celda?

			//Vectores que representan donde esta mirando el jugador
			float fEyeX = sinf(fRayAngle);
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall && fDistanceToWall < fDepth)
			{

				fDistanceToWall += 0.1f;

				//Test de raycast
				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Test si el rayo esta out of bounds
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
				{
					//Se deja la distancia a la profundidad maxima
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else
				{
					//El rayo esta inbound asi que testear si ve un muro
					if (map[nTestX * nMapWidth + nTestY] == '#')
					{
						bHitWall = true;
						vector<pair<float, float>> p; // Distancia, Producto Punto

						for (int tx = 0; tx < 2; tx++)
							for (int ty = 0; ty < 2; ty++)
							{
								// Angulo del rabillo del ojo
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx*vx + vy * vy); //magnitud del vector (distancia hacia la esquina)
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d); //producto punto
								p.push_back(make_pair(d, dot));
							}

						// Ordena los pares del mas cercano al mas lejano
						sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

						float fBound = 0.01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
						if (acos(p.at(2).second) < fBound) bBoundary = true;

					}
				}

			}

			//Calcula la distancia del cielo y suelo
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;


			//Variables para Muros en ascii extendido escritas en unicode
			short nShade = ' ';
			if (fDistanceToWall <= fDepth / 4.0f)			nShade = 0x2588;	// Muy cerca color claro	
			else if (fDistanceToWall < fDepth / 3.0f)		nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0f)		nShade = 0x2592;
			else if (fDistanceToWall < fDepth)				nShade = 0x2591;
			else											nShade = ' ';		// Muy lejos, invisible

			//Si es un Boundary
			if (bBoundary)									nShade = ' '; //lo vuelve negro

			//Crea el display del mundo basado en el mapa (muros)
			for (int y = 0; y < nScreenHeight; y++)
			{
				if (y <= nCeiling)
					screen[y*nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y*nScreenWidth + x] = nShade;
				else
				{
					//Crea el suelo en base a la distancia
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShade = '#';
					else if (b < 0.5)	nShade = 'x';
					else if (b < 0.75)	nShade = '.';
					else if (b < 0.9)	nShade = '-';
					else				nShade = ' ';
					screen[y*nScreenWidth + x] = nShade;
				}
			}

		}

		//Estadisticas
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Mapa HUD
		for (int nx = 0; nx < nMapWidth; nx++)
			for (int ny = 0; ny < nMapWidth; ny++)
			{
				//screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + (nMapWidth - nx - 1)];
				screen[(ny + 1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
			}
		//screen[((int)fPlayerY + 1) * nScreenWidth + (int)(nMapWidth - fPlayerX)] = 'P';
		screen[((int)fPlayerX + 1) * nScreenWidth + (int)fPlayerY] = 'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

	}

	return 0;
}