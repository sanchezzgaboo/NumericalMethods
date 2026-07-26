#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <implot.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>
#include <vector>

struct Point2D
{
    std::vector<float> x;
    std::vector<float> y;
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



int main(){
    //INIT conditions for IVP

    float x0 = 0;
    float y1_x0 = 1;
    float y2_x0 = -100;

    Point2D res = forward_euler(x0, y1_x0, y2_x0, 1);
    //std::cout << res.x.back() << std::endl;
    //std::cout << res.y.back() << std::endl;
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

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Forward Euler");

        if (ImPlot::BeginPlot("Solution", ImVec2(-1, 500)))
        {
            ImPlot::PlotLine(
                "Forward Euler",
                res.x.data(),
                res.y.data(),
                static_cast<int>(res.x.size())
            );

            ImPlot::EndPlot();
        }

        ImGui::End();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glViewport(0, 0, display_w, display_h);

        glClearColor(0.15f,0.15f,0.18f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    ImPlot::DestroyContext();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    glfwTerminate();
}

