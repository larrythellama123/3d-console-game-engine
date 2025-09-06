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
                // break;
                
                refresh();
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
            proj.m[0][0] = aspect_ratio * f_fov_rad;
            proj.m[1][1] = f_fov_rad;
            proj.m[2][2] = z_far/(z_far - z_near);
            proj.m[3][2] = (-(z_far) * z_near)/(z_far - z_near);
            proj.m[2][3] = 1.0f;  // This should be 1.0f, not the translation
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
                    PutPixel(x_coord,y_coord,'#');
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

        int EngineBackend::Illumination_calculation(vec3D normal, vec3D illum, triangle a){
            int color_pair = 0;
            
            // Since illum is {0,0,0} in view space, light direction is just negative vertex position
            vec3D light_dir = {-a.vertices[0].x, -a.vertices[0].y, -a.vertices[0].z};
            
            // Normalize light direction
            float light_magnitude = sqrtf(light_dir.x * light_dir.x + light_dir.y * light_dir.y + light_dir.z * light_dir.z);
            if(light_magnitude > 0.0f) {
                light_dir.x /= light_magnitude;
                light_dir.y /= light_magnitude;
                light_dir.z /= light_magnitude;
            }
            
            // Normal should already be normalized from your main loop
            float cos_theta = (normal.x * light_dir.x) + (normal.y * light_dir.y) + (normal.z * light_dir.z);
            
            // Only light surfaces facing the camera
            if(cos_theta > 0.0f) {
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


            int color_pair = std::max(3,Illumination_calculation(normal, illum, t));


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
    
                        vec3D bary = BaryCentricCoords(p, a.vertices[0], a.vertices[1], a.vertices[2]);
                        // Interpolate depth using barycentric coordinates
                        float depth = bary.x * t.vertices[0].z + 
                                    bary.y * t.vertices[1].z + 
                                    bary.z * t.vertices[2].z;

                        if(depth_buffer[y*screen_width+x]>depth){
                            depth_buffer[y*screen_width+x]=depth;
                            color_buffer[y*screen_width+x] = color_pair;
                        }
                    }
                }   
            }
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
                // std::cout<<line_number << " ";
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

        vec3D EngineBackend::Vector_Div(vec3D &v1, float k)
        {
            return { v1.x / k, v1.y / k, v1.z / k };
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

        void EngineBackend::MultiplyMatrixVector(vec3D& i, vec3D& o, mat4x4& m)
        {
            o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
            o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
            o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
            float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

            if (w != 0.0f)
            {
                o.x /= w; o.y /= w; o.z /= w;
            }   
        };

        vec3D EngineBackend::Matrix_MultiplyVector(mat4x4 &m, vec3D &i)
        {
            vec3D v;
            v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
            v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
            v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
            v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
            return v;
        }

        void EngineBackend::Clipping(triangle tri){
            triangle clipped[2];
			std::deque<triangle> listTriangles;
            int nNewTriangles = 1;
            listTriangles.push_back(tri);

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


            for(auto a: listTriangles){
                global_tqueue.push_back(a);
            }
        }

        // void EngineBackend::Plane_Clipping(vec3D plane, std::deque<triangle>& tqueue){
        //     std::vector<vec3D> vqueue;
        //     triangle tri;
        //     int que_len = tqueue.size();
        //     while (que_len>0){
    
        //         tri = tqueue.back();
        //         tqueue.pop_back();
        //         que_len--;
    
        //         vec3D a = tri.vertices[0];
        //         vec3D b = tri.vertices[1];
        //         vec3D c = tri.vertices[2];

        //         bool flag_a = false;
        //         bool flag_b = false;
        //         bool flag_c = false;


                
        //         //check if point a infront of the plane
        //         if(a.x*plane.x + a.y*plane.y +  a.z*plane.z + plane.w < 0){
        //             flag_a = true;
        //         }

        //         if(b.x*plane.x + b.y*plane.y +  b.z*plane.z + plane.w < 0){
        //             flag_b = true;
        //         }
        //         if(c.x*plane.x + c.y*plane.y +  c.z*plane.z + plane.w < 0){
        //             flag_c = true;

        //         }

        //         // if(plane.x==near_plane.x && plane.y==near_plane.y && plane.z==near_plane.z && plane.w==near_plane.w){
        //             // flag_a = !flag_a;
        //             // flag_b = !flag_b;
        //             // flag_c = !flag_c;
        //         // } 
                

                
  

        //         if(flag_a){
        //             vqueue.push_back(a);
        //         }
        //         if((!flag_a && flag_b) || (flag_a && !flag_b) ){
        //             vec3D AB_dir = Vector_Sub(a,b);
        //             vec3D poi = Vector_Intersect_Plane(plane,AB_dir,a);
        //             if(poi.x==0 && poi.y==0 && poi.z==0){  }
        //             else{
        //                 vqueue.push_back(poi);
        //             }
        //         }

        //         if(flag_b){
        //             vqueue.push_back(b);
        //         }
        //         if((!flag_b && flag_c) || (flag_b && !flag_c) ){
          
        //             vec3D BC_dir = Vector_Sub(b,c);
        //             vec3D poi = Vector_Intersect_Plane(plane,BC_dir,b);
        //             if(poi.x==0 && poi.y==0 && poi.z==0){  }
        //             else{
        //                 vqueue.push_back(poi);
        //             }
        
        //         }


        //         if(flag_c){
        //             vqueue.push_back(c);
        //         }
        //         if((!flag_a && flag_c) || (flag_a && !flag_c) ){
        //             vec3D CA_dir = Vector_Sub(c,a);
        //             vec3D poi = Vector_Intersect_Plane(plane,CA_dir,c);
        //             if(poi.x==0 && poi.y==0 && poi.z==0){  }
        //             else{
        //                 vqueue.push_back(poi);
        //             }
        //         }


        //         if(vqueue.size()==3){
        //             triangle t1 = {vqueue[0],vqueue[1], vqueue[2]};
        //             tqueue.push_back(t1);
        //         }


        //          if(vqueue.size()==4){
        //             triangle t1 = {vqueue[0],vqueue[1], vqueue[2]};
        //             triangle t2 = {vqueue[0],vqueue[2], vqueue[3]};
        //             tqueue.push_back(t1);
        //             tqueue.push_back(t2);
        //         }

        //         vqueue.clear();
        //     }

        // }
        

        void EngineBackend::Generate_Planes(mat4x4 matCamera){
            mat4x4 view_proj = Matrix_MultiplyMatrix(matCamera, proj );
            vec3D w_val = {view_proj.m[3][0],view_proj.m[3][1],view_proj.m[3][2],view_proj.m[3][3]};
            vec3D x_val = {view_proj.m[0][0],view_proj.m[0][1],view_proj.m[0][2],view_proj.m[0][3]};
            vec3D y_val = {view_proj.m[1][0],view_proj.m[1][1],view_proj.m[1][2],view_proj.m[1][3]};
            vec3D z_val = {view_proj.m[2][0],view_proj.m[2][1],view_proj.m[2][2],view_proj.m[2][3]};

            

            left_plane  = Vector_Add(w_val,x_val);
            float length = sqrt(left_plane.x*left_plane.x + left_plane.y*left_plane.y + left_plane.z*left_plane.z);
            left_plane.x /= length;
            left_plane.y /= length;
            left_plane.z /= length;
            left_plane.w /= length;

            right_plane  = Vector_Sub(w_val,x_val);
            length = sqrt(right_plane.x*right_plane.x + right_plane.y*right_plane.y + right_plane.z*right_plane.z);
            right_plane.x /= length;
            right_plane.y /= length;
            right_plane.z /= length;
            right_plane.w /= length;

            top_plane  = Vector_Add(w_val,y_val);
            length = sqrt(top_plane.x*top_plane.x + top_plane.y*top_plane.y + top_plane.z*top_plane.z);
            top_plane.x /= length;
            top_plane.y /= length;
            top_plane.z /= length;
            top_plane.w /= length;

            bottom_plane  = Vector_Sub(w_val,y_val);
            length = sqrt(bottom_plane.x*bottom_plane.x + bottom_plane.y*bottom_plane.y + bottom_plane.z*bottom_plane.z);
            bottom_plane.x /= length;
            bottom_plane.y /= length;
            bottom_plane.z /= length;
            bottom_plane.w /= length;
            
            far_plane  = Vector_Add(w_val,z_val);
            length = sqrt(far_plane.x*far_plane.x + far_plane.y*far_plane.y + far_plane.z*far_plane.z);
            far_plane.x /= length;
            far_plane.y /= length;
            far_plane.z /= length;
            far_plane.w /= length;

            near_plane  = Vector_Sub(w_val,z_val);
            length = sqrt(near_plane.x*near_plane.x + near_plane.y*near_plane.y + near_plane.z*near_plane.z);
            near_plane.x /= length;
            near_plane.y /= length;
            near_plane.z /= length;
            near_plane.w /= length;
        }

       

        vec3D EngineBackend::Vector_Normalise(vec3D &v)
        {
            float l = Vector_Length(v);
            return { v.x / l, v.y / l, v.z / l };
        }

        float EngineBackend::Vector_Length(vec3D &v)
        {
            return sqrtf(dot(v, v));
	    }


        vec3D EngineBackend::Vector_IntersectPlane(vec3D &plane_p, vec3D &plane_n, vec3D &lineStart, vec3D &lineEnd)
        {
            plane_n = Vector_Normalise(plane_n);
            float plane_d = -dot(plane_n, plane_p);
            float ad = dot(lineStart, plane_n);
            float bd = dot(lineEnd, plane_n);
            float t = (-plane_d - ad) / (bd - ad);
            vec3D lineStartToEnd = Vector_Sub(lineEnd, lineStart);
            vec3D lineToIntersect = Vector_Mul(lineStartToEnd, t);
            return Vector_Add(lineStart, lineToIntersect);
        }

        int EngineBackend::Triangle_ClipAgainstPlane(vec3D plane_p, vec3D plane_n, triangle &in_tri, triangle &out_tri1, triangle &out_tri2)
        {
            // Make sure plane normal is indeed normal
            plane_n = Vector_Normalise(plane_n);

            // Return signed shortest distance from point to plane, plane normal must be normalised
            auto dist = [&](vec3D &p)
            {
                vec3D n = Vector_Normalise(p);
                return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - dot(plane_n, plane_p));
            };

            // Create two temporary storage arrays to classify points either side of plane
            // If distance sign is positive, point lies on "inside" of plane
            vec3D* inside_points[3];  int nInsidePointCount = 0;
            vec3D* outside_points[3]; int nOutsidePointCount = 0;

            // Get signed distance of each point in triangle to plane
            float d0 = dist(in_tri.vertices[0]);
            float d1 = dist(in_tri.vertices[1]);
            float d2 = dist(in_tri.vertices[2]);

            if (d0 >= 0) { inside_points[nInsidePointCount++] = &in_tri.vertices[0]; }
            else { outside_points[nOutsidePointCount++] = &in_tri.vertices[0]; }
            if (d1 >= 0) { inside_points[nInsidePointCount++] = &in_tri.vertices[1]; }
            else { outside_points[nOutsidePointCount++] = &in_tri.vertices[1]; }
            if (d2 >= 0) { inside_points[nInsidePointCount++] = &in_tri.vertices[2]; }
            else { outside_points[nOutsidePointCount++] = &in_tri.vertices[2]; }

            // Now classify triangle points, and break the input triangle into 
            // smaller output triangles if required. There are four possible
            // outcomes...

            if (nInsidePointCount == 0)
            {
                // All points lie on the outside of plane, so clip whole triangle
                // It ceases to exist

                return 0; // No returned triangles are valid
            }

            if (nInsidePointCount == 3)
            {
                // All points lie on the inside of plane, so do nothing
                // and allow the triangle to simply pass through
                out_tri1 = in_tri;

                return 1; // Just the one returned original triangle is valid
            }

            if (nInsidePointCount == 1 && nOutsidePointCount == 2)
            {
                // Triangle should be clipped. As two points lie outside
                // the plane, the triangle simply becomes a smaller triangle

                
                // The inside point is valid, so keep that...
                out_tri1.vertices[0] = *inside_points[0];

                // but the two new points are at the locations where the 
                // original sides of the triangle (lines) intersect with the plane
                out_tri1.vertices[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
                out_tri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

                return 1; // Return the newly formed single triangle
            }

            if (nInsidePointCount == 2 && nOutsidePointCount == 1)
            {
                // Triangle should be clipped. As two points lie inside the plane,
                // the clipped triangle becomes a "quad". Fortunately, we can
                // represent a quad with two new triangles


                // The first triangle consists of the two inside points and a new
                // point determined by the location where one side of the triangle
                // intersects with the plane
                out_tri1.vertices[0] = *inside_points[0];
                out_tri1.vertices[1] = *inside_points[1];
                out_tri1.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);

                // The second triangle is composed of one of he inside points, a
                // new point determined by the intersection of the other side of the 
                // triangle and the plane, and the newly created point above
                out_tri2.vertices[0] = *inside_points[1];
                out_tri2.vertices[1] = out_tri1.vertices[2];
                out_tri2.vertices[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);

                return 2; // Return two newly formed triangles which form a quad
            }
        }