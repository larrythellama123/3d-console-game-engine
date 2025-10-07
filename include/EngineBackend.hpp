#pragma once

#include <opencv2/opencv.hpp>

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
#include <tuple>

struct vec3D{
    float x=0;
    float y=0;
    float z=0;
    float w=1;
};

struct vec2D{
    float u=0;
    float v=0;
    float z=0;
};

struct triangle{
    vec3D vertices[3];
    vec2D t[3];
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
int QuantizeChannel(int value, int levels);
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

        void Scanline_Rasterize(vec3D normal, vec3D illum, triangle a,triangle t);

        void Clear_Buffers();

        vec3D BaryCentricCoords(vec3D p, vec3D a, vec3D b, vec3D c);
        vec2D BaryCentricCoords_2D(vec2D p, vec2D a, vec2D b, vec2D c);

        std::vector<triangle> Read_File(std::string file_path);

        void Render();


        vec3D normalize(vec3D& vector );

        mat4x4 pointAt(vec3D eye, vec3D target, vec3D up);

        float dot(vec3D& i, vec3D& j);

        vec3D Vector_Mul(vec3D &v1, float k);
        vec3D Vector_Sub(vec3D &v1, vec3D &v2);
        mat4x4 Matrix_QuickInverse(mat4x4 &m);
        vec3D Vector_CrossProduct(vec3D &v1, vec3D &v2);
        vec3D Vector_Add(vec3D &v1, vec3D &v2);
        vec3D Vector_Normalise(vec3D &v);
        float Vector_Length(vec3D &v);
        vec3D Vector_Div(vec3D &v1, float k);
        vec2D Vector_2D_Add(vec2D &v1, vec2D &v2);
        vec2D Vector_2D_Sub(vec2D& v1, vec2D& v2);
        vec2D Vector_2D_Mul(vec2D &v1, float k);
        

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
        // vec3D Vector_Intersect_Plane(vec3D plane, vec3D vector, vec3D point);        
        std::tuple<vec3D, float> Vector_IntersectPlane(vec3D &plane_p, vec3D &plane_n, vec3D &lineStart, vec3D &lineEnd);
        int Triangle_ClipAgainstPlane(vec3D plane_p, vec3D plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2);
        int Sample_PNG(float u , float v);
        void generate_3d_plane();
        void generate_perlin();
        
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
        int count = 16;
        cv::Mat image;
        std::unordered_map<uint32_t, int> color_cache; // RGB -> color_pair_id
        int next_color_id = 10;
        int32_t* pixels;

};