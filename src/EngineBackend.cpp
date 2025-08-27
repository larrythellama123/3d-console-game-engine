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
            keypad(stdscr, TRUE);
            nodelay(stdscr, TRUE);
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

            triangle_data.reserve(300000); // Adjust based on expected size
            
            vertice_data.reserve(500000); // Pre-allocate reasonable size
                
            int line_number = 0;
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file!" << std::endl;
            }
            std::string line;
            while (std::getline(file, line)) {
                line_number++;
                std::cout<<line_number << " ";
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

                    if (iss >> x >> y >> z) {


                        // Convert to 0-based indexing and check bounds
                        int idx1 = x - 1;
                        int idx2 = y - 1; 
                        int idx3 = z - 1;
                        
                        // Bounds checking
                        if (idx1 >= 0 && idx1 < vertice_data.size() &&
                            idx2 >= 0 && idx2 < vertice_data.size() &&
                            idx3 >= 0 && idx3 < vertice_data.size()) {
                            
                            triangle tri = {vertice_data[idx1], vertice_data[idx2], vertice_data[idx3]};
                            triangle_data.push_back(tri);
                        } else {
                            std::cerr << "Error: Invalid face indices on line " << line_number 
                                    << " (indices: " << x << "," << y << "," << z 
                                    << ", vertex count: " << vertice_data.size() << ")" << std::endl;
                        }
                    } else {
                        std::cerr << "Warning: Invalid face on line " << line_number << std::endl;
                    }

                    // iss >> x >> y >> z;
                    // // std::cout<<x-1<<" x";

                    // std::cout<<y-1<<" y";
                    // triangle tri ={vertice_data[x-1],vertice_data[y-1],vertice_data[z-1]};
                    // triangle_data.push_back(tri);
                }
            }
            return triangle_data;
        }

        void EngineBackend::Camera_Rotation(){
            //shld i find all the points in 3d space
            
        }

        mat4x4 EngineBackend::pointAt(vec3D pos, vec3D target, vec3D up) {
            vec3D f = Vector_Sub(target, pos);
            vec3D newForward = normalize(f);
            vec3D a = Vector_Mul(newForward, dot(up, newForward));
            vec3D newUp = Vector_Sub(up, a);
            newUp =  normalize(newUp);
            vec3D r = Vector_CrossProduct(newUp, newForward);
            vec3D newRight = normalize(r);
            vec3D u = Vector_CrossProduct(newRight, newForward);

            mat4x4 matrix;
            matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
            matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
            matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
            matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;			matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
            return matrix;
        }

        float EngineBackend::dot(vec3D& i, vec3D& j){
            float res = i.x * j.x + i.y * j.y + i.z * j.z;
            return res;
        }   

        vec3D EngineBackend::normalize(vec3D& vector){
            float magnitude = sqrtf(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
            if (magnitude > 0.0f) { // Avoid division by zero for zero vectors
                vector.x /= magnitude;
                vector.y /= magnitude;
                vector.z /= magnitude;
            }
            return vector;
        }

        vec3D EngineBackend::Vector_Mul(vec3D &v1, float k)
        {
            return { v1.x * k, v1.y * k, v1.z * k };
        }

        vec3D EngineBackend::Vector_Sub(vec3D& v1, vec3D& v2)
        {
            return {v1.x-v2.x, v1.y-v2.y, v1.z-v2.z};
        }


        mat4x4 EngineBackend::Matrix_QuickInverse(mat4x4 &m) 
        {
            mat4x4 matrix;
            matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
            matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
            matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
            matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
            matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
            matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
            matrix.m[3][3] = 1.0f;
            return matrix;
        }

        vec3D EngineBackend::Vector_CrossProduct(vec3D &v1, vec3D &v2)
        {
            vec3D v;
            v.x = v1.y * v2.z - v1.z * v2.y;
            v.y = v1.z * v2.x - v1.x * v2.z;
            v.z = v1.x * v2.y - v1.y * v2.x;
            return v;
        }

        vec3D EngineBackend::Vector_Add(vec3D &v1, vec3D &v2)
        {
            return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
        }

        mat4x4 EngineBackend::Matrix_MakeIdentity()
        {
            mat4x4 matrix;
            matrix.m[0][0] = 1.0f;
            matrix.m[1][1] = 1.0f;
            matrix.m[2][2] = 1.0f;
            matrix.m[3][3] = 1.0f;
            return matrix;
        }

        mat4x4 EngineBackend::Matrix_MakeRotationX(float fAngleRad)
        {
            mat4x4 matrix;
            matrix.m[0][0] = 1.0f;
            matrix.m[1][1] = cosf(fAngleRad);
            matrix.m[1][2] = sinf(fAngleRad);
            matrix.m[2][1] = -sinf(fAngleRad);
            matrix.m[2][2] = cosf(fAngleRad);
            matrix.m[3][3] = 1.0f;
            return matrix;
        }

        mat4x4 EngineBackend::Matrix_MakeRotationY(float fAngleRad)
        {
            mat4x4 matrix;
            matrix.m[0][0] = cosf(fAngleRad);
            matrix.m[0][2] = sinf(fAngleRad);
            matrix.m[2][0] = -sinf(fAngleRad);
            matrix.m[1][1] = 1.0f;
            matrix.m[2][2] = cosf(fAngleRad);
            matrix.m[3][3] = 1.0f;
            return matrix;
        }

        mat4x4 EngineBackend::Matrix_MakeRotationZ(float fAngleRad)
        {
            mat4x4 matrix;
            matrix.m[0][0] = cosf(fAngleRad);
            matrix.m[0][1] = sinf(fAngleRad);
            matrix.m[1][0] = -sinf(fAngleRad);
            matrix.m[1][1] = cosf(fAngleRad);
            matrix.m[2][2] = 1.0f;
            matrix.m[3][3] = 1.0f;
            return matrix;
        }

        mat4x4 EngineBackend::Matrix_MakeTranslation(float x, float y, float z)
        {
            mat4x4 matrix;
            matrix.m[0][0] = 1.0f;
            matrix.m[1][1] = 1.0f;
            matrix.m[2][2] = 1.0f;
            matrix.m[3][3] = 1.0f;
            matrix.m[3][0] = x;
            matrix.m[3][1] = y;
            matrix.m[3][2] = z;
            return matrix;
        }

        mat4x4 EngineBackend::Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
        {
            mat4x4 matrix;
            for (int c = 0; c < 4; c++)
                for (int r = 0; r < 4; r++)
                    matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
            return matrix;
        }

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

            


