#include "EngineBackend.hpp"
using namespace std;


class Engine3D: public EngineBackend{

    public:
        Engine3D(){};
        bool OnUserCreate() override{
            perlin();
            generate_3d_plane();

            mesh.tris = Read_File("../build/flat_plane.obj");


            // mesh.tris = {
            //     // SOUTH
            //     { 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,}, 
            //     { 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
                                                                                                            
            //     // EAST           																			   
            //     { 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
            //     { 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
                                                                                                            
            //     // NORTH           																			   
            //     { 1.0f, 0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
            //     { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
                                                                                                            
            //     // WEST            																			   
            //     { 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
            //     { 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
                                                                                                            
            //     // TOP             																			   
            //     { 0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
            //     { 0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
                                                                                                            
            //     // BOTTOM          																			  
            //     { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 1.0f,},
            //     { 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 1.0f,		1.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f,},
            // };


            return true;
        };
        bool OnUserUpdate(float fElapsedTime) override{ 


            Clear_Buffers();
            global_tqueue.clear();
            mat4x4 matRotZ, matRotX;
            // fTheta += 1.0f * fElapsedTime; // Uncomment to spin around
            matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
            matRotX = Matrix_MakeRotationX(fTheta);

            mat4x4 matTrans;
            matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

            mat4x4 matWorld;
            matWorld = Matrix_MakeIdentity();	// Form World Matrix
            matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX); // Transform by rotation
            matWorld = Matrix_MultiplyMatrix(matWorld, matTrans); // Transform by translation

            vec3D vTarget = {0,0,1};

            mat4x4 matCameraRotP = Matrix_MakeRotationX(fPitch);
            vlookDir = Matrix_MultiplyVector(matCameraRotP, vTarget);

            mat4x4 matCameraRotY = Matrix_MakeRotationY(fYaw);
            vlookDir = Matrix_MultiplyVector(matCameraRotY, vlookDir);


            vTarget = Vector_Add(camera, vlookDir);
            mat4x4 matCamera = pointAt(camera, vTarget, vup);
            // Make view matrix from camera
            mat4x4 matView = Matrix_QuickInverse(matCamera);

            for (auto tri : mesh.tris)
            {
                triangle  triViewed, triWorldSpace;

                MultiplyMatrixVector(tri.vertices[0], triWorldSpace.vertices[0], matWorld);
                MultiplyMatrixVector(tri.vertices[1], triWorldSpace.vertices[1], matWorld);
                MultiplyMatrixVector(tri.vertices[2], triWorldSpace.vertices[2], matWorld);
                triWorldSpace.t[0] = tri.t[0];
                triWorldSpace.t[1] = tri.t[1];
                triWorldSpace.t[2] = tri.t[2];


                MultiplyMatrixVector(triWorldSpace.vertices[0], triViewed.vertices[0], matView);
                MultiplyMatrixVector(triWorldSpace.vertices[1], triViewed.vertices[1], matView);
                MultiplyMatrixVector(triWorldSpace.vertices[2], triViewed.vertices[2], matView);
                triViewed.t[0] = triWorldSpace.t[0];
                triViewed.t[1] = triWorldSpace.t[1]; 
                triViewed.t[2] = triWorldSpace.t[2]; 


                
                triangle clipped[2];
                int clipped_tris = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.01f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);
                for (int n = 0; n < clipped_tris; n++)
				{
                    triangle triTranslated,triProjected;
                    triTranslated = clipped[n];
                    vec3D line1, line2;
                    line1.x = triTranslated.vertices[0].x - triTranslated.vertices[1].x;
                    line1.y = triTranslated.vertices[0].y - triTranslated.vertices[1].y;
                    line1.z = triTranslated.vertices[0].z - triTranslated.vertices[1].z;

                    line2.x = triTranslated.vertices[0].x - triTranslated.vertices[2].x;
                    line2.y = triTranslated.vertices[0].y - triTranslated.vertices[2].y;
                    line2.z = triTranslated.vertices[0].z - triTranslated.vertices[2].z;

                    //cross product
                    normal.x = line1.y * line2.z - line1.z * line2.y; // Cx
                    normal.y = line1.z * line2.x - line1.x * line2.z; // Cy
                    normal.z = line1.x * line2.y - line1.y * line2.x; // Cz
                    
                    //normalize
                    float magnitude = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
                    if (magnitude > 0.0f) { // Avoid division by zero for zero vectors
                        normal.x /= magnitude;
                        normal.y /= magnitude;  
                        normal.z /= magnitude;
                    }

                    // check if this works for backface culling
                    if(((normal.x*(triTranslated.vertices[0].x-vlookDir.x)) + (normal.y*(triTranslated.vertices[0].y - vlookDir.y))
                        + (normal.z*(triTranslated.vertices[0].z- vlookDir.z)))>0.0f){
                            continue;
                    }

                   
                    // Project triangles from 3D --> 2D
                    MultiplyMatrixVector(triTranslated.vertices[0], triProjected.vertices[0], proj);
                    MultiplyMatrixVector(triTranslated.vertices[1], triProjected.vertices[1], proj);
                    MultiplyMatrixVector(triTranslated.vertices[2], triProjected.vertices[2], proj);
                    triProjected.t[0] = triTranslated.t[0];
                    triProjected.t[1] = triTranslated.t[1];
                    triProjected.t[2] = triTranslated.t[2];


                    triProjected.vertices[0] = Vector_Div(triProjected.vertices[0], triProjected.vertices[0].w);
                    triProjected.vertices[1] = Vector_Div(triProjected.vertices[1], triProjected.vertices[1].w);
                    triProjected.vertices[2] = Vector_Div(triProjected.vertices[2], triProjected.vertices[2].w);

                    triProjected.vertices[0].x *= -1.0f;
                    triProjected.vertices[1].x *= -1.0f;
                    triProjected.vertices[2].x *= -1.0f;
                    triProjected.vertices[0].y *= -1.0f;
                    triProjected.vertices[1].y *= -1.0f;
                    triProjected.vertices[2].y *= -1.0f;

                    vec3D vOffsetView = { 1,1,0 };
					triProjected.vertices[0] = Vector_Add(triProjected.vertices[0], vOffsetView);
					triProjected.vertices[1] = Vector_Add(triProjected.vertices[1], vOffsetView);
					triProjected.vertices[2] = Vector_Add(triProjected.vertices[2], vOffsetView);
                    // Scale into view
                    triProjected.vertices[0].x *= 0.5f * (float)screen_width;
                    triProjected.vertices[0].y *= 0.5f * (float)screen_height;
                    triProjected.vertices[1].x *= 0.5f* (float)screen_width;
                    triProjected.vertices[1].y *= 0.5f * (float)screen_height;
                    triProjected.vertices[2].x *= 0.5f * (float)screen_width;
                    triProjected.vertices[2].y *= 0.5f * (float)screen_height;

                    triangle clipped[2];
                    std::deque<triangle> listTriangles;
                    int nNewTriangles = 1;
                    listTriangles.push_back(triProjected);
                    
                    for (int p = 0; p < 4; p++)
                    {
                        int nTrisToAdd = 0;
                        while (nNewTriangles > 0)
                        {

                            // Take triangle from front of queue
                            triangle test = listTriangles.front();
                            listTriangles.pop_front();
                            nNewTriangles--;

                
                            switch (p)
                            {
                            case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                            case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)screen_height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                            case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                            case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)screen_width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
                            default: break;
                            }

                            for (int w = 0; w < nTrisToAdd; w++)
                                listTriangles.push_back(clipped[w]);
                        }
                        nNewTriangles = listTriangles.size();
                    }

                    for(auto tri: listTriangles){
                        DrawTriangle(tri.vertices[0].x, tri.vertices[0].y,
                        tri.vertices[1].x, tri.vertices[1].y,
                        tri.vertices[2].x, tri.vertices[2].y, tri,triTranslated);

                        RasterizeTriangle_EdgeFunction(normal,illumination,tri, triTranslated);
                    }
                }
            }
           
            Render();
            
            int ch;
            ch = getch();
            if (ch != ERR) {
                clear();
                vec3D vForward = Vector_Mul(vlookDir, 400.0f * fElapsedTime);
                if ((char)ch == 'w') {
                    camera = Vector_Add(vForward,camera);
                }
                if ((char)ch == 's') {
                    camera = Vector_Sub(camera,vForward);
                }


                if ((char)ch == 'a') {
                    camera.x += 400.0f* fElapsedTime;
                }
                if ((char)ch == 'd') {
                    camera.x -= 400.0f* fElapsedTime;
                }


                //yaw
                if ((char)ch == 't') {
                    fYaw += 200.0f* fElapsedTime;
                }
                if ((char)ch == 'g') {
                    fYaw -= 200.0f* fElapsedTime;
                }

                //move up
                if ((char)ch == 'u') {
                    camera.y += 400.0f* fElapsedTime;
                }
                if ((char)ch == 'j') {
                    camera.y -= 400.0f* fElapsedTime;
                }


                //pitch
                if ((char)ch == 'p') {
                    fPitch += 100.0f* fElapsedTime;
                }
                if ((char)ch == 'l') {
                    fPitch -= 100.0f* fElapsedTime;
                }

                
            }



            // refresh();
            // clear();
            return true;
        }

        

    private:
        Mesh mesh;
        float fTheta;
        vec3D illumination=  { 0.0f, 1.0f, -1.0f };
        vec3D normal;
        vec3D vlookDir = {0.0f, -2.0f, 1.0f};
        vec3D vup = {0.0f,1.0f,0.0f};
        vec3D camera = {0.0f,-1.0f, 0.0f} ;
        float fYaw = 0;
        float fPitch = 0;

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