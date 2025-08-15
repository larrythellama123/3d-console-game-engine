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
            ConstructProjectionMatrix();    
            return true;
        };

        void EngineBackend::Close(){
            endwin();
        };

        void EngineBackend::Start(){
            OnUserCreate();
            auto start_time = std::chrono::steady_clock::now();
            while(true){
                auto end_time = std::chrono::steady_clock::now();
                // auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
                std::chrono::duration<float> elapsedTime = end_time - start_time;
                start_time = end_time;
                fElapsedTime = elapsedTime.count();
                // std::chrono::duration<double>
                // fElapsedTime = static_cast<float>(duration.count());   
                // std::cout<<fElapsedTime;
            
                OnUserUpdate(fElapsedTime); 
            }
        };

        bool EngineBackend::OnUserUpdate(float fElapsedTime){
            return true;
        };

        bool EngineBackend::OnUserCreate(){
            return true;
        };

        void EngineBackend::ConstructProjectionMatrix(){
            proj = new mat4x4;
            proj->m[0][0] = aspect_ratio * f_fov_rad;
            proj->m[1][1] = f_fov_rad;
            proj->m[2][2] = z_far/(z_far - z_near);
            proj->m[3][2] = (-(z_far) * z_near)/(z_far - z_near);
            proj->m[2][3] = 1.0f;  // This should be 1.0f, not the translation
        };

        void EngineBackend::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2){
            DrawLine(x0,y0,x1,y1);
            DrawLine(x1,y1,x2,y2);
            DrawLine(x2,y2,x0,y0);
            refresh();
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