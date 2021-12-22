#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"
#define M_PI 3.14159265358979323846


#include <cmath>
#include <vector>
using namespace std;

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}

double* NormalVector(double A[], double B[], double C[])
{
	double temp[3];
	double a[] = { 0,0,0 };
	double b[] = { 0,0,0 };
	double length;
	for (int i = 0; i < 3; i++)
	{
		a[i] = A[i] - B[i];
		b[i] = C[i] - B[i];
	}
	temp[0] = a[1] * b[2] - b[1] * a[2];
	temp[1] = -a[0] * b[2] + b[0] * a[2];
	temp[2] = a[0] * b[1] - b[0] * a[1];
	length = sqrt(pow(temp[0], 2) + pow(temp[1], 2) + pow(temp[2], 2));
	temp[0] /= length;
	temp[1] /= length;
	temp[2] /= length;
	return temp;
}






GLuint texId;
//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

#pragma region Лаба1



double A[] = { -2,5,0 };
double B[] = { 6,3,0 };
double C[] = { 4,-2,0 };
double D[] = { 9,-6,0 };
double E[] = { 1,-3,0 };
double F[] = { -5,-9,0 };
double G[] = { -2,-3,0 };
double H[] = { -5,1,0 };

double A1[] = { -2,5,2 };
double B1[] = { 6,3,2 };
double C1[] = { 4,-2,2 };
double D1[] = { 9,-6,2 };
double E1[] = { 1,-3,2 };
double F1[] = { -5,-9,2 };
double G1[] = { -2,-3,2 };
double H1[] = { -5,1,2 };
const int ahCount = 60;
double* ahX = new double[ahCount];
double* ahY = new double[ahCount];


const int abCount = 240;
double* abX = new double[abCount];
double* abY = new double[abCount];
void GetXY(double& x, double& y, double x0, double y0, double radius, int countPoints, int i, double _angle, double _acos)
{
	double angle = _angle / countPoints * i;
	double addRad = acos(_acos);
	x = x0 + radius * cos(angle * M_PI / 180 + addRad);
	y = y0 + radius * sin(angle * M_PI / 180 + addRad);
}

bool init = true;
void Initialize()
{
	if (init)
	{
		init = false;
		ahX[0] = A[0];
		ahY[0] = A[1];
		for (size_t i = 1; i < ahCount - 1; i++)
		{
			GetXY(ahX[i], ahY[i], -3.5, 3, 2.5, ahCount, i, 150, 0.6);
		}
		ahX[ahCount - 1] = H[0];
		ahY[ahCount - 1] = H[1];


		for (size_t i = 1; i < abCount - 1; i++)
		{
			GetXY(abX[i], abY[i], 2.85, 7, 5.15, abCount, i, 180, -0.95);
		}
		abX[56] = A[0];
		abY[56] = A[1];
		abX[abCount - 48] = B[0];
		abY[abCount - 48] = B[1];


	}
}

void SetHeight(double h)
{
	A[2] = h;
	B[2] = h;
	C[2] = h;
	D[2] = h;
	E[2] = h;
	F[2] = h;
	G[2] = h;
	H[2] = h;
}

double* ChangeH(double* a, double h)
{
	a[2] = h;
	return a;
}


void Foundation(double h)
{
	Initialize();
	SetHeight(h);
	glBegin(GL_TRIANGLES);

	glVertex3dv(A);
	glVertex3dv(H);
	glVertex3dv(E);

	glVertex3dv(C);
	glVertex3dv(B);
	glVertex3dv(E);



	glVertex3dv(G);
	glVertex3dv(E);
	glVertex3dv(H);

	glVertex3dv(G);
	glVertex3dv(E);
	glVertex3dv(F);

	glVertex3dv(E);
	glVertex3dv(C);
	glVertex3dv(D);

	for (size_t i = 0; i < ahCount - 1; i++)
	{
		glVertex3d(ahX[i], ahY[i], h);
		glVertex3d(ahX[i + 1], ahY[i + 1], h);
		glVertex3d(-3.5, 3, h);
	}
	for (size_t i = 56; i < abCount - 48; i++)
	{
		glVertex3d(abX[i], abY[i], h);
		glVertex3d(abX[i + 1], abY[i + 1], h);
		glVertex3dv(E);
	}

	glEnd();
}


void Walls(double h)
{
	glBegin(GL_QUADS);

	//BC
	glNormal3dv(NormalVector(C, B1, B));
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C1);
	glVertex3dv(B1);
	//CD
	glNormal3dv(NormalVector(C, D, D1));
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(D1);
	glVertex3dv(C1);
	//DE
	glNormal3dv(NormalVector(D, E, E1));
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(E1);
	glVertex3dv(D1);
	//EF
	glBindTexture(GL_TEXTURE_2D, texId);




	glNormal3dv(NormalVector(E, F, F1));
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glTexCoord2d(1, 0);
	glVertex3dv(E);
	glTexCoord2d(1, 1);
	glVertex3dv(E1);
	glTexCoord2d(0, 1);
	glVertex3dv(F1);
	
	//FG
	glNormal3dv(NormalVector(F, G, G1));
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G1);
	glVertex3dv(F1);
	//GH
	glNormal3dv(NormalVector(G, H, H1));
	glVertex3dv(G);
	glVertex3dv(H);
	glVertex3dv(H1);
	glVertex3dv(G1);
	//AH
	for (size_t i = 0; i < ahCount - 1; i++)
	{
		double z[3] = { ahX[i], ahY[i], 0 };
		double z1[3] = { ahX[i], ahY[i], h };
		double w1[3] = { ahX[i + 1], ahY[i + 1], h };
		double w[3] = { ahX[i + 1], ahY[i + 1], 0 };
		glNormal3dv(NormalVector(w, z, z1));
		glVertex3dv(z);
		glVertex3dv(z1);
		glVertex3dv(w1);
		glVertex3dv(w);
		//glVertex3d(-3.5, 3, h);
	}


	//AB
	for (size_t i = 56; i < abCount - 48; i++)
	{
		double z[3] = { abX[i], abY[i], 0 };
		double z1[3] = { abX[i], abY[i], h };
		double w1[3] = { abX[i + 1], abY[i + 1], h };
		double w[3] = { abX[i + 1], abY[i + 1], 0 };
		glNormal3dv(NormalVector(w1, z1, z));
		glVertex3dv(z);
		glVertex3dv(z1);
		glVertex3dv(w1);
		glVertex3dv(w);
		//glVertex3d(-3.5, 3, h);
	}
	glEnd();


}

double h = 2;

void LAB1()
{
	glColor3d(0.5, 0.2, 0.8);
	glNormal3d(0, 0, -1);
	Foundation(0);
	glColor3d(0.5, 1, 0.5);
	Walls(h);
	glColor3d(1, 0.5, 1);
	//glColor4d(0.5, 0.5, 1, 0.75);

	glNormal3d(0, 0, 1);
	Foundation(h);
}
#pragma endregion



void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);


	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	LAB1();

	////Начало рисования квадратика станкина
	//double A[2] = { -4, -4 };
	//double B[2] = { 4, -4 };
	//double C[2] = { 4, 4 };
	//double D[2] = { -4, 4 };

	

	//glEnd();
	////конец рисования квадратика станкина


   ////Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}