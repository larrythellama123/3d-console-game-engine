#include "EngineBackend.hpp"
#include <iostream>


//issues:
// triangles drawn behind other triangles
//depth buffer - z depth of plane if there is another 
EngineBackend::EngineBackend(){};


bool EngineBackend::ConstructConsole(){
            initscr();
            start_color();
            if (can_change_color()) {
                // Create 8 different intensity levels (colors 8-15)
                for (int i = 0; i < 8; i++) {
                    int intensity = (i * 1000) / 7; // 0 to 1000
                    init_color(8 + i, intensity, intensity, intensity);
                    init_pair( 8 - i , 8 + i, COLOR_BLACK);
                }
            }
            cbreak();
            noecho();
            init_pair(9, COLOR_GREEN, COLOR_BLACK);   
            int width; int height;
            GetTerminalSize(width, height);
            screen_width = width;
            screen_height = height;
            depth_buffer.resize(width * height, std::numeric_limits<float>::max());
            color_buffer.resize(width * height, -1);

            ConstructProjectionMatrix();    
            return true;
        };
        void EngineBackend::Clear_Buffers(){
            std::fill(depth_buffer.begin(), depth_buffer.end(), std::numeric_limits<float>::max());
            std::fill(color_buffer.begin(), color_buffer.end(), -1);
        }

        void EngineBackend::Close(){
            endwin();
        };

        void EngineBackend::Start(){
            OnUserCreate();
            auto start_time = std::chrono::system_clock::now();
            while(true){
                auto end_time = std::chrono::system_clock::now();
                std::chrono::duration<float> elapsedTime = end_time - start_time;
                start_time = end_time;
                fElapsedTime = elapsedTime.count();
                OnUserUpdate(fElapsedTime);
                refresh();
                clear();
            }
        };

        bool EngineBackend::OnUserUpdate(float fElapsedTime){
            return true;
        };

        bool EngineBackend::OnUserCreate(){
            return true;
        };

        void EngineBackend::ConstructProjectionMatrix(){
            float aspect_ratio = static_cast<float>(screen_width/screen_height) * 0.2f;
            proj = new mat4x4;
            proj->m[0][0] = aspect_ratio * f_fov_rad;
            proj->m[1][1] = f_fov_rad;
            proj->m[2][2] = z_far/(z_far - z_near);
            proj->m[3][2] = (-(z_far) * z_near)/(z_far - z_near);
            proj->m[2][3] = 1.0f;  // This should be 1.0f, not the translation
        };

        void EngineBackend::DrawTriangle(float x0, float y0, float x1, float y1, float x2, float y2, triangle a, triangle t){
            attron(COLOR_PAIR(9));
            DrawLine(x0,y0,x1,y1, a, t);
            DrawLine(x1,y1,x2,y2, a, t);
            DrawLine(x2,y2,x0,y0, a, t);
            attroff(COLOR_PAIR(9));
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

        void EngineBackend::DrawLine(float fx0, float fy0, float fx1, float fy1, triangle a, triangle t){

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
                        vec3D p;
                        p.x=x;
                        p.y=y;
                        p.z=0;
                        vec3D bary = BaryCentricCoords(p, a.vertices[0], a.vertices[1], a.vertices[2]);
                        // Interpolate depth using barycentric coordinates
                        float depth = bary.x * t.vertices[0].z + 
                                    bary.y * t.vertices[1].z + 
                                    bary.z * t.vertices[2].z;
                    
                        if(depth_buffer[y*screen_width+x]>depth){
                            depth_buffer[y*screen_width+x]=depth;
                            color_buffer[y*screen_width+x] = 9;
                            // PutPixel(x, y, '*');
                        }
                        
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
        void EngineBackend::Render(){
            for(int i=0; i<screen_width*screen_height; i++){
                if(color_buffer[i]!=-1){
                    attron(COLOR_PAIR(color_buffer[i]));
                    int x_coord = i%screen_width;
                    int y_coord = (i-x_coord)/screen_width;
                    PutPixel(x_coord,y_coord,'*');
                    attroff(COLOR_PAIR(color_buffer[i]));
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

        int EngineBackend::Illumination_calculation(vec3D normal, vec3D illum,  triangle a){
            int color_pair = 0;
            vec3D illum_obj = {(a.vertices[0].x - illum.x),(a.vertices[0].y - illum.y),(a.vertices[0].z - illum.z)};
            float dp = (normal.x*(a.vertices[0].x - illum.x)) + (normal.y*(a.vertices[0].y - illum.y)) + (normal.z*(a.vertices[0].z - illum.z));
            float normal_magnitude = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            float illum_obj_magnitude = sqrtf(illum_obj.x * illum_obj.x + illum_obj.y * illum_obj.y + illum_obj.z * illum_obj.z);
            float cos_theta = dp/(normal_magnitude*illum_obj_magnitude);
            if(dp>0.0f){
                  // Map intensity to one of your 8 pre-defined color pairs (1-8)
                color_pair = static_cast<int>(cos_theta * 7) + 1;
                color_pair = std::max(1, std::min(color_pair, 8));
            }
            return color_pair;
        }

        

        
        

        void EngineBackend::RasterizeTriangle_EdgeFunction(vec3D normal, vec3D illum, triangle a,triangle t){
            
            //find bounding box
            int minX = static_cast<int>(std::min({a.vertices[0].x,a.vertices[1].x,a.vertices[2].x}));
            int maxX = static_cast<int>(std::max({a.vertices[0].x,a.vertices[1].x,a.vertices[2].x}));
            int minY = static_cast<int>(std::min({a.vertices[0].y,a.vertices[1].y,a.vertices[2].y}));
            int maxY = static_cast<int>(std::max({a.vertices[0].y,a.vertices[1].y,a.vertices[2].y}));

            auto EdgeFunction = [](vec3D a, vec3D b, vec3D p) -> int { return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x); };


            int color_pair = Illumination_calculation(normal, illum, a);


            // attron(COLOR_PAIR(color_pair));
            // For triangle with vertices A, B, C (counter-clockwise)
            vec3D p;
            for(int y=minY; y<maxY; y++){
                for(int x=minX; x<maxX; x++){
                    p.x=x;
                    p.y=y;;
                    p.z=0;
                    
                    int w0 = EdgeFunction(a.vertices[1], a.vertices[2], p);  // Edge BC
                    int w1 = EdgeFunction(a.vertices[2], a.vertices[0], p);  // Edge CA  
                    int w2 = EdgeFunction(a.vertices[0], a.vertices[1], p);  // Edge AB

                    if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                        // if(depth_buffer[y*screen_width+x]>depth){
                        //     depth_buffer[y*screen_width+x]=depth;
                        // }
                        // else{
                        //     continue;
                        // }

                        vec3D bary = BaryCentricCoords(p, a.vertices[0], a.vertices[1], a.vertices[2]);
                        // Interpolate depth using barycentric coordinates
                        float depth = bary.x * t.vertices[0].z + 
                                    bary.y * t.vertices[1].z + 
                                    bary.z * t.vertices[2].z;

                        if(depth_buffer[y*screen_width+x]>depth){
                            depth_buffer[y*screen_width+x]=depth;
                            color_buffer[y*screen_width+x] = color_pair;
                            // PutPixel(x, y, '*');
                        }
                    }
                }   
            }
            // attroff(COLOR_PAIR(color_pair));
        }



        vec3D EngineBackend::BaryCentricCoords(vec3D p, vec3D a, vec3D b, vec3D c){
            vec3D v0 = {c.x - a.x, c.y - a.y, 0};
            vec3D v1 = {b.x - a.x, b.y - a.y, 0};
            vec3D v2 = {p.x - a.x, p.y - a.y, 0};
    

            vec3D result = {0.0f,0.0f,0.0f};
            float dot00 = v0.x * v0.x + v0.y * v0.y;
            float dot01 = v0.x * v1.x + v0.y * v1.y;
            float dot02 = v0.x * v2.x + v0.y * v2.y;
            float dot11 = v1.x * v1.x + v1.y * v1.y;        
            float dot12 = v1.x * v2.x + v1.y * v2.y;

            // In matrix form:
            // [v0.x v1.x] [u] [v2.x]
            // [v0.y v1.y] [v] = [v2.y] 
            float D_coeff =  v0.x * v1.y - v0.y * v1.x;

            // In matrix form:
            // [v2.x v1.x] [u] 
            // [v2.y v1.y] [v]
            float D_xmatrix = v2.x * v1.y - v2.y * v1.x;

            // In matrix form:
            // [v0.x v2.x] [u] 
            // [v0.y v2.y] [v] 
            float D_ymatrix = v2.y * v0.x - v2.x * v0.y;

            float u =  D_xmatrix/D_coeff;
            float v =  D_ymatrix/D_coeff;
            float w = 1 -u - v;
            result = {w,v,u};
            return result;
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

        std::vector<triangle> EngineBackend::Read_File(std::string file_path){
            std::ifstream file(file_path); // Replace with your file path
            std::vector<vec3D> vertice_data;
            std::vector<triangle> triangle_data;
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file!" << std::endl;
            }
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string prefix;
                iss >> prefix;

                if(prefix == "v"){
                    float x, y, z;
                    iss >> x >> y >> z;  
                    vec3D vector = {x,y,z};
                    vertice_data.push_back(vector);
                }
                if(prefix == "f"){
                    int x, y, z;
                    iss >> x >> y >> z;
                    triangle tri ={vertice_data[x-1],vertice_data[y-1],vertice_data[z-1]};
                    triangle_data.push_back(tri);
                }
            }
            return triangle_data;
        }

        void EngineBackend::Camera_Rotation(){
            
        }


