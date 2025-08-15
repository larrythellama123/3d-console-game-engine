#include "EngineBackend.hpp"
using namespace std;


class Engine3D: public EngineBackend{

    public:
        Engine3D(){};
        bool OnUserCreate() override{
            meshCube.tris = {

            // SOUTH
            { 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

            // EAST                                                      
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

            // NORTH                                                     
            { 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

            // WEST                                                      
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

            // TOP                                                       
            { 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
            { 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

            // BOTTOM                                                    
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f },

            };
            return true;
        };
        bool OnUserUpdate(float fElapsedTime) override{ 
            // Set up rotation matrices
            mat4x4 matRotZ, matRotX;
            fTheta += 1.0f * fElapsedTime;

            // Rotation Z
            matRotZ.m[0][0] = cosf(fTheta);
            matRotZ.m[0][1] = sinf(fTheta);
            matRotZ.m[1][0] = -sinf(fTheta);
            matRotZ.m[1][1] = cosf(fTheta);
            matRotZ.m[2][2] = 1;
            matRotZ.m[3][3] = 1;

            // Rotation X
            matRotX.m[0][0] = 1;
            matRotX.m[1][1] = cosf(fTheta * 0.5f);      
            matRotX.m[1][2] = sinf(fTheta * 0.5f);
            matRotX.m[2][1] = -sinf(fTheta * 0.5f);
            matRotX.m[2][2] = cosf(fTheta * 0.5f);
            matRotX.m[3][3] = 1;
            for (auto tri : meshCube.tris)
            {
                triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

                // Rotate in Z-Axis
                MultiplyMatrixVector(tri.vertices[0], triRotatedZ.vertices[0], matRotZ.m);
                MultiplyMatrixVector(tri.vertices[1], triRotatedZ.vertices[1], matRotZ.m);
                MultiplyMatrixVector(tri.vertices[2], triRotatedZ.vertices[2], matRotZ.m);

                // Rotate in X-Axis
                MultiplyMatrixVector(triRotatedZ.vertices[0], triRotatedZX.vertices[0], matRotX.m );
                MultiplyMatrixVector(triRotatedZ.vertices[1], triRotatedZX.vertices[1], matRotX.m);
                MultiplyMatrixVector(triRotatedZ.vertices[2], triRotatedZX.vertices[2], matRotX.m);

                // Offset into the screen
                triTranslated = triRotatedZX;
                triTranslated.vertices[0].z = triRotatedZX.vertices[0].z + 3.0f;
                triTranslated.vertices[1].z = triRotatedZX.vertices[1].z + 3.0f;
                triTranslated.vertices[2].z = triRotatedZX.vertices[2].z + 3.0f;

                // Project triangles from 3D --> 2D
                MultiplyMatrixVector(triTranslated.vertices[0], triProjected.vertices[0], proj->m);
                MultiplyMatrixVector(triTranslated.vertices[1], triProjected.vertices[1], proj->m);
                MultiplyMatrixVector(triTranslated.vertices[2], triProjected.vertices[2], proj->m);

                // Scale into view
                triProjected.vertices[0].x += 1.0f; triProjected.vertices[0].y += 1.0f;
                triProjected.vertices[1].x += 1.0f; triProjected.vertices[1].y += 1.0f;
                triProjected.vertices[2].x += 1.0f; triProjected.vertices[2].y += 1.0f;
                triProjected.vertices[0].x *= 0.5f * (float)screen_width;
                triProjected.vertices[0].y *= 0.5f * (float)screen_height;
                triProjected.vertices[1].x *= 0.5f * (float)screen_width;
                triProjected.vertices[1].y *= 0.5f * (float)screen_height;
                triProjected.vertices[2].x *= 0.5f * (float)screen_width;
                triProjected.vertices[2].y *= 0.5f * (float)screen_height;

                // Rasterize triangle
                DrawTriangle(triProjected.vertices[0].x, triProjected.vertices[0].y,
                    triProjected.vertices[1].x, triProjected.vertices[1].y,
                    triProjected.vertices[2].x, triProjected.vertices[2].y);

            }
            clear();
            return true;
        }

        

    private:
        mesh meshCube;
        float fTheta;


};

int main(){
    Engine3D engine3D;
    // signal(SIGINT, &engine3D.signal_handler());

    if(engine3D.ConstructConsole()){
        engine3D.Start();
    };
    engine3D.Close();
    return 0;
}