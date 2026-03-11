//---------------------------------------
// Program: surface2.cpp
// Purpose: Generate fractal surface using
//          recursive rectangle splitting.
// Author:  John Gauch
// Date:    September 2008
//---------------------------------------
// EDITTED BY MATTHEW WILKINSON
// FOR PROJ3 OF GRAPHICS CLASS
//---------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Global variables 
int xangle = 0;
int yangle = 0;
int zangle = 0;

// Surface coordinates
const int SIZE = 128;
float Px[SIZE + 1][SIZE + 1];
float Py[SIZE + 1][SIZE + 1];
float Pz[SIZE + 1][SIZE + 1];

//for colors 
float Pr[SIZE + 1][SIZE + 1];
float Pg[SIZE + 1][SIZE + 1];
float Pb[SIZE + 1][SIZE + 1];

//normal arrays
float Nx[SIZE + 1][SIZE + 1];
float Ny[SIZE + 1][SIZE + 1];
float Nz[SIZE + 1][SIZE + 1];

//---------------------------------------
// Calculate random value between [-R..R]
//---------------------------------------
float myrand(float R)
{
   return (2 * R * rand()) / RAND_MAX - R;
}

//---------------------------------------
// HELPER FUNC ADDED BY MATTHEW WILKINSON
//---------------------------------------
float clamp(float value, float low, float high)
{
   if (value < low) return low;
   if (value > high) return high;
   return value;
}

//---------------------------------------
// normalize func ADDED BY MATTHEW WILKINSON
//---------------------------------------
void normalize(float &x, float &y, float &z)
{
   float length = sqrt(x * x + y * y + z * z);
   if (length > 0.00001)
   {
      x /= length;
      y /= length;
      z /= length;
   }
}

//---------------------------------------
// SMOOTH SURFACE FUNC ADDED BY MATTHEW WILKINSON
//---------------------------------------
void smoothSurface()
{
   float temp[SIZE + 1][SIZE + 1];

   // copy original heights first
   for (int i = 0; i <= SIZE; i++){
        for (int j = 0; j <= SIZE; j++){
            temp[i][j] = Pz[i][j];
        }
    }

   // avg close by points
   for (int i = 1; i < SIZE; i++){
        for (int j = 1; j < SIZE; j++)
        {
            float sum = 0.0;
            int count = 0;

            for (int di = -1; di <= 1; di++)
            for (int dj = -1; dj <= 1; dj++)
            {
                sum += Pz[i + di][j + dj];
                count++;
            }

            temp[i][j] = sum / count;
        }
    }

   // actually put the smoother shit in
   for (int i = 1; i < SIZE; i++){
        for (int j = 1; j < SIZE; j++){
            Pz[i][j] = temp[i][j];
        }
    }
}

//---------------------------------------
// setColor FUNC ADDED BY MATTHEW WILKINSON
//---------------------------------------
void setColor(float z, float &r, float &g, float &b)
{
   // Low elevations: dark green
   if (z < -0.05)
   {
      r = 0.0;
      g = 0.35 + (z + 0.30) * 0.4;
      b = 0.0;
   }
   // Lower middle: lighter green
   else if (z < 0.10)
   {
      r = 0.1;
      g = 0.55 + z * 0.8;
      b = 0.1;
   }
   // Middle elevations: light brown
   else if (z < 0.25)
   {
      r = 0.55 + z * 0.6;
      g = 0.40 + z * 0.3;
      b = 0.20;
   }
   // Higher elevations: dark brown
   else if (z < 0.40)
   {
      r = 0.45;
      g = 0.30;
      b = 0.15;
   }
   // Peaks: white snow
   else
   {
      r = 0.95;
      g = 0.95;
      b = 0.95;
   }

   r = clamp(r, 0.0, 1.0);
   g = clamp(g, 0.0, 1.0);
   b = clamp(b, 0.0, 1.0);
}

//---------------------------------------
// init color func ADDED BY MATTHEW WILKINSON
//---------------------------------------
void initColors()
{
   for (int i = 0; i <= SIZE; i++)
   {
      for (int j = 0; j <= SIZE; j++)
      {
        setColor(Pz[i][j], Pr[i][j], Pg[i][j], Pb[i][j]);
      }
   }
}

//---------------------------------------
// init normals func ADDED BY MATTHEW WILKINSON
//---------------------------------------
void initNormals()
{
   for (int i = 1; i < SIZE; i++)
   {
      for (int j = 1; j < SIZE; j++)
      {
        // tangent in x direction
        float tx1 = Px[i+1][j] - Px[i-1][j];
        float ty1 = Py[i+1][j] - Py[i-1][j];
        float tz1 = Pz[i+1][j] - Pz[i-1][j];

        // tangent in y direction
        float tx2 = Px[i][j+1] - Px[i][j-1];
        float ty2 = Py[i][j+1] - Py[i][j-1];
        float tz2 = Pz[i][j+1] - Pz[i][j-1];

        // cross product T1 x T2
        Nx[i][j] = ty1 * tz2 - tz1 * ty2;
        Ny[i][j] = tz1 * tx2 - tx1 * tz2;
        Nz[i][j] = tx1 * ty2 - ty1 * tx2;

        normalize(Nx[i][j], Ny[i][j], Nz[i][j]);
      }
   }

   // copy edge normals from nearby
   for (int i = 0; i <= SIZE; i++)
   {
      Nx[i][0] = Nx[i][1];
      Ny[i][0] = Ny[i][1];
      Nz[i][0] = Nz[i][1];

      Nx[i][SIZE] = Nx[i][SIZE-1];
      Ny[i][SIZE] = Ny[i][SIZE-1];
      Nz[i][SIZE] = Nz[i][SIZE-1];
   }

   for (int j = 0; j <= SIZE; j++)
   {
      Nx[0][j] = Nx[1][j];
      Ny[0][j] = Ny[1][j];
      Nz[0][j] = Nz[1][j];

      Nx[SIZE][j] = Nx[SIZE-1][j];
      Ny[SIZE][j] = Ny[SIZE-1][j];
      Nz[SIZE][j] = Nz[SIZE-1][j];
   }
}

//---------------------------------------
// diffuse lighting func ADDED BY MATTHEW WILKINSON
//---------------------------------------
void applyLighting(int i, int j, float &r, float &g, float &b)
{
   // light direction
   float Lx = 0.6;
   float Ly = 0.6;
   float Lz = 1.0;
   normalize(Lx, Ly, Lz);

   float dot = Nx[i][j] * Lx + Ny[i][j] * Ly + Nz[i][j] * Lz;

   if (dot < 0.0){
      dot = 0.0;
   }

   float brightness = dot;

   r = clamp(Pr[i][j] * brightness, 0.0, 1.0);
   g = clamp(Pg[i][j] * brightness, 0.0, 1.0);
   b = clamp(Pb[i][j] * brightness, 0.0, 1.0);
}

//---------------------------------------
// Initialize random surface FUNC ADDED BY MATTHEW WILKINSON
//---------------------------------------
void init_surface()
{
   // reset x, y and z
   for (int i = 0; i <= SIZE; i++)
   {
      for (int j = 0; j <= SIZE; j++)
      {
         Px[i][j] = -0.8 + 1.6 * i / SIZE;
         Py[i][j] = -0.8 + 1.6 * j / SIZE;
         Pz[i][j] = 0.0;
      }
   }

   // make several random hills, so we can see snow :)
   int numHills = 8;

   for (int h = 0; h < numHills; h++)
   {
      float cx = myrand(0.8);
      float cy = myrand(0.8);
      float height = 0.15 + (rand() / (float)RAND_MAX) * 0.35;
      float spread = 0.15 + (rand() / (float)RAND_MAX) * 0.25;

      for (int i = 0; i <= SIZE; i++)
      {
         for (int j = 0; j <= SIZE; j++)
         {
            float dx = Px[i][j] - cx;
            float dy = Py[i][j] - cy;
            float dist2 = dx * dx + dy * dy;

            // smooth round hill
            Pz[i][j] += height * exp(-dist2 / (spread * spread));
         }
      }
   }

   // small random noise so it does not look too perfect
   for (int i = 0; i <= SIZE; i++)
   {
      for (int j = 0; j <= SIZE; j++)
      {
         Pz[i][j] += myrand(0.05);
      }
   }

   // smooth several times, so it looks better
   smoothSurface();
   smoothSurface();
   smoothSurface();

   initColors();
   initNormals();
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   // Initialize OpenGL
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
   glEnable(GL_DEPTH_TEST);
}
  

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   // Incrementally rotate objects
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glRotatef(xangle, 1.0, 0.0, 0.0);
   glRotatef(yangle, 0.0, 1.0, 0.0);
   glRotatef(zangle, 0.0, 0.0, 1.0);

   // Draw the surface
   /*int i, j;
   glColor3f(1.0, 0.0, 0.0);
   for (i = 0; i <= SIZE; i++)
   {
      glBegin(GL_LINE_STRIP);
      for (j = 0; j <= SIZE; j++)
	 glVertex3f(Px[i][j], Py[i][j], Pz[i][j]);
      glEnd();
   }
   glColor3f(0.0, 1.0, 0.0);
   for (j = 0; j <= SIZE; j++)
   {
      glBegin(GL_LINE_STRIP);
      for (i = 0; i <= SIZE; i++)
	 glVertex3f(Px[i][j], Py[i][j], Pz[i][j]);
      glEnd();
   }*/
   for(int i = 0; i < SIZE; i++){
    for(int j = 0; j < SIZE; j++){
        glBegin(GL_QUADS);

        float r, g, b;

        applyLighting(i, j, r, g, b);
        glColor3f(r, g, b);
        glVertex3f(Px[i][j], Py[i][j], Pz[i][j]);

        applyLighting(i+1, j, r, g, b);
        glColor3f(r, g, b);
        glVertex3f(Px[i+1][j], Py[i+1][j], Pz[i+1][j]);

        applyLighting(i+1, j+1, r, g, b);
        glColor3f(r, g, b);
        glVertex3f(Px[i+1][j+1], Py[i+1][j+1], Pz[i+1][j+1]);

        applyLighting(i, j+1, r, g, b);
        glColor3f(r, g, b);
        glVertex3f(Px[i][j+1], Py[i][j+1], Pz[i][j+1]);

        glEnd();
    }
   }
   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   // Initialize surface
   if (key == 'i')
      init_surface();

   // Update angles
   else if (key == 'x')
      xangle -= 5;
   else if (key == 'y')
      yangle -= 5;
   else if (key == 'z')
      zangle -= 5;
   else if (key == 'X')
      xangle += 5;
   else if (key == 'Y')
      yangle += 5;
   else if (key == 'Z')
      zangle += 5;

   // Quit program
   else if (key == 'q')
      exit(0);

   // Redraw objects
   glutPostRedisplay();
}

//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   srand(time(NULL)); //so we get different surfaces everytime, yay :)
   // well I dont think it works, oh well

   glutInit(&argc, argv);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(250, 250);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
   glutCreateWindow("Surface");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   init();
   init_surface();
    printf("Keyboard commands:\n");
    printf("   'x' - rotate x-axis -5 degrees\n");
    printf("   'X' - rotate x-axis +5 degrees\n");
    printf("   'y' - rotate y-axis -5 degrees\n");
    printf("   'Y' - rotate y-axis +5 degrees\n");
    printf("   'z' - rotate z-axis -5 degrees\n");
    printf("   'Z' - rotate z-axis +5 degrees\n");
    printf("   'i' - initialize surface\n");
    printf("   'q' - quit program\n");
   glutMainLoop();
   return 0;
}
