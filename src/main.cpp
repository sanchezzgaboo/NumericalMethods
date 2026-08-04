#include <glad/gl.h>
#include <GLFW/glfw3.h>


#include <imgui.h>
#include <implot.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <filesystem>
#include <fstream>
#include <sstream>

GLuint CompileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);

    const char* src = source.c_str();

    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char info[1024];

        glGetShaderInfoLog(shader, sizeof(info), nullptr, info);

        std::cerr << info << std::endl;

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

GLuint CreateProgram(
    const std::string& vertex,
    const std::string& fragment)
{
    GLuint vs = CompileShader(GL_VERTEX_SHADER, vertex);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fragment);

    if (!vs || !fs)
    {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }
    GLuint program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint success;

    glGetProgramiv(
        program,
        GL_LINK_STATUS,
        &success);

    if (!success)
    {
        char info[1024];
        glGetProgramInfoLog(program, sizeof(info), nullptr, info);

        std::cerr << info << '\n';

        glDeleteShader(vs);
        glDeleteShader(fs);
        glDeleteProgram(program);

        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open: " + path.string());
    }

    std::stringstream ss;
    ss << file.rdbuf();

    return ss.str();
}

bool ReloadShader(GLuint& shader,
                  const std::filesystem::path& vert,
                  const std::filesystem::path& frag)
{
    try
    {
        std::cout << "Reload begin\n";

        auto vertSrc = ReadFile(vert);
        std::cout << "Vertex read\n";

        auto fragSrc = ReadFile(frag);
        std::cout << "Fragment read\n";

        GLuint newShader = CreateProgram(vertSrc, fragSrc);
        std::cout << "Program created: " << newShader << '\n';

        if (newShader == 0)
        {
            std::cout << "Compilation failed\n";
            return false;
        }

        glDeleteProgram(shader);
        shader = newShader;

        std::cout << "Reload complete\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << '\n';
        return false;
    }
}

struct Point2D
{
    std::vector<float> x;
    std::vector<float> y;
};
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};
struct RollingBuffer {
    float Span;
    ImVector<ImVec2> Data;
    RollingBuffer() {
        Span = 10.0f;
        Data.reserve(2000);
    }
    void AddPoint(float x, float y) {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};
float func(float x, float y1, float y2){ //PLACE AS MANY y as the order of the diff. eq.
    
    //place the function that solves the highest order derivative 
    // Such as: d^n/dt^n = f(t,y,y',y'',...,y^(n-1))
    // EX: y''+2y'-3y=5
    // y'' = 5-2y'+3y
    return {-30*y1}; 
}

Point2D forward_euler(float x0, float y1_x0, float y2_x0, float x_final){ //place as many initial conditions as needed for the IVP
    Point2D numbers;
    float step = 1e-4;
    while (x0<x_final){
        y1_x0 = y1_x0+step*y2_x0;
        y2_x0 = y2_x0+step*(func(x0, y1_x0, y2_x0));
        numbers.x.push_back(x0);
        numbers.y.push_back(y1_x0);
        x0=x0+step;
    }    
    return numbers;
}

Point2D discretize_func(float initial_point, float final_point, float n_steps){
    Point2D numbers;
    float start = initial_point;
    for (int i = 0; i<=n_steps;i++){
            numbers.x.push_back(start + (final_point-initial_point)/n_steps*i);
            numbers.y.push_back(exp(-1*(pow(numbers.x[i],2)))); //FUNCTION TO DISCRETIZE
            std::cout << numbers.x[i] << ", " << numbers.y[i] << std::endl;
    }

    return numbers;
}

std::vector<Point2D> FTCS(float initial_point, float final_point, float n_steps, float alpha, float t_final){ //place as many initial conditions as needed for the IVP
    float timestep = 1e-3;//pow(final_point-initial_point,2)/(2*alpha);
    
    std::vector<Point2D> sol;

    Point2D numbers = discretize_func(initial_point, final_point, n_steps);
    
    std::vector<float> current_temp = numbers.y;
    for (int j = 0; j*timestep <= t_final; j++){
        if (j == 0){
            sol.push_back(numbers);
        }
        else{
            std::vector<float> res;
            for(int k = 0; k <= n_steps; k++){
                // Boundary Init
                if(k == 0){
                    res.push_back(current_temp[k] + alpha*timestep/((final_point-initial_point)/n_steps)*(0.5-current_temp[k]));
                }
                //Boundary Final
                else if (k == n_steps){
                    res.push_back(current_temp[k] + alpha*timestep/((final_point-initial_point)/n_steps)*(0.5-current_temp[k]));
                }
                else{
                    res.push_back(current_temp[k] + (alpha*timestep/((final_point-initial_point)/n_steps))*(current_temp[k+1]-2*current_temp[k]+current_temp[k-1]));
                }
            }
            sol.push_back({numbers.x, res});
            current_temp = res;
        }
    }


    return sol;
}



int main(){
    //init PARAM
    float theta = acos(0.0);
    float omega = 3;


    Eigen::Matrix3d A;

    A << 2, 1, 3,
         2, 4, 4,
         1, 4, 5;

    std::cout << A << std::endl;
    //INIT GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window =
        glfwCreateWindow(1280, 720,
                         "Numerical Methods",
                         nullptr,
                         nullptr);
    //INIT WINDOW
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //INIT GLAD
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    //CANVAS VERTICES POSITION
    constexpr float quadVertices[] =
    {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f,
        -1.0f,  1.0f
    };
    //CANVAS VERTICES ORIENTATION
    constexpr unsigned int quadIndices[] =
    {
        0, 1, 2,
        2, 3, 0
    };
    
    std::cout << "Vendor   : " << glGetString(GL_VENDOR) << '\n';
    std::cout << "Renderer : " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Version  : " << glGetString(GL_VERSION) << '\n';

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(quadVertices),
        quadVertices,
        GL_STATIC_DRAW);

    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        sizeof(quadIndices),
        quadIndices,
        GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(
        0,                  // location
        2,                  // x,y
        GL_FLOAT,
        GL_FALSE,
        2*sizeof(float),
        (void*)0);

    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    std::cout << "TEST" << std::endl;
    std::cout << std::filesystem::current_path() << '\n';
    std::cout << "Reading vertex...\n";
    const std::filesystem::path vertexPath = "Shaders/fullscreen.vert";
    const std::filesystem::path fragmentPath = "Shaders/pendulum.frag";

    auto vert = ReadFile(vertexPath);
    std::cout << "Reading fragment...\n";
    auto frag = ReadFile(fragmentPath);

    std::cout << "Vertex size = " << vert.size() << '\n';
    std::cout << "Fragment size = " << frag.size() << '\n';

    std::cout << "Calling CreateProgram...\n";
    GLuint shader = CreateProgram(vert, frag);

    std::cout << "Done\n";

    //UNIFORMS TO SEND TO SHADER

    GLint resolutionLoc = glGetUniformLocation(shader, "uResolution");
    GLint timeLoc       = glGetUniformLocation(shader, "uTime");
    GLint thetaLoc       = glGetUniformLocation(shader, "uTheta");

    constexpr float physics_dt = 1.0f / 240.0f;
    float accumulator = 0.0f;
    float lastTime = glfwGetTime();
    static RollingBuffer data_theta;
    static RollingBuffer data_x;
    static RollingBuffer data_y;
    static float history = 10.0f;
    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        accumulator += currentTime - lastTime;
        lastTime = currentTime;

        while (accumulator >= physics_dt)
        {
            // Forward Euler
            theta += physics_dt * omega;
            omega += physics_dt * (-0.0*omega - 9.77 / 0.6 * std::sin(theta));

            accumulator -= physics_dt;
        }
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("pendulum plot");
        data_theta.AddPoint(currentTime, theta);
        data_theta.Span = static_cast<double>(history);
        data_x.AddPoint(currentTime, 0.6*sin(theta));
        data_x.Span = static_cast<double>(history);
        data_y.AddPoint(currentTime, -0.6*cos(theta));
        data_y.Span = static_cast<double>(history);

        ImGui::SliderFloat("History",&history,1,30,"%.1f s");
        if (ImPlot::BeginPlot("Movement",ImVec2(-1,ImGui::GetTextLineHeight()*30))){
            ImPlot::SetupAxes(nullptr, "Radians", flags);
            ImPlot::SetupAxisLimits(ImAxis_X1,0, static_cast<double>(history), ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1,-3.14,3.14);
            ImPlotSpec spec;
            spec.Offset = 0;
            spec.Stride = 2 * sizeof(float);
            ImPlot::PlotLine("Theta", &data_theta.Data[0].x, &data_theta.Data[0].y, data_theta.Data.size(), spec);
            ImPlot::PlotLine("X", &data_x.Data[0].x, &data_x.Data[0].y, data_x.Data.size(), spec);
            ImPlot::PlotLine("Y", &data_y.Data[0].x, &data_y.Data[0].y, data_y.Data.size(), spec);
            ImPlot::EndPlot();
        }
        
        ImGui::End();
        ImGui::Render();
        static bool pressed = false;
        // NO FUNCIONA EL RELOAD!!!
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !pressed)
        {
            pressed = true;

            ReloadShader(shader,
                        vertexPath,
                        fragmentPath);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
        {
            pressed = false;
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        //sending uniforms
        glUniform1f(timeLoc, physics_dt);
        glUniform1f(thetaLoc, theta);

        glUniform2f(resolutionLoc, display_w, display_h);

        glBindVertexArray(VAO);

        glDrawElements(
            GL_TRIANGLES,
            6,
            GL_UNSIGNED_INT,
            nullptr);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    ImPlot::DestroyContext();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader);

    glfwTerminate();
}

