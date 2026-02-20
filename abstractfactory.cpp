#include <iostream>
#include <memory>

using namespace std;

class Window {
public:
    virtual ~Window() = default;
    virtual void draw()= 0;
};

class Button {
public:
    virtual ~Button() = default;
    virtual void draw()= 0;
};

class Shader {
public:
    virtual ~Shader() = default;
    virtual void compile()= 0;
};

class OpenGLWindow: public Window {
public:
    void draw() override {cout << "[OpenGL] Drawing Window"<<endl;}
};

class OpenGlButton: public Button {
public:
    void draw() override {cout << "[OpenGL] Drawing Button"<<endl;}
};

class OpenGLShader: public Shader {
public:
    void compile() override { cout << "[OpenGL] Compiling Shader"<<endl;}
};



class VulkanWindow: public Window {
public:
    void draw() override {cout << "[Vulkan] Drawing Window"<< endl;}
};
class VulkanButton: public Button {
public:
    void draw() override {cout << "[Vulkan] Drawing Button"<<endl;}
};
class VulkanShader: public Shader {
public:
    void compile() override { cout << "[Vulkan] Compiling Shader"<<endl;}
};

class RenderFactory {
public:
    virtual ~RenderFactory() = default;
    virtual Window* createWindow()= 0;
    virtual Button* createButton()= 0;
    virtual Shader* createShader()= 0;
};

class OpenGLFactory: public RenderFactory {
    public:
    Window* createWindow() override {return new OpenGLWindow();}
    Button* createButton() override {return new OpenGlButton();}
    Shader* createShader() override {return new OpenGLShader();}
};
class VulkanFactory: public RenderFactory {
    public:
    Window* createWindow() override {return new VulkanWindow();}
    Button* createButton() override {return new VulkanButton();}
    Shader* createShader() override {return new VulkanShader();}
};


int main() {

    unique_ptr<RenderFactory> factory = make_unique<OpenGLFactory>();
    auto window = factory->createWindow();
    auto button = factory->createButton();
    window->draw();
    button->draw();
    factory = make_unique<VulkanFactory>();
    auto shader = factory->createShader();
    shader->compile();

    return 0;
}