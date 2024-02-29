#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <sstream> // Do formatowania stringa

#include "project.hpp"
void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}
int main(int argc, char** argv)
{
    int horizontal;
    int vertical;
    GetDesktopResolution(horizontal, vertical);
    // inicjalizacja glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // tworzenie okna za pomoca glfw
    GLFWwindow* window = glfwCreateWindow(horizontal, vertical, "FirstWindow", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ladowanie OpenGL za pomoca glew
    glewInit();
    glViewport(0, 0, horizontal+80, vertical+60);

    init(window);

    // Zapisz pocz¹tkowy czas


    // Modyfikacja g³ównej pêtli
    while (!glfwWindowShouldClose(window))
    {
        // Obliczanie up³ywaj¹cego czasu


        // Reszta twojej pêtli renderowania
        renderLoop(window);

        // Poll for and process events
        glfwPollEvents();
    }

    shutdown(window);
    glfwTerminate();
    return 0;
}