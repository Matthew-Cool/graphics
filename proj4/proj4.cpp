//Matthew Wilkinson - proj4 - Graphics

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <random>

#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "libim/im_color.h"

struct Object
{
    float px, py, pz;
    float vx, vy, vz;
    float ax, ay, az;
    float rx, ry, rz;
    float size;
    GLuint textureID;
};

const int NUM_OBJECTS = 6;
Object objects[NUM_OBJECTS];

int windowWidth = 800;
int windowHeight = 600;

float cameraAngleX = 20.0f;
float cameraAngleY = 20.0f;
float cameraDistance = 18.0f;

std::random_device rd;
std::mt19937 gen(rd());

//reused from proj1, editted
float randomFloat(double min, double max, std::mt19937 &g){
    std::uniform_real_distribution<double> dist(min, max);
    return (float)dist(g);
}

GLuint LoadTexture(const char *filename)
{
    im_color image;

    if (!image.ReadJpg((char *)filename))
    {
        printf("Error reading texture file %s\n", filename);
        exit(1);
    }

    int xdim = image.R.Xdim;
    int ydim = image.R.Ydim;

    unsigned char *texture = new unsigned char[xdim * ydim * 3];

    int index = 0;
    for (int y = 0; y < ydim; y++)
    {
        for (int x = 0; x < xdim; x++)
        {
            texture[index + 0] = (unsigned char)image.R.Data2D[y][x];
            texture[index + 1] = (unsigned char)image.G.Data2D[y][x];
            texture[index + 2] = (unsigned char)image.B.Data2D[y][x];
            index += 3;
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, xdim, ydim, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, texture);

    delete[] texture;
    return textureID;
}

void InitObjects()
{
    GLuint tex1 = LoadTexture("textures/door0.jpg");
    GLuint tex2 = LoadTexture("textures/algmidterm.jpg");
    GLuint tex3 = LoadTexture("textures/earth0.jpg");
    GLuint tex4 = LoadTexture("textures/midterm.jpg");
    GLuint tex5 = LoadTexture("textures/proj1.jpg");
    GLuint tex6 = LoadTexture("textures/brick0.jpg");

    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        objects[i].px = randomFloat(-6.0f, 6.0f, gen);
        objects[i].py = randomFloat(-3.0f, 3.0f, gen);
        objects[i].pz = randomFloat(-6.0f, 6.0f, gen);

        objects[i].vx = randomFloat(-0.03f, 0.03f, gen);
        objects[i].vy = randomFloat(-0.02f, 0.02f, gen);
        objects[i].vz = randomFloat(-0.03f, 0.03f, gen);

        objects[i].ax = randomFloat(0.0f, 360.0f, gen);
        objects[i].ay = randomFloat(0.0f, 360.0f, gen);
        objects[i].az = randomFloat(0.0f, 360.0f, gen);

        objects[i].rx = randomFloat(0.2f, 1.0f, gen);
        objects[i].ry = randomFloat(0.2f, 1.0f, gen);
        objects[i].rz = randomFloat(0.2f, 1.0f, gen);

        objects[i].size = randomFloat(1.8f, 3.8f, gen);

        if (i == 0)
            objects[i].textureID = tex1;
        else if (i == 1)
            objects[i].textureID = tex2;
        else if (i == 2)
            objects[i].textureID = tex3;
        else if (i == 3)
            objects[i].textureID = tex4;
        else if (i == 4)
            objects[i].textureID = tex5;
        else if (i == 5)
            objects[i].textureID = tex6;

    }
}

void DrawTexturedCube(float size)
{
    float s = size / 2.0f;

    glBegin(GL_QUADS);

    //front
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s,  s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s,  s);

    //back
    glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s, -s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s, -s);

    //left
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-s, -s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-s,  s,  s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);

    //right
    glTexCoord2f(0.0f, 0.0f); glVertex3f( s, -s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( s,  s,  s);

    //top
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s,  s,  s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s,  s,  s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s,  s, -s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s,  s, -s);

    //bottom
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-s, -s, -s);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( s, -s, -s);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( s, -s,  s);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-s, -s,  s);

    glEnd();
}

void UpdateObjects()
{
    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        objects[i].px += objects[i].vx;
        objects[i].py += objects[i].vy;
        objects[i].pz += objects[i].vz;

        objects[i].ax += objects[i].rx;
        objects[i].ay += objects[i].ry;
        objects[i].az += objects[i].rz;

        //bounce inside a box
        if (objects[i].px < -7.0f || objects[i].px > 7.0f)
            objects[i].vx = -objects[i].vx;
        if (objects[i].py < -4.0f || objects[i].py > 4.0f)
            objects[i].vy = -objects[i].vy;
        if (objects[i].pz < -7.0f || objects[i].pz > 7.0f)
            objects[i].vz = -objects[i].vz;
    }
}

void Idle()
{
    UpdateObjects();
    glutPostRedisplay();
}

void Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -cameraDistance);
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    for (int i = 0; i < NUM_OBJECTS; i++)
    {
        glPushMatrix();

        glTranslatef(objects[i].px, objects[i].py, objects[i].pz);
        glRotatef(objects[i].ax, 1.0f, 0.0f, 0.0f);
        glRotatef(objects[i].ay, 0.0f, 1.0f, 0.0f);
        glRotatef(objects[i].az, 0.0f, 0.0f, 1.0f);

        glBindTexture(GL_TEXTURE_2D, objects[i].textureID);
        DrawTexturedCube(objects[i].size);

        glPopMatrix();
    }

    glutSwapBuffers();
}

void Reshape(int w, int h)
{
    windowWidth = w;
    windowHeight = h;
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)w / (double)h, 1.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

void Keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 27: // ESC
            exit(0);
            break;
        case 'a':
            cameraAngleY -= 5.0f;
            break;
        case 'd':
            cameraAngleY += 5.0f;
            break;
        case 'w':
            cameraAngleX -= 5.0f;
            break;
        case 's':
            cameraAngleX += 5.0f;
            break;
        case 'q':
            cameraDistance -= 1.0f;
            break;
        case 'e':
            cameraDistance += 1.0f;
            break;
    }

    glutPostRedisplay();
}

void Init()
{
    srand((unsigned int)time(NULL));

    glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    InitObjects();
}




int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("proj4 - Matthew Wilkinson - :)");

    printf("\n===== Controls =====\n");
    printf("w : Rotate up\n");
    printf("a : Rotate left\n");
    printf("s : Rotate down\n");
    printf("d : Rotate right\n");
    printf("q : Zoom in\n");
    printf("e : Zoom out\n");
    printf("====================\n\n");

    Init();

    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Keyboard);
    glutIdleFunc(Idle);

    glutMainLoop();
    return 0;
}


