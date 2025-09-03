#pragma once


#include <ncurses.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <algorithm>
#include <sys/ioctl.h> // Required for ioctl
#include <unistd.h>    // Required for STDOUT_FILENO
#include <csignal> 

#include <iostream>  // For input/output operations
#include <fstream>   // For file stream operations (ifstream)
#include <string>    // For string manipulation
#include <vector>    // To store data dynamically
#include <sstream>   // For parsing lines (istringstream)
#include <deque>

struct vec3D{
    float x=0;
    float y=0;
    float z=0;
    float w=1;
};

struct triangle{
    vec3D vertices[3];
};

struct Mesh{
    std::vector<triangle> tris;
};

struct projection{

    float matrix[4][4] = {0.0f};
};

struct mat4x4{
    float m[4][4] = {0.0f};
};

class EngineBackend{
    public:
        EngineBackend();
        bool ConstructConsole();

        // Signal handler function
        void signal_handler(int signum);

        void Close();
        void Start();

        bool virtual OnUserUpdate(float fElapsedTime);

        bool virtual OnUserCreate();

        void ConstructProjectionMatrix();

        void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, triangle a, triangle t);

        void MultiplyMatrixVector(vec3D& i, vec3D& o, mat4x4& m);

        void DrawLine(float x0, float x1, float y0, float y1, triangle a, triangle t);

      
        void PutPixel(int x, int y, char pixel);

        void GetTerminalSize(int& width, int& height);

        int Illumination_calculation(vec3D normal, vec3D illum, triangle a);

        void RasterizeTriangle_EdgeFunction(vec3D normal, vec3D illum, triangle a, triangle triTranslated);

        void Clear_Buffers();

        vec3D BaryCentricCoords(vec3D p, vec3D a, vec3D b, vec3D c);

        std::vector<triangle> Read_File(std::string file_path);

        void Render();

        void Camera_Rotation();

        vec3D normalize(vec3D& vector );

        mat4x4 pointAt(vec3D eye, vec3D target, vec3D up);

        float dot(vec3D& i, vec3D& j);

        vec3D Vector_Mul(vec3D &v1, float k);
        vec3D Vector_Sub(vec3D &v1, vec3D &v2);
        mat4x4 Matrix_QuickInverse(mat4x4 &m);
        vec3D Vector_CrossProduct(vec3D &v1, vec3D &v2);
        vec3D Vector_Add(vec3D &v1, vec3D &v2);

        mat4x4 Matrix_MakeIdentity();

        mat4x4 Matrix_MakeRotationX(float fAngleRad);

        mat4x4 Matrix_MakeRotationY(float fAngleRad);

        mat4x4 Matrix_MakeRotationZ(float fAngleRad);

        mat4x4 Matrix_MakeTranslation(float x, float y, float z);

        mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2);
        vec3D Matrix_MultiplyVector(mat4x4 &m, vec3D &i);
        void Plane_Clipping( vec3D plane,  std::deque<triangle>& tqueue);
        void Clipping(triangle tri);
        void Generate_Planes(mat4x4 matCamera);
        vec3D Vector_Intersect_Plane(vec3D plane, vec3D vector, vec3D point);        
        
    protected:
        int screen_width;
        int screen_height;
        mat4x4 proj; 
        volatile sig_atomic_t program_interrupted = 0;
        float z_far = 1000.0f;
        float z_near = 0.1f;
        float f_fov = 90.0f;
        float f_fov_rad = 1.0f /std::tan((f_fov/2)/180.0f * 3.14159f);
        float q = z_far/(z_far - z_near);
        float fElapsedTime;
        std::vector<float> depth_buffer;
        std::vector<int> color_buffer;
        std::deque<triangle> global_tqueue;
        vec3D left_plane;
        vec3D right_plane;
        vec3D top_plane;
        vec3D bottom_plane;
        vec3D near_plane;
        vec3D far_plane;
        

};