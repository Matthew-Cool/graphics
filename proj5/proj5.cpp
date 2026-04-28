//---------------------------------------
// Program: ray_trace.cpp
// Author:  John Gauch (modified)
// Date:    Spring 2026
//---------------------------------------
#include <cmath>
#include <cstdio>
#include <cstdlib>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
using namespace std;

#include <iostream>
#include <string>

// Include ray tracing and phong shading classes
#include "ray_classes.h"

// Global variables
#define XDIM 600
#define YDIM 600
#define ZDIM 600
unsigned char image[YDIM][XDIM][3];
float position = -5;
string mode = "phong";
const int SPHERES = 1;
Sphere3D sphere[SPHERES];
ColorRGB color[SPHERES];

float ballAngle = 0.0;
const float FLOOR_Y = -0.67;

// Feature 2 - Axis-aligned boxes/cubes
const int BOXES = 2;

class Box3D
{
public:
   Point3D min_pt;
   Point3D max_pt;

   void set(Point3D minp, Point3D maxp)
   {
      min_pt = minp;
      max_pt = maxp;
   }

   bool get_intersection(Ray3D ray, Point3D &point, Vector3D &normal)
   {
      float tmin = -999999;
      float tmax =  999999;

      Vector3D nmin;
      nmin.set(0, 0, 0);

      // X slab
      if (fabs(ray.dir.vx) < 0.0001)
      {
         if (ray.point.px < min_pt.px || ray.point.px > max_pt.px)
            return false;
      }
      else
      {
         float tx1 = (min_pt.px - ray.point.px) / ray.dir.vx;
         float tx2 = (max_pt.px - ray.point.px) / ray.dir.vx;
         Vector3D nx1, nx2;
         nx1.set(-1, 0, 0);
         nx2.set(1, 0, 0);

         if (tx1 > tx2)
         {
            float temp = tx1; tx1 = tx2; tx2 = temp;
            Vector3D ntemp = nx1; nx1 = nx2; nx2 = ntemp;
         }

         if (tx1 > tmin)
         {
            tmin = tx1;
            nmin = nx1;
         }
         if (tx2 < tmax)
            tmax = tx2;
      }

      // Y slab
      if (fabs(ray.dir.vy) < 0.0001)
      {
         if (ray.point.py < min_pt.py || ray.point.py > max_pt.py)
            return false;
      }
      else
      {
         float ty1 = (min_pt.py - ray.point.py) / ray.dir.vy;
         float ty2 = (max_pt.py - ray.point.py) / ray.dir.vy;
         Vector3D ny1, ny2;
         ny1.set(0, -1, 0);
         ny2.set(0, 1, 0);

         if (ty1 > ty2)
         {
            float temp = ty1; ty1 = ty2; ty2 = temp;
            Vector3D ntemp = ny1; ny1 = ny2; ny2 = ntemp;
         }

         if (ty1 > tmin)
         {
            tmin = ty1;
            nmin = ny1;
         }
         if (ty2 < tmax)
            tmax = ty2;
      }

      // Z slab
      if (fabs(ray.dir.vz) < 0.0001)
      {
         if (ray.point.pz < min_pt.pz || ray.point.pz > max_pt.pz)
            return false;
      }
      else
      {
         float tz1 = (min_pt.pz - ray.point.pz) / ray.dir.vz;
         float tz2 = (max_pt.pz - ray.point.pz) / ray.dir.vz;
         Vector3D nz1, nz2;
         nz1.set(0, 0, -1);
         nz2.set(0, 0, 1);

         if (tz1 > tz2)
         {
            float temp = tz1; tz1 = tz2; tz2 = temp;
            Vector3D ntemp = nz1; nz1 = nz2; nz2 = ntemp;
         }

         if (tz1 > tmin)
         {
            tmin = tz1;
            nmin = nz1;
         }
         if (tz2 < tmax)
            tmax = tz2;
      }

      if (tmin > tmax || tmax < 0)
         return false;

      float t = tmin;
      if (t < 0)
         t = tmax;

      point.px = ray.point.px + t * ray.dir.vx;
      point.py = ray.point.py + t * ray.dir.vy;
      point.pz = ray.point.pz + t * ray.dir.vz;

      normal = nmin;
      normal.normalize();

      return true;
   }
};

Box3D box[BOXES];
ColorRGB box_color[BOXES];

//---------------------------------------
// Calculate random value between [min..max]
//---------------------------------------
float myrand(float min, float max)
{
   return rand() * (max - min) / RAND_MAX + min;
}

//---------------------------------------
// Check to see if point is in shadow
//---------------------------------------
bool in_shadow(Point3D pt, Vector3D dir, int current_sphere, Sphere3D sphere[], int sphere_count,
               int current_box, Box3D box[], int box_count)
{
   Ray3D shadow_ray;
   shadow_ray.set(pt, dir);

   Point3D point;
   Vector3D normal;

   for (int index = 0; index < sphere_count; index++)
      if ((index != current_sphere) &&
         (sphere[index].get_intersection(shadow_ray, point, normal)))
         return true;

   for (int index = 0; index < box_count; index++)
      if ((index != current_box) &&
         (box[index].get_intersection(shadow_ray, point, normal)))
         return true;

   return false;
}

//---------------------------------------
// Feature 3 helper functions for reflection rays
//---------------------------------------
float dot_product(Vector3D a, Vector3D b)
{
   return a.vx * b.vx + a.vy * b.vy + a.vz * b.vz;
}

float distance_squared(Point3D a, Point3D b)
{
   float dx = a.px - b.px;
   float dy = a.py - b.py;
   float dz = a.pz - b.pz;
   return dx * dx + dy * dy + dz * dz;
}

unsigned char clamp_color(float value)
{
   if (value < 0) return 0;
   if (value > 255) return 255;
   return (unsigned char)value;
}

Vector3D get_reflection_dir(Vector3D incoming, Vector3D normal)
{
   incoming.normalize();
   normal.normalize();

   float d = dot_product(incoming, normal);

   Vector3D reflect;
   reflect.vx = incoming.vx - 2 * d * normal.vx;
   reflect.vy = incoming.vy - 2 * d * normal.vy;
   reflect.vz = incoming.vz - 2 * d * normal.vz;
   reflect.normalize();

   return reflect;
}

bool trace_reflection_ray(Ray3D ray, ColorRGB &reflected_color)
{
   int closest_sphere = -1;
   int closest_box = -1;

   Point3D p, closest_p;
   Vector3D n, closest_n;

   float closest_dist = 999999999;

   // Check reflected ray against spheres
   for (int s = 0; s < SPHERES; s++)
   {
      if (sphere[s].get_intersection(ray, p, n))
      {
         float dist = distance_squared(ray.point, p);
         if (dist < closest_dist)
         {
            closest_dist = dist;
            closest_sphere = s;
            closest_box = -1;
            closest_p = p;
            closest_n = n;
         }
      }
   }

   // Check reflected ray against boxes
   for (int b = 0; b < BOXES; b++)
   {
      if (box[b].get_intersection(ray, p, n))
      {
         float dist = distance_squared(ray.point, p);
         if (dist < closest_dist)
         {
            closest_dist = dist;
            closest_sphere = -1;
            closest_box = b;
            closest_p = p;
            closest_n = n;
         }
      }
   }

   // If reflection hits an object, shade it
   if (closest_sphere >= 0 || closest_box >= 0)
   {
      ColorRGB object_color;

      if (closest_sphere >= 0)
         object_color = color[closest_sphere];
      else
         object_color = box_color[closest_box];

      ColorRGB light_color;
      light_color.set(250, 250, 250);

      Vector3D light_dir;
      light_dir.set(-1, -1, -1);
      light_dir.normalize();

      Phong shader;
      shader.SetCamera(ray.point);
      shader.SetLight(light_color, light_dir);

      if (in_shadow(closest_p, light_dir, closest_sphere, sphere, SPHERES, closest_box, box, BOXES))
         shader.SetObject(object_color, 0.4, 0.0, 0.0, 1);
      else
         shader.SetObject(object_color, 0.4, 0.4, 0.4, 10);

      shader.GetShade(closest_p, closest_n, reflected_color);
      return true;
   }

   // If reflection hits the checkerboard floor
   if (ray.dir.vy != 0)
   {
      float t = (FLOOR_Y - ray.point.py) / ray.dir.vy;

      if (t > 0)
      {
         Point3D floor_point;
         floor_point.px = ray.point.px + t * ray.dir.vx;
         floor_point.py = FLOOR_Y;
         floor_point.pz = ray.point.pz + t * ray.dir.vz;

         if (floor_point.pz > -3.0 && floor_point.pz < 6.0 &&
             floor_point.px > -0.88 && floor_point.px < 3.0)
         {
            int check_x = (int)floor((floor_point.px + 100.0) * 4.5);
            int check_z = (int)floor((floor_point.pz + 100.0) * 4.5);

            if ((check_x + check_z) % 2 == 0)
               reflected_color.set(215, 25, 20);
            else
               reflected_color.set(235, 225, 55);

            return true;
         }
      }
   }

   // Otherwise reflect the blue background
   return false;
}

//---------------------------------------
// Perform ray tracing of scene
//---------------------------------------
void ray_trace()
{
   // Define camera point
   Point3D camera;
   camera.set(0, 0.5, position);

   // Define light source
   ColorRGB light_color;
   light_color.set(250, 250, 250);
   Vector3D light_dir;
   light_dir.set(-1, -1, -1);
   light_dir.normalize();

   // Define shader
   Phong shader;
   shader.SetCamera(camera);
   shader.SetLight(light_color, light_dir);

   // Perform ray tracing
   for (int y = 0; y < YDIM; y++)
   for (int x = 0; x < XDIM; x++)
   {
      // Blue background
      image[y][x][0] = 40;
      image[y][x][1] = 120;
      image[y][x][2] = 220;

      // Define sample point on image plane
      float xpos = (x - XDIM/2) * 2.0 / XDIM;
      float ypos = (y - YDIM/2) * 2.0 / YDIM;
      Point3D point;
      point.set(xpos, ypos, 0);

      // Define ray from camera through image
      Ray3D ray;
      ray.set(camera, point);

      // Perform sphere intersection
        // Perform object intersection
        int closest_sphere = -1;
        int closest_box = -1;

        Point3D p, closest_p;
        Vector3D n, closest_n;
        closest_p.set(0, 0, ZDIM);

        // Check spheres
        for (int s = 0; s < SPHERES; s++)
        {
        if ((sphere[s].get_intersection(ray, p, n)) && (p.pz < closest_p.pz))
        {
            closest_sphere = s;
            closest_box = -1;
            closest_p = p;
            closest_n = n;
        }
        }

        // Check boxes/cubes
        for (int b = 0; b < BOXES; b++)
        {
        if ((box[b].get_intersection(ray, p, n)) && (p.pz < closest_p.pz))
        {
            closest_sphere = -1;
            closest_box = b;
            closest_p = p;
            closest_n = n;
        }
        }

      // Checkerboard floor
      if (closest_sphere < 0 && closest_box < 0 && ray.dir.vy != 0)
      {
         float t = (FLOOR_Y - camera.py) / ray.dir.vy;
         if (t > 0)
         {
            Point3D floor_point;
            floor_point.px = camera.px + t * ray.dir.vx;
            floor_point.py = FLOOR_Y;
            floor_point.pz = camera.pz + t * ray.dir.vz;

            // Finite board so blue background shows on sides
            if (floor_point.pz > -3.0 && floor_point.pz < 6.0 &&
                floor_point.px > -0.88 && floor_point.px < 3.0)
            {
               int check_x = (int)floor((floor_point.px + 100.0) * 4.5);
               int check_z = (int)floor((floor_point.pz + 100.0) * 4.5);

               if ((check_x + check_z) % 2 == 0)
               {
                  image[y][x][0] = 215;
                  image[y][x][1] = 25;
                  image[y][x][2] = 20;
               }
               else
               {
                  image[y][x][0] = 235;
                  image[y][x][1] = 225;
                  image[y][x][2] = 55;
               }

               // Cast ball shadow onto floor
               if (in_shadow(floor_point, light_dir, -1, sphere, SPHERES, -1, box, BOXES))
               {
                  image[y][x][0] = (unsigned char)(image[y][x][0] * 0.4);
                  image[y][x][1] = (unsigned char)(image[y][x][1] * 0.4);
                  image[y][x][2] = (unsigned char)(image[y][x][2] * 0.4);
               }
            }
         }
      }

      // Calculate pixel color for sphere hit
      if (closest_sphere >= 0 || closest_box >= 0)
      {
         // Display surface normal
         if (mode == "normal")
         {
            image[y][x][0] = 127 + closest_n.vx * 127;
            image[y][x][1] = 127 + closest_n.vy * 127;
            image[y][x][2] = 127 + closest_n.vz * 127;
         }

         // Calculate Phong shade
         if (mode == "phong")
         {
            ColorRGB object_color;

            if (closest_sphere >= 0)
            object_color = color[closest_sphere];
            else
            object_color = box_color[closest_box];

            if (in_shadow(closest_p, light_dir, closest_sphere, sphere, SPHERES, closest_box, box, BOXES))
            shader.SetObject(object_color, 0.4, 0.0, 0.0, 1);
            else
            shader.SetObject(object_color, 0.4, 0.4, 0.4, 10);

            ColorRGB pixel;
            shader.GetShade(closest_p, closest_n, pixel);

            // Feature 3 - cast one reflection ray
            Vector3D reflection_dir = get_reflection_dir(ray.dir, closest_n);

            Point3D reflection_start;
            reflection_start.px = closest_p.px + closest_n.vx * 0.01;
            reflection_start.py = closest_p.py + closest_n.vy * 0.01;
            reflection_start.pz = closest_p.pz + closest_n.vz * 0.01;

            Point3D reflection_target;
            reflection_target.px = reflection_start.px + reflection_dir.vx;
            reflection_target.py = reflection_start.py + reflection_dir.vy;
            reflection_target.pz = reflection_start.pz + reflection_dir.vz;

            Ray3D reflection_ray;
            reflection_ray.set(reflection_start, reflection_target);

            ColorRGB reflected_color;
            if (trace_reflection_ray(reflection_ray, reflected_color))
            {
                float Ks = 0.35;
                pixel.R = clamp_color((1.0 - Ks) * pixel.R + Ks * reflected_color.R);
                pixel.G = clamp_color((1.0 - Ks) * pixel.G + Ks * reflected_color.G);
                pixel.B = clamp_color((1.0 - Ks) * pixel.B + Ks * reflected_color.B);
            }

            image[y][x][0] = pixel.R;
            image[y][x][1] = pixel.G;
            image[y][x][2] = pixel.B;
         }
      }
   }
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   glClearColor(0.0, 0.0, 0.0, 1.0);

   cout << "Program commands:\n"
        << "   '+' - increase camera distance\n"
        << "   '-' - decrease camera distance\n"
        << "   'p' - show Phong shading\n"
        << "   'n' - show surface normals\n"
        << "   'q' - quit program\n";

   // Solid yellow ball - timer will move it
   Point3D center;
   center.set(0.85, FLOOR_Y + 0.38, 1.5);
   sphere[0].set(center, 0.5);
   color[0].set(255, 220, 0);

    // Feature 2 - two cubes/boxes sitting on the checkerboard
    Point3D bmin, bmax;

    // Red box on the left
    bmin.set(-0.75, FLOOR_Y, 1.0);
    bmax.set(-0.20, FLOOR_Y + 0.7, 1.55);
    box[0].set(bmin, bmax);
    box_color[0].set(220, 40, 40);

    // Green box on the right
    bmin.set(1.15, FLOOR_Y, 2.2);
    bmax.set(1.75, FLOOR_Y + 0.7, 2.8);
    box[1].set(bmin, bmax);
    box_color[1].set(40, 210, 80);

   cout << "camera: 0,0," << position << endl;
   ray_trace();
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   glClear(GL_COLOR_BUFFER_BIT);
   glDrawPixels(XDIM, YDIM, GL_RGB, GL_UNSIGNED_BYTE, image);
   glFlush();
}

//---------------------------------------
// Keyboard callback for OpenGL
//---------------------------------------
void keyboard(unsigned char key, int x, int y)
{
   if (key == 'q')
      exit(0);
   else if (key == '+' && position > -10)
   {
      position -= 0.5;
      cout << "camera: 0,0," << position << endl;
   }
   else if (key == '-' && position < -5)
   {
      position += 0.5;
      cout << "camera: 0,0," << position << endl;
   }
   else if (key == 'n')
      mode = "normal";
   else if (key == 'p')
      mode = "phong";

   ray_trace();
   glutPostRedisplay();
}

//---------------------------------------
// Timer callback for OpenGL
// Moves yellow ball in a circle over the checkerboard
//---------------------------------------
void timer(int value)
{
   ballAngle += 0.04;

   float pathRadius = 1.2;
   float centerZ    = 0.5;

   // Keep ball sitting on the floor as it orbits
   sphere[0].center.px = pathRadius * cos(ballAngle);
   sphere[0].center.py = FLOOR_Y + sphere[0].radius;
   sphere[0].center.pz = centerZ + pathRadius * sin(ballAngle);

   ray_trace();
   glutPostRedisplay();
   glutTimerFunc(10, timer, 0);
}

//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitWindowSize(XDIM, YDIM);
   glutInitWindowPosition(0, 0);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
   glutCreateWindow("Proj5 - mmw");
   init();

   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutTimerFunc(10, timer, 0);
   glutMainLoop();
   return 0;
}