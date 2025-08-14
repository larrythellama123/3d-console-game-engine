#include "EngineBackend.hpp"
#include <iostream>

EngineBackend::EngineBackend(){};

//projection matrix scales it up
//then i have to scale it bakc down

bool EngineBackend::ConstructConsole(){
            initscr();
            cbreak();
            noecho();
            int width; int height;
            GetTerminalSize(width, height);
            screen_width = width;
            screen_height = height;
            // screen_buffer = new char[screen_width*screen_height];
            // memset(screen_buffer, 0, sizeof(char)*screen_height*screen_width);
            ConstructProjectionMatrix();

            return true;
        };

        void EngineBackend::Close(){
            endwin();
        };

        void EngineBackend::Start(){
            OnUserCreate();
            OnUserUpdate();
        };

        bool EngineBackend::OnUserUpdate(float fElapsedTime){
            return true;
        };

        bool EngineBackend::OnUserCreate(){
            return true;
        };

        void EngineBackend::ConstructProjectionMatrix(){
            proj = new projection;
            proj->matrix[0][0] = proj->aspect_ratio * proj->f_fov_rad;
            proj->matrix[1][1] = proj->f_fov_rad;
            proj->matrix[2][2] = proj->z_far/(proj->z_far - proj->z_near);
            proj->matrix[3][2] = (-(proj->z_far) * proj->z_near)/(proj->z_far - proj->z_near);
            proj->matrix[2][3] = 1.0f;  // This should be 1.0f, not the translation
        };

        void EngineBackend::DrawTriangles(mesh mesh){
            // ConstructProjectionMatrix();
            for(auto tri : mesh.tris){
                triangle* projected_tri = new triangle;
                for(int i=0; i<3; i++){
                    MultiplyMatrixVector( tri.vertices[i],projected_tri->vertices[i], proj->matrix);
                }


                projected_tri->vertices[0].x+=1.0f;projected_tri->vertices[0].y+=1.0f;
                projected_tri->vertices[1].x+=1.0f;projected_tri->vertices[1].y+=1.0f;
                projected_tri->vertices[2].x+=1.0f;projected_tri->vertices[2].y+=1.0f;
                projected_tri->vertices[0].x *= 0.3f  *(float)screen_width;
                projected_tri->vertices[0].y *= 0.3f  *(float)screen_height;
                projected_tri->vertices[1].x *= 0.3f  *(float)screen_width;
                projected_tri->vertices[1].y *= 0.3f  *(float)screen_height;
                projected_tri->vertices[2].x *= 0.3f  *(float)screen_width;
                projected_tri->vertices[2].y *= 0.3f  *(float)screen_height;


                DrawLine(projected_tri->vertices[0].x,projected_tri->vertices[0].y,projected_tri->vertices[1].x,projected_tri->vertices[1].y);
                DrawLine(projected_tri->vertices[1].x,projected_tri->vertices[1].y,projected_tri->vertices[2].x,projected_tri->vertices[2].y);
                DrawLine(projected_tri->vertices[2].x,projected_tri->vertices[2].y,projected_tri->vertices[0].x,projected_tri->vertices[0].y);

            }
            refresh();
            getch();

        };
        

        void EngineBackend::MultiplyMatrixVector(vec3D& i, vec3D& o, float m[][4])
        {
            o.x = i.x * m[0][0] + i.y * m[1][0] + i.z * m[2][0] + m[3][0];
            o.y = i.x * m[0][1] + i.y * m[1][1] + i.z * m[2][1] + m[3][1];
            o.z = i.x * m[0][2] + i.y * m[1][2] + i.z * m[2][2] + m[3][2];
            float w = i.x * m[0][3] + i.y * m[1][3] + i.z * m[2][3] + m[3][3];

            if (w != 0.0f)
            {
                o.x /= w; o.y /= w; o.z /= w;
            }   
        };

        void EngineBackend::DrawLine(float fx0, float fy0, float fx1, float fy1){

            int x0 = static_cast<int>(fx0);
            int x1 = static_cast<int>(fx1);
            int y0 = static_cast<int>(fy0);
            int y1 = static_cast<int>(fy1);
            int dx = abs(x1 - x0);
            int dy = abs(y1 - y0);
            
            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;
            
            int err = dx - dy;
            int x = x0;
            int y = y0;
            
                while (true) {
                    // Check bounds before drawing
                    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
                        PutPixel(x, y, '*');
                    }
                    
                    if (x == x1 && y == y1) break;
                    
                    int e2 = 2 * err;
                    
                    if (e2 > -dy) {
                        err -= dy;
                        x += sx;
                    }
                    
                    if (e2 < dx) {
                        err += dx;
                        y += sy;
                    }
            }
        }

        void EngineBackend::PutPixel(int x, int y, char pixel){
            // Check bounds
            if (x < 0 || x >= screen_width || y < 0 || y >= screen_height) {
                return;
            }
            
            move(y, x);  // ncurses uses (row, col) which is (y, x)
            addch(pixel);
        }

       
        void EngineBackend::GetTerminalSize(int& width, int& height) {
            struct winsize w;
            // Use ioctl to get the window size of the standard output
            if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
                width = w.ws_col;  // Number of columns (width)
                height = w.ws_row; // Number of rows (height)
            } else {
                // Handle error if ioctl fails
                width = 0;
                height = 0;
                std::cerr << "Error getting terminal size." << std::endl;
            }
        }

        void EngineBackend::signal_handler(int signum) {
            if (signum == SIGINT) { // Check if the signal is SIGINT (Ctrl+C)
                program_interrupted = 1;
                std::cout << "\nCtrl+C detected! Shutting down gracefully...\n";
            }
        }