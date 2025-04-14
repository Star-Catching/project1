#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <opencv2/opencv.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

// 控制状态用的全局变量（方便切换控件状态）
bool show_demo_window = false;
bool show_another_window = false;
float clear_color[4] = {0.2f, 0.3f, 0.3f, 1.0f};
float slider_value = 0.5f;
int counter = 0;
char text_input[128] = "你好，ImGui！";
GLuint camera_texture = 0;
cv::VideoCapture cap;

/*
函数负责初始化 OpenGL 纹理对象，用来储存摄像头捕获的图像数据。
它仅仅是为纹理设置参数，并为后续的图像数据传输做准备。
*/
// 初始化摄像头纹理
void InitCameraTexture() {
    // 生成一个纹理 ID 并存储到 camera_texture 变量
    glGenTextures(1, &camera_texture);
    
    // 绑定该纹理，使接下来的 OpenGL 操作作用于此纹理
    glBindTexture(GL_TEXTURE_2D, camera_texture);

    // 设置纹理的缩小过滤方式为线性过滤（平滑效果）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    // 设置纹理的放大过滤方式为线性过滤（平滑效果）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // 解绑当前纹理对象，结束对其的操作
    glBindTexture(GL_TEXTURE_2D, 0);
}
/*
    UpdateCameraTexture(cv::Mat& frame)：
    将摄像头的图像帧更新为 OpenGL 纹理，以便在渲染过程中显示它。
    它的主要目的是处理图像数据，并将其上传到 GPU 以供渲染使用，通常用于显示视频流或实时摄像头画面。
*/
void UpdateCameraTexture(cv::Mat& frame) {
    // 检查摄像头帧是否为空
    if (frame.empty()) return;

    // 将图像颜色从 BGR 转换为 RGB（因为 OpenGL 通常期望 RGB 格式）
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);  

    // 绑定纹理以准备更新数据
    glBindTexture(GL_TEXTURE_2D, camera_texture);  // 将 OpenGL 纹理对象 camera_texture 绑定到当前的 2D 纹理单元 (GL_TEXTURE_2D) 上。

    // 将 OpenCV 图像数据上传到 OpenGL 纹理对象
    // 参数含义：
    // GL_TEXTURE_2D: 目标纹理类型（2D 纹理）
    // 0: 纹理的 mipmap 层级，0 是基础层
    // GL_RGB: 纹理的颜色格式
    // frame.cols: 纹理宽度  即图像的尺寸。
    // frame.rows: 纹理高度  即图像的尺寸。
    // 0: 边界宽度（没有边界）
    // GL_RGB: 数据的颜色格式
    // GL_UNSIGNED_BYTE: 每个颜色分量的类型（无符号字节）
    // frame.data: 实际的图像数据（OpenCV 提供的图像数据）
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame.cols, frame.rows, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, frame.data);

    // 解绑纹理，操作完成
    glBindTexture(GL_TEXTURE_2D, 0);
}


int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW 初始化失败" << std::endl;
        return -1;
    }
    
    // OpenGL 版本设置为 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(2000, 960, "ImGui 多控件示例", nullptr, nullptr);
    if (!window) {
        std::cerr << "无法创建窗口" << std::endl;
        glfwTerminate(); // 关闭 GLFW 并释放它占用的所有资源。          
        return -1;
    }
    
    // 创建窗口
    glfwMakeContextCurrent(window);// 意思是：让 OpenGL 的“绘图上下文”绑定到这个窗口上，也就是告诉 OpenGL：“我接下来要在这个 window 里面画图！”
    glfwSwapInterval(1);  // 启用垂直同步 glfwSwapInterval(1);  // 启用垂直同步
    InitCameraTexture();
    // 初始化 ImGui
    IMGUI_CHECKVERSION();  //! wxz标记的框架 检查你当前使用的 ImGui 版本是否与编译时的版本匹配。
    ImGui::CreateContext();  //! wxz标记的框架 // 创建一个新的 ImGui 上下文。
    ImGuiIO& io = ImGui::GetIO(); (void)io;   // 获取 ImGui 的 IO（输入输出） 配置和状态对象。 (void)io：并避免编译警告
    
    // 加载支持中文的字体
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "/home/wxz/fonts/SourceHanSerifSC-VF.ttf", 18.0f, NULL,
        io.Fonts->GetGlyphRangesChineseFull()
    );
    if (!font) {
        std::cerr << "字体加载失败！请检查路径和字体文件是否存在。" << std::endl;
    }

    // 启用 Docking 和多视口 这段多个窗口显示
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();   //! wxz标记的框架 // ImGui 的主题设置为“暗色模式”


    /*
    这个函数用于初始化 ImGui 与 GLFW 之间的接口。具体来说，它会：
    初始化 ImGui 与 GLFW 之间的交互，让 ImGui 可以获取来自 GLFW 窗口 的输入事件（如键盘、鼠标、窗口大小等）。
    window 参数：你传入的 window 是你之前创建的 GLFW 窗口，它会告诉 ImGui 使用这个窗口的 OpenGL 上下文和输入事件。
    第二个参数 true：这个参数告诉 ImGui 是否启用 安装键盘和鼠标事件处理函数。通常我们会传入 true，因为我们希望 ImGui 能够自动处理所有输入事件（例如，鼠标点击、键盘按键等）。
    */
    ImGui_ImplGlfw_InitForOpenGL(window, true); //! wxz标记的框架
    /*
    这个函数用于初始化 ImGui 与 OpenGL 之间的接口，它告诉 ImGui 如何使用 OpenGL 来进行图形渲染。
    "#version 330"：这个字符串参数指定了 OpenGL 着色器的版本，通常是 OpenGL 3.x 或更高版本。
    在这里，#version 330 表示你将使用 OpenGL 3.3 版本的着色器。这对于正确运行 ImGui 在 OpenGL 上下文中的渲染很重要。
    */
    ImGui_ImplOpenGL3_Init("#version 330"); //! wxz标记的框架
    

    // 初始化摄像头
    cap.open(0);  // 默认摄像头
    if (!cap.isOpened()) {
        std::cerr << "无法打开摄像头！" << std::endl;
        return -1;
    }
    // 主循环
    while (!glfwWindowShouldClose(window)) {     // 启动 主渲染循环 的，具体来说，它的作用是不断检查窗口是否被关闭，并在窗口未关闭时保持程序的渲染和更新。
        
        glfwPollEvents();   //! wxz标记的框架
                            //glfwPollEvents() 会 检查并处理所有待处理的事件，例如用户的键盘输入、鼠标移动、窗口大小变化等。
                            //这个函数是 非阻塞的，即调用它后，它会立即返回，而不会停下来等待事件。如果没有事件要处理，它会直接返回，不会让程序进入阻塞状态。

        // 获取摄像头图像
        cv::Mat frame;
        cap >> frame;
        UpdateCameraTexture(frame);

        


        // 开始 ImGui 新帧
        //! wxz标记的框架
        ImGui_ImplOpenGL3_NewFrame();  // 作用：此函数初始化 OpenGL3 渲染器，用于处理 ImGui 的渲染部分。
                                       // 功能：为下一帧的渲染做好准备。它在每一帧的开始时被调用，确保 OpenGL 环境已经为 ImGui 渲染设置好了合适的状态。
        //! wxz标记的框架
        ImGui_ImplGlfw_NewFrame();     //作用：此函数初始化 GLFW 的输入状态，用于处理与窗口和用户输入相关的交互。
                                        //功能：在每一帧渲染之前调用，确保 GLFW 相关的输入事件（如键盘输入、鼠标移动、点击等）已经被正确处理，并且传递给 ImGui。
                                        //GLFW 是用来创建窗口和处理用户输入（如鼠标、键盘）的库。在每一帧渲染时，这个函数会更新所有与窗口和输入相关的状态，并将它们传递给 ImGui，以便后续界面可以响应用户的输入操作。
        //! wxz标记的框架
        ImGui::NewFrame();              // 作用：这是 ImGui 的函数，表示开始新的一帧渲染，并准备好下一帧的用户界面。
                                        // 功能：该函数会初始化 ImGui 渲染的所有状态，并为下一帧的 UI 布局和绘制做好准备。
                                        // 调用该函数后，后续的所有 UI 绘制（如创建窗口、按钮、文本等）都会影响到下一帧的界面。它在每一帧开始时调用，确保 ImGui 知道应该渲染新的一帧界面。




      



        // 顶部菜单栏
        if (ImGui::BeginMainMenuBar()) {           //ImGui 中用来创建主菜单栏的函数。它会开始一个主菜单栏的定义，并允许你在该菜单栏中添加菜单项。
            if (ImGui::BeginMenu("文件")) {
                if (ImGui::MenuItem("退出")) {     //请求关闭指定的窗口。
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("窗口")) {
                
                // ImGui::MenuItem 这里创建了一个菜单项 "显示 Demo 窗口"，当用户点击该菜单项时，它会根据 show_demo_window 变量的值来控制是否显示 Demo 窗口。
                // 第一个参数 ("显示 Demo 窗口")：这是菜单项的标签，用户在菜单中看到的文本。
                // 第二个参数 (NULL)：这是快捷键（可以用来绑定快捷键）。NULL 表示没有快捷键。你可以指定类似 "Ctrl+D" 这样的快捷键来触发菜单项。
                // 第三个参数 (&show_demo_window)：这是一个指向布尔值的指针，当用户点击菜单项时，show_demo_window 的值会被改变（从 false 切换为 true 或者反之）。这意味着，通过点击这个菜单项，show_demo_window 会控制是否显示 Demo 窗口。
                ImGui::MenuItem("显示 Demo 窗口", NULL, &show_demo_window);
                ImGui::MenuItem("显示附加窗口", NULL, &show_another_window);
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();   // 结束主菜单栏（Main Menu Bar）的创建。
        }

        // 主控面板
        {
           
            ImGui::Begin("主控窗口");
            // --- 上方控件 ---
            ImGui::Text("欢迎使用 ImGui 示例页面！");
            ImGui::InputText("文本输入", text_input, IM_ARRAYSIZE(text_input));
            ImGui::SliderFloat("滑动条", &slider_value, 0.0f, 1.0f);
            ImGui::ColorEdit3("背景颜色", clear_color);

            if (ImGui::Button("计数器++")) {
                counter++;
            }
            ImGui::SameLine(); //让下一个控件出现在同一行
            ImGui::Text("次数: %d", counter);

            ImGui::Separator();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            // --- 下方图像 ---
            if (!frame.empty()) {
                ImVec2 img_size(1000, 640);  // 显示大小
                ImGui::Image((ImTextureID)(uintptr_t)camera_texture, img_size);  // 在 ImGui 窗口中显示一张图像（纹理），也就是把 OpenGL 的纹理 camera_texture 渲染到 ImGui 的界面里
            } else {
                ImGui::Text("未捕获到图像");
            }

            ImGui::End();

        }

        // 附加窗口
        // if (show_another_window) {
        //     ImGui::Begin("附加窗口", &show_another_window);
        //     ImGui::Text("这个窗口可以被关闭和移动。");
        //     if (ImGui::Button("关闭")) {
        //         show_another_window = false;
        //     }
        //     ImGui::End();
        // }
       
        // 选项卡窗口
        // ImGui::Begin("选项卡示例");
        // if (ImGui::BeginTabBar("Tabs")) {
        //     if (ImGui::BeginTabItem("状态")) {
        //         ImGui::Text("当前值: %.2f", slider_value);
        //         ImGui::EndTabItem();
        //     }
        //     if (ImGui::BeginTabItem("设置")) {
        //         ImGui::Checkbox("显示 Demo 窗口", &show_demo_window);
        //         ImGui::Checkbox("显示 附加窗口", &show_another_window);
        //         ImGui::EndTabItem();
        //     }
        //     ImGui::EndTabBar();
        // }
        // ImGui::End();

        // 官方示例窗口
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

         // 渲染 ImGui：ImGui::Render() 函数会生成当前 ImGui UI 所需的渲染指令。
        ImGui::Render();   //! wxz标记的框架
  
        // 获取当前窗口的帧缓冲区尺寸，并将其存储到 display_w 和 display_h 变量中
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // 设置 OpenGL 的视口大小，视口大小通常与窗口大小相同
        glViewport(0, 0, display_w, display_h);

        // 设置 OpenGL 背景颜色（这里是一个暗色背景）
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);  //! wxz标记的框架


        // 清空屏幕的颜色缓冲区（实际上是用上面设置的背景色填充屏幕）
        glClear(GL_COLOR_BUFFER_BIT);   //! wxz标记的框架

        // 使用 OpenGL 渲染 ImGui 的绘制数据，生成最终的 UI 界面
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  //! wxz标记的框架

        // 检查是否启用了 ImGui 的多视口支持
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            // 保存当前 OpenGL 上下文（防止在更新平台窗口时丢失上下文）
            GLFWwindow* backup = glfwGetCurrentContext();
            
            // 更新平台窗口，主要是对多视口支持的处理
            ImGui::UpdatePlatformWindows();

            // 渲染所有平台窗口（包括 ImGui 本身的窗口和额外的视口窗口）
            ImGui::RenderPlatformWindowsDefault();
            
            // 恢复原先的 OpenGL 上下文
            glfwMakeContextCurrent(backup);
        }

        glfwSwapBuffers(window);  //! wxz标记的框架  //交换窗口的前后缓冲区，并将渲染结果显示在窗口上  
    }

    // 清理
    ImGui_ImplOpenGL3_Shutdown();  //! wxz标记的框架
    ImGui_ImplGlfw_Shutdown();     //! wxz标记的框架
    ImGui::DestroyContext();       //! wxz标记的框架

    glfwDestroyWindow(window);     //! wxz标记的框架
    glfwTerminate();               //! wxz标记的框架
    return 0;
}