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

struct vec3D{
    float x,y,z;
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

        void DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2);

        void MultiplyMatrixVector(vec3D& i, vec3D& o, float m[][4]);

        void DrawLine(float x0, float x1, float y0, float y1);

      
        void PutPixel(int x, int y, char pixel);

        void GetTerminalSize(int& width, int& height);

        int Illumination_calculation(vec3D normal, vec3D illum, triangle a);

        void RasterizeTriangle_EdgeFunction(vec3D normal, vec3D illum, triangle a);

        void Clear_DepthBuffer();

        vec3D BaryCentricCoords(vec3D p, vec3D a, vec3D b, vec3D c);

        std::vector<triangle> Read_File(std::string file_path);
        
        
    protected:
        int screen_width;
        int screen_height;
        mat4x4* proj; 
        volatile sig_atomic_t program_interrupted = 0;
        float z_far = 1000.0f;
        float z_near = 0.1f;
        float f_fov = 90.0f;
        float f_fov_rad = 1.0f /std::tan((f_fov/2)/180.0f * 3.14159f);
        float q = z_far/(z_far - z_near);
        float fElapsedTime;
        std::vector<float> depth_buffer;
};