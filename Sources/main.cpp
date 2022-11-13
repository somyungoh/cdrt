#include "GLFW/glfw3.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#include "renderer.h"

//----------------------------------------------------
// global variables

float*              pixmap = nullptr;
cd::CRenderer       renderer;
cd::SRenderSetting  renderSetting;

//----------------------------------------------------

void saveJpeg(u_int32_t img_w, u_int32_t img_h)
{
    const char* filename = "MyRender.jpg";

    // remap pixmap to 8 bits
    unsigned char pixmap8bits[img_w * img_h * 3];
    for (size_t i = 0; i < img_w * img_h * 3; ++i)
        pixmap8bits[i] = pixmap[i] * 255.f;

    stbi_flip_vertically_on_write(true);
    int success = stbi_write_jpg(filename, img_w, img_h, 3, pixmap8bits, 100);

    if (success)
        std::cout << "Image saved: " << filename << std::endl;
    else
        std::cout << "Error while saving image: " << filename << std::endl;
}

//----------------------------------------------------

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        saveJpeg(renderSetting.render_w, renderSetting.render_h);
}

//----------------------------------------------------

void init()
{
    // render setting
    renderSetting.render_w      = 800;
    renderSetting.render_h      = 600;
    renderSetting.nSamples      = 1;    // 4, 9 ,16
    renderSetting.nMaxDepth     = 10;

    renderSetting.K_DIG             = 0.001f;
    renderSetting.A_ZI              = 0.00001f;
    renderSetting.K_R0              = 1.f;
    renderSetting.K_RN              = 0.05f;
    renderSetting.K_TOTAL_DR_S      = 1.f;
    renderSetting.EXP_TOTAL_DR_S    = 1.f;

    renderer.SetRenderSetting(renderSetting);
    renderer.InitScene();
}

//----------------------------------------------------

int main(void)
{
    init();

    // GLFW init
    GLFWwindow* window;
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(800, 600, "Convolution Domain Raytracer (beta)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwSetKeyCallback(window, keyCallback);
    glfwMakeContextCurrent(window);

    // render!
    renderer.Render();

    // GLFW main loop
    while (!glfwWindowShouldClose(window))
    {
        int fbuffer_w, fbuffer_h;
        int window_w, window_h;
        glfwGetFramebufferSize(window, &fbuffer_w, &fbuffer_h);
        glfwGetWindowSize(window, &window_w, &window_h);

        glViewport(0, 0, fbuffer_w, fbuffer_h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_w, 0, window_h, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        renderer.GetLastRender(pixmap);

        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i(0, 0);
        glPixelZoom(fbuffer_w / renderSetting.render_w, fbuffer_h / renderSetting.render_h);    // because (window size != framebuffer) size can happen
        glDrawPixels(renderSetting.render_w, renderSetting.render_h, GL_RGB, GL_FLOAT, pixmap);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    
    delete[] pixmap;

    return 0;
}
