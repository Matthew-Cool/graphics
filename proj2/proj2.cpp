// main.cpp
// Compile: g++ main.cpp -o app -lGL -lGLU -lglut

#include <GL/glut.h>
#include <cstdlib>
#include <iostream>

#include <cmath>
#include <vector>
#include <random>
#include <GL/freeglut.h>


int winW = 800;
int winH = 600;
bool gameOver = false;
float bgHue = 0.0f;
int gameWinPoints = 100;

std::random_device rd;
std::mt19937 gen(rd());

//reused from proj1, editted
float getRandomNum(double min, double max, std::mt19937 &g){
    std::uniform_real_distribution<double> dist(min, max);
    return (float)dist(g);
}


class Pacman {
    private: 
        float x;
        float y;
        int points;
        float angle;

    public: 
        Pacman(){
            this->x = 400.0f;
            this->y = 300.0f;
            this->points = 0;
            this->angle = 0.0f;
        }
        
        float getX(){
            return this->x;
        }

        float getY(){
            return this->y;
        }

        void setX(float x){
            if(x >= winW){
                this->x = winW;
            } else if (x <= 0.0f){
                this->x = 0.0f;
            } else {
                this->x = x;
            }
        }

        void setY(float y){
            if(y >= winH){
                this->y = winH;
            } else if (y <= 0.0f){
                this->y = 0.0f;
            } else {
                this->y = y;
            }
        }

        int getPoints(){
            return this->points;
        }

        void addPoint(){
            this->points++;
        }

        float getAngle(){
            return this->angle;
        }

        void setAngle(float a){
            this->angle = a;
        }

        void draw() const {
            glColor3f(1.0f, 1.0f, 0.0f); //yellow
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(this->x, this->y);
            float radius = 15.0f;
            float mouthAngle = 45.0f;

            for (int i = mouthAngle / 2; i <= 360 - mouthAngle/2; i++) {
                float deg = i + this->angle;
                float rad = deg * 3.1415926f / 180.0f;
                glVertex2f(this->x + std::cos(rad) * radius,
                        this->y + std::sin(rad) * radius);
            }
            glEnd();
        }
};

class Food {
    private:
        float x;
        float y;
        float radius;
    
    public: 
        Food(float x, float y){
            this->x = x;
            this->y = y;
            this->radius = 5.0f;
        }

        void setPos(float x, float y){
            this->x = x;
            this->y = y;
        }

        float getX(){
            return this->x;
        }

        float getY(){
            return this->y;
        }

        void reset(){
            float x = getRandomNum(0, winW, gen);
            float y = getRandomNum(0, winH, gen);
            this->x = x;
            this->y = y;
        }

        void draw() const {
            glColor3f(1.0f, 1.0f, 1.0f);  // white

            int segments = 40;

            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(this->x, this->y);

            for (int i = 0; i <= segments; i++) {
                float angle = 2.0f * 3.1415926f * i / segments;
                float cx = this->radius * cos(angle);
                float cy = this->radius * sin(angle);
                glVertex2f(x + cx, y + cy);
            }

            glEnd();
        }

        //make a func if eaten, move position and add point
};

class Ghost {
    private: 
        float x;
        float y;
        float r, g, b;
    
    public: 
        Ghost(float x, float y, float r, float g, float b){
            this->x = x;
            this->y = y;
            this->r = r;
            this->g = g;
            this->b = b;
        }

        float getX(){
            return this->x;
        }

        float getY(){
            return this->y;
        }

        void setPos(float x, float y){
            this->x = x;
            this->y = y;
        }

        void draw() const {
            float radius = 15.0f;
            int segments = 40;

            glColor3f(this->r, this->g, this->b);

            // rectangle body
            glBegin(GL_QUADS);
            glVertex2f(x - radius, y);
            glVertex2f(x + radius, y);
            glVertex2f(x + radius, y - radius);
            glVertex2f(x - radius, y - radius);
            glEnd();

            // semi-circle top
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y + radius);

            for (int i = 0; i <= segments; i++) {
                float angle = 3.14f * i / segments;
                float cx = radius * std::cos(angle);
                float cy = radius * std::sin(angle);
                glVertex2f(x + cx, y + cy);
            }
            glEnd();

            //eyes
            glColor3f(1.0f, 1.0f, 1.0f);  // white

            float eyeRadius = 5.0f;
            int eyeSegments = 20;

            // Left eye
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x - 6, y + 5);
            for (int i = 0; i <= eyeSegments; i++) {
                float angle = 2.0f * 3.1415926f * i / eyeSegments;
                glVertex2f(x - 6 + std::cos(angle) * eyeRadius,
                        y + 5 + std::sin(angle) * eyeRadius);
            }
            glEnd();

            // Right eye
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x + 6, y + 5);
            for (int i = 0; i <= eyeSegments; i++) {
                float angle = 2.0f * 3.1415926f * i / eyeSegments;
                glVertex2f(x + 6 + std::cos(angle) * eyeRadius,
                        y + 5 + std::sin(angle) * eyeRadius);
            }
            glEnd();
        }

};


// ******************************* Main edit stuff here, idk what to call it lol
//Food food = Food(100.0f, 20.0f);
const int numFood = 40;
const int numGhost = 4;

//g vars
Pacman pacman = Pacman();
bool keyUp=false, keyDown=false, keyLeft=false, keyRight=false;
float pacSpeed = 1.0f;
std::vector<Food> foods;
std::vector<Ghost> ghosts;

float speedMin = 0.5f;
float speedMax = 2.0f;
std::vector<float> foodDX;
std::vector<float> foodDY;
std::vector<float> ghostDX;
std::vector<float> ghostDY;

//to check distance between 2 points
bool collisionCheck(float ax, float ay, float ar, float bx, float by, float br)
{
    float dx = ax - bx;
    float dy = ay - by;
    float r = ar + br;
    return (dx*dx + dy*dy) <= (r*r);
}

void init(){
    //init spawn food
    for(int i = 0; i < numFood; i++){
        float x = getRandomNum(0, winW, gen);
        float y = getRandomNum(0, winH, gen);

        Food item = Food(x, y);

        foods.push_back(item);

        float dx = getRandomNum(-0.5, 0.5, gen);
        float dy = getRandomNum(-0.5, 0.5, gen);

        // avoid zero so it actually moves
        if (std::fabs(dx) < 0.2f) dx = (dx < 0 ? -0.2f : 0.2f);
        if (std::fabs(dy) < 0.2f) dy = (dy < 0 ? -0.2f : 0.2f);

        foodDX.push_back(dx);
        foodDY.push_back(dy);
    }

    //init spawn ghosts
    float gx[4] = {200, 600, 200, 600};
    float gy[4] = {450, 450, 150, 150};
    float gr[4] = {1.0, 1.0, 0.0, 1.0};
    float gg[4] = {0.0, 0.4, 1.0, 0.5};
    float gb[4] = {0.0, 0.7, 1.0, 0.0};
    for(int i = 0; i < 4; i++){
        Ghost g = Ghost(gx[i], gy[i], gr[i], gg[i], gb[i]);
        ghosts.push_back(g);

        float dx = getRandomNum(-1.5, 1.5, gen);
        float dy = getRandomNum(-1.5, 1.5, gen);

        // avoid zero so it actually moves
        if (std::fabs(dx) < 0.2f) dx = (dx < 0 ? -0.2f : 0.2f);
        if (std::fabs(dy) < 0.2f) dy = (dy < 0 ? -0.2f : 0.2f);

        ghostDX.push_back(dx);
        ghostDY.push_back(dy);
    }

    //print rules
    std::cout << "Hello! Welcome to completely normal pacman.\nThe RULES are simple!\nHave a ghost touch you, you die (pacman) and the program ends forever (not really).\nEat food to increase your score. Get to 100 points to get a special victory screen!\nThats it! Good luck!!!\n\n\n" << std::endl;

}

//for the special victory screen
void hsvToRgb(float h, float s, float v, float &r, float &g, float &b)
{
    float c = v * s;
    float x = c * (1 - std::fabs(fmod(h / 60.0f, 2) - 1));
    float m = v - c;

    float rp=0, gp=0, bp=0;

    if (h < 60)       { rp=c; gp=x; bp=0; }
    else if (h < 120) { rp=x; gp=c; bp=0; }
    else if (h < 180) { rp=0; gp=c; bp=x; }
    else if (h < 240) { rp=0; gp=x; bp=c; }
    else if (h < 300) { rp=x; gp=0; bp=c; }
    else              { rp=c; gp=0; bp=x; }

    r = rp + m;
    g = gp + m;
    b = bp + m;
}

void drawText(float x, float y, const std::string &text, void* font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

void display()
{    
    if (pacman.getPoints() >= gameWinPoints) {
        float r,g,b;
        hsvToRgb(bgHue, 1.0f, 1.0f, r, g, b);
        glClearColor(r, g, b, 1.0f);
    } else {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    //draw pacman
    pacman.draw();

    //draw food
    for(int i = 0; i < numFood; i++){
        foods[i].draw();
    }

    //draw ghosts
    for(int i = 0; i < 4; i++){
        if(pacman.getPoints() < gameWinPoints){
            ghosts[i].draw();

            //change ghost dir if ghost is lucky
            int ran = (int)getRandomNum(0.0, 1000.0, gen);
            if(ran == 67){ //hahaha 67, do it, ik you wanna
                float dx = getRandomNum(-1.5, 1.5, gen);
                float dy = getRandomNum(-1.5, 1.5, gen);
                ghostDX[i] = dx;
                ghostDY[i] = dy;
                //std::cout << "changing dirs" << std::endl;
            }
        }
    }

    //draw victory text
    if (pacman.getPoints() >= gameWinPoints) {
        glColor3f(1.0f, 1.0f, 1.0f);  // white text
        
        drawText(winW/2 - 120, winH/2 + 20, "VICTORY!");
        drawText(winW/2 - 160, winH/2 - 10, "You got 100 points!");
        drawText(winW/2 - 200, winH/2 - 40, "The ghosts are gone now, eat all the food you want!");
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 27) // ESC
        std::exit(0);
}

void specialUp(int key, int, int)
{
    if (key == GLUT_KEY_UP) {
        keyUp = false;
    }
    if (key == GLUT_KEY_DOWN) {
        keyDown = false;
    }
    if (key == GLUT_KEY_LEFT) {
        keyLeft = false;
    }
    if (key == GLUT_KEY_RIGHT) {
        keyRight = false;
    }
    glutPostRedisplay();

}

void special(int key, int, int)
{
    if (key == GLUT_KEY_UP) {
        keyUp = true;
    }
    if (key == GLUT_KEY_DOWN) {
        keyDown = true;
    }
    if (key == GLUT_KEY_LEFT) {
        keyLeft = true;
    }
    if (key == GLUT_KEY_RIGHT) {
        keyRight = true;
    }
    glutPostRedisplay();

}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (double)w, 0.0, (double)h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int)
{
    float r = 5.0f;

    //food shit
    for (int i = 0; i < (int)foods.size(); i++) {
        float x = foods[i].getX() + foodDX[i];
        float y = foods[i].getY() + foodDY[i];

        // bounce
        if (x - r < 0)      { x = r;        foodDX[i] = -foodDX[i]; }
        if (x + r > winW)   { x = winW - r; foodDX[i] = -foodDX[i]; }
        if (y - r < 0)      { y = r;        foodDY[i] = -foodDY[i]; }
        if (y + r > winH)   { y = winH - r; foodDY[i] = -foodDY[i]; }

        foods[i].setPos(x, y);
    }

    //ghost shit
    for (int i = 0; i < (int)ghosts.size(); i++) {
        float x = ghosts[i].getX() + ghostDX[i];
        float y = ghosts[i].getY() + ghostDY[i];

        // bounce
        if (x - r < 0)      { x = r;        ghostDX[i] = -ghostDX[i]; }
        if (x + r > winW)   { x = winW - r; ghostDX[i] = -ghostDX[i]; }
        if (y - r < 0)      { y = r;        ghostDY[i] = -ghostDY[i]; }
        if (y + r > winH)   { y = winH - r; ghostDY[i] = -ghostDY[i]; }

        ghosts[i].setPos(x, y);
    }

    //pacman shit
    float dx = 0.0f;
    float dy = 0.0f;
    if(keyUp){
        dy += pacSpeed;
    }
    if(keyDown){
        dy -= pacSpeed;
    }
    if(keyRight){
        dx += pacSpeed;
    }
    if(keyLeft){
        dx -= pacSpeed;
    }

    //so diagonal isn't so fast lmao
    float len = std::sqrt(dx*dx + dy*dy);
    if (len > 0.0f) {
        dx = dx / len * pacSpeed;
        dy = dy / len * pacSpeed;
    }

    pacman.setX(pacman.getX() + dx);
    pacman.setY(pacman.getY() + dy);

    if(dx != 0.0f || dy != 0.0f){
        float ang = std::atan2(dy, dx) * 180.0f / 3.14;
        pacman.setAngle(ang);
    }

    //lets do collision checking here
    float pR = 15.0f;
    //food shit
    for (int i = 0; i < (int)foods.size(); i++) {
        float x = foods[i].getX();
        float y = foods[i].getY();

        if(collisionCheck(pacman.getX(), pacman.getY(), pR, x, y, 5.0f)){
            foods[i].reset();
            pacman.addPoint();
            std::cout << "Eaten Food! Total Points: " << pacman.getPoints() << std::endl;
            if(pacman.getPoints() == gameWinPoints){
                std::cout << "\n\nWOW! You have won!\nThe ghosts are not a problem anymore, get all the food you want!!!\n\nPress ESC to exit." << std::endl;
            }
        }
    }

    //ghost shit
    for (int i = 0; i < (int)ghosts.size(); i++) {
        float x = ghosts[i].getX();
        float y = ghosts[i].getY();

        if(collisionCheck(pacman.getX(), pacman.getY(), pR, x, y, 15.0f) && pacman.getPoints() < gameWinPoints){
            std::cout << "\n\nOMG! A ghost got you! Pacman has died.\nHow could you have done this to him?\nGame ENDING... :(" << std::endl;
            gameOver = true;
            glutLeaveMainLoop();
            return;
        }
    }

    if (pacman.getPoints() >= gameWinPoints) {
        bgHue += 2.0f;   // speed of rainbow ig
        if (bgHue > 360.0f) {
            bgHue -= 360.0f;
        }
    }

    glutPostRedisplay();
    if (!gameOver) {
        glutTimerFunc(16, timer, 0);
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Matthew's Pacman Game");

    glutReshapeFunc(reshape);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black background

    glutDisplayFunc(display);
    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(16, timer, 0);
    init(); //setup the game and shit
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}