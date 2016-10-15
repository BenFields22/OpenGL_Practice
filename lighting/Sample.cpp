#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#include <GL/glew.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

float angle = 0,angle2 =0;
int mult =1;//used to change animation from forward to backwards
float rot = 0;
GLuint boxlist1,boxlist2;
bool Freeze = false;




int	ReadInt(FILE *);
short	ReadShort(FILE *);


struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

/**
** read a BMP file into a Texture:
**/

unsigned char *
BmpToTexture(char *filename, int *width, int *height)
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open Bmp file '%s'\n", filename);
		return NULL;
	}

	FileHeader.bfType = ReadShort(fp);


	// if bfType is not 0x4d42, the file is not a bmp:

	if (FileHeader.bfType != 0x4d42)
	{
		fprintf(stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType);
		fclose(fp);
		return NULL;
	}

	FileHeader.bfSize = ReadInt(fp);
	FileHeader.bfReserved1 = ReadShort(fp);
	FileHeader.bfReserved2 = ReadShort(fp);
	FileHeader.bfOffBits = ReadInt(fp);


	InfoHeader.biSize = ReadInt(fp);
	InfoHeader.biWidth = ReadInt(fp);
	InfoHeader.biHeight = ReadInt(fp);

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort(fp);
	InfoHeader.biBitCount = ReadShort(fp);
	InfoHeader.biCompression = ReadInt(fp);
	InfoHeader.biSizeImage = ReadInt(fp);
	InfoHeader.biXPelsPerMeter = ReadInt(fp);
	InfoHeader.biYPelsPerMeter = ReadInt(fp);
	InfoHeader.biClrUsed = ReadInt(fp);
	InfoHeader.biClrImportant = ReadInt(fp);

	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );

	texture = new unsigned char[3 * nums * numt];
	if (texture == NULL)
	{
		fprintf(stderr, "Cannot allocate the texture array!\b");
		return NULL;
	}

	// extra padding bytes:

	numextra = 4 * (((3 * InfoHeader.biWidth) + 3) / 4) - 3 * InfoHeader.biWidth;


	// we do not support compression:

	if (InfoHeader.biCompression != birgb)
	{
		fprintf(stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression);
		fclose(fp);
		return NULL;
	}

	rewind(fp);
	fseek(fp, 14 + 40, SEEK_SET);

	if (InfoHeader.biBitCount == 24)
	{
		for (t = 0, tp = texture; t < numt; t++)
		{
			for (s = 0; s < nums; s++, tp += 3)
			{
				*(tp + 2) = fgetc(fp);		// b
				*(tp + 1) = fgetc(fp);		// g
				*(tp + 0) = fgetc(fp);		// r
			}

			for (e = 0; e < numextra; e++)
			{
				fgetc(fp);
			}
		}
	}

	fclose(fp);

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt(FILE *fp)
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	b2 = fgetc(fp);
	b3 = fgetc(fp);
	return (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
}


short
ReadShort(FILE *fp)
{
	unsigned char b1, b0;
	b0 = fgetc(fp);
	b1 = fgetc(fp);
	return (b1 << 8) | b0;
}


void display()
{
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90., 1., 0.1, 1000.);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(1.5, 0.0, 1.5, 0., 0., 0., 0., 1.0, 0.0);

	//glRotatef(45, 0., 1., 0.);
	glPushMatrix();
	glTranslatef(angle, 0.0, 0.);
	glRotatef(rot, 0.0, 0.0, 1.0);
	glCallList(boxlist1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(angle2, 0.0, 0.);
	glTranslatef(0.0, 0.0, .5);
	glRotatef(rot, -.5, 0.0, 0.0);
	glTranslatef(0.0, 0.0, -.5);
	glCallList(boxlist2);
	glPopMatrix();

	glTranslatef(-2.0, 0.0, 0.0);
	glCallList(boxlist2);


	glutSwapBuffers();
	glFlush();
	//printf("Finished display\n");
}

void initLists()
{
	boxlist1 = glGenLists(1);
	glNewList(boxlist1, GL_COMPILE);
	int width = 1024, height = 512;
	unsigned char *Texture = BmpToTexture("worldtex.bmp", &width, &height);
	GLuint tex0;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &tex0);
	glBindTexture(GL_TEXTURE_2D, tex0); // make tex1 texture current
										// and set its parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 512, 0, GL_RGB, GL_UNSIGNED_BYTE, Texture);
	
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex0);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glNormal3f(0, 0, 1);
	glVertex3f(-.5, -.5, 0.);

	glTexCoord2f(0, 1);
	glNormal3f(0, 0, 1);
	glVertex3f(-.5, .5, 0.);

	glTexCoord2f(1, 1);
	glNormal3f(0, 0, 1);
	glVertex3f(.5, .5, 0.);

	glTexCoord2f(1, 0);
	glNormal3f(0, 0, 1);
	glVertex3f(.5, -.5, 0.);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex0);
	glEndList();

	boxlist2 = glGenLists(1);
	glNewList(boxlist2, GL_COMPILE);

	glBegin(GL_QUADS);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(-.5, -.5, 0.);

		glColor3f(1.0, 1.0, 0.0);
		glVertex3f(-.5, -.5, 1.);

		glColor3f(0.0, 1.0, 1.0);
		glVertex3f(-.5, .5, 1.);

		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(-.5, .5, 0.);
	glEnd();
	glEndList();

}

void animate()
{
	if (angle > 1||angle <0)
		mult *= -1;
	angle += (float)(.01 * mult);
	angle2 -= (float)(.01* mult);
	rot += 1;
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

void initOpenGL()
{
	glClearColor(.5f, .5f, .5f, 1.0f);

}
void
Keyboard(unsigned char c, int x, int y)
{

	switch (c)
	{

	case 'q':
	case 'Q':
		glFinish();	
		exit(0);
		break;
	case 'f':
	case 'F':
		Freeze = !Freeze;
		if (Freeze)
			glutIdleFunc(NULL);
		else
			glutIdleFunc(animate);
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	//initialize freeglut
	glutInit(&argc, argv);

	//declare what type of window to use
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1024, 512);
	glutInitWindowPosition(100, 100);

	//create the window
	glutCreateWindow("My window :)");
	
	//initialize the openGl api
	initOpenGL();
	initLists();

	glutDisplayFunc(display);
	glutIdleFunc(animate);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(Keyboard);

	//enter the graphics loop
	glutMainLoop();


	return 0;
}