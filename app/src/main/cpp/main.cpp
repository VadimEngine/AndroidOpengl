// dear imgui: standalone example application for Android + OpenGL ES 3
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>

// Data
static EGLDisplay           g_EglDisplay = EGL_NO_DISPLAY;
static EGLSurface           g_EglSurface = EGL_NO_SURFACE;
static EGLContext           g_EglContext = EGL_NO_CONTEXT;
static struct android_app*  g_App = nullptr;
static bool                 g_Initialized = false;
static char                 g_LogTag[] = "ImGuiExample";
static std::string          g_IniFilename = "";

// Forward declarations of helper functions
static int ShowSoftKeyboardInput();
static int PollUnicodeChars();
static int GetAssetData(const char* filename, void** out_data);

float triangleVertices[] = {
        0.0f, 0.5f,   // Vertex 1 (x, y)
        -0.5f, -0.5f, // Vertex 2 (x, y)
        0.5f, -0.5f   // Vertex 3 (x, y)
};

const char* vertexShaderSource = R"(
    #version 300 es
    layout (location=0) in vec2 position;

    void main() {
        gl_Position = vec4(position, 0.0f, 1.0f);
    }
)";

GLuint vertexShader, fragmentShader, shaderProgram;
GLuint vao;

// Fragment Shader
const char* fragmentShaderSource = R"(
    #version 300 es
    precision mediump float;

    out vec4 fragColor;

    void main() {
        fragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f); // White color
    }
)";

void init(struct android_app* app) {
    if (g_Initialized)
        return;

    g_App = app;
    ANativeWindow_acquire(g_App->window);

    // Initialize EGL
    // This is mostly boilerplate code for EGL...
    {
        g_EglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (g_EglDisplay == EGL_NO_DISPLAY)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglGetDisplay(EGL_DEFAULT_DISPLAY) returned EGL_NO_DISPLAY");

        if (eglInitialize(g_EglDisplay, nullptr, nullptr) != EGL_TRUE) {
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s",
                                "eglInitialize() returned with an error");
        }
        const EGLint egl_attributes[] = {
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_DEPTH_SIZE, 24,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE
        };
        EGLint num_configs = 0;
        if (eglChooseConfig(g_EglDisplay, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglChooseConfig() returned with an error");
        if (num_configs == 0)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglChooseConfig() returned 0 matching config");

        // Get the first matching config
        EGLConfig egl_config;
        eglChooseConfig(g_EglDisplay, egl_attributes, &egl_config, 1, &num_configs);
        EGLint egl_format;
        eglGetConfigAttrib(g_EglDisplay, egl_config, EGL_NATIVE_VISUAL_ID, &egl_format);
        ANativeWindow_setBuffersGeometry(g_App->window, 0, 0, egl_format);

        const EGLint egl_context_attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
        g_EglContext = eglCreateContext(g_EglDisplay, egl_config, EGL_NO_CONTEXT, egl_context_attributes);

        if (g_EglContext == EGL_NO_CONTEXT)
            __android_log_print(ANDROID_LOG_ERROR, g_LogTag, "%s", "eglCreateContext() returned EGL_NO_CONTEXT");

        g_EglSurface = eglCreateWindowSurface(g_EglDisplay, egl_config, g_App->window, nullptr);
        eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);

        // shader stuff?
        // Initialize GL state.
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        // Compile and link shaders
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        // Create and link the shader program
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Set up vertex data
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, triangleVertices);

        // Generate and bind a vertex array object (VAO)
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Generate and bind a vertex buffer object (VBO)
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        // Enable vertex array
        glEnableVertexAttribArray(0);

        // unbind?
        glBindVertexArray(0);
        //glUseProgram(0);
        // Clean up shaders and program (you should do this when you're done with them)
        //glDeleteShader(vertexShader);
        //glDeleteShader(fragmentShader);
        //glDeleteProgram(shaderProgram);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Redirect loading/saving of .ini file to our location.
    // Make sure 'g_IniFilename' persists while we use Dear ImGui.
    g_IniFilename = std::string(app->activity->internalDataPath) + "/imgui.ini";
    io.IniFilename = g_IniFilename.c_str();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplAndroid_Init(g_App->window);
    ImGui_ImplOpenGL3_Init("#version 300 es");

    // We load the default font with increased size to improve readability on many devices with "high" DPI.
    // FIXME: Put some effort into DPI awareness.
    // Important: when calling AddFontFromMemoryTTF(), ownership of font_data is transferred by Dear ImGui by default (deleted is handled by Dear ImGui), unless we set FontDataOwnedByAtlas=false in ImFontConfig
    ImFontConfig font_cfg;
    font_cfg.SizePixels = 22.0f;
    io.Fonts->AddFontDefault(&font_cfg);
    // Arbitrary scale-up
    // FIXME: Put some effort into DPI awareness
    ImGui::GetStyle().ScaleAllSizes(3.0f);

    g_Initialized = true;
}

void drawShader() {
    glUseProgram(shaderProgram);
    // bind vao
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Swap buffers
    glBindVertexArray(0); // unbind

    //eglSwapBuffers(g_EglDisplay, g_EglSurface);
}

void drawImgui() {
    ImGuiIO& io = ImGui::GetIO();
    if (g_EglDisplay == EGL_NO_DISPLAY)
        return;

    static ImVec4 clear_color = ImVec4(0,0,0, 1.00f);

    // Open on-screen (soft) input if requested by Dear ImGui
    static bool WantTextInputLast = false;
    if (io.WantTextInput && !WantTextInputLast)
        ShowSoftKeyboardInput();
    WantTextInputLast = io.WantTextInput;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void shutdown() {
    if (!g_Initialized)
        return;

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplAndroid_Shutdown();
    ImGui::DestroyContext();

    if (g_EglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(g_EglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (g_EglContext != EGL_NO_CONTEXT)
            eglDestroyContext(g_EglDisplay, g_EglContext);

        if (g_EglSurface != EGL_NO_SURFACE)
            eglDestroySurface(g_EglDisplay, g_EglSurface);

        eglTerminate(g_EglDisplay);
    }

    g_EglDisplay = EGL_NO_DISPLAY;
    g_EglContext = EGL_NO_CONTEXT;
    g_EglSurface = EGL_NO_SURFACE;
    ANativeWindow_release(g_App->window);

    g_Initialized = false;
}

static void handleAppCmd(struct android_app* app, int32_t appCmd) {
    switch (appCmd) {
    case APP_CMD_SAVE_STATE:
        break;
    case APP_CMD_INIT_WINDOW:
        init(app);
        break;
    case APP_CMD_TERM_WINDOW:
        shutdown();
        break;
    case APP_CMD_GAINED_FOCUS:
    case APP_CMD_LOST_FOCUS:
    default:
        break;
    }
}

static int32_t handleInputEvent(struct android_app* app, AInputEvent* inputEvent) {
    return ImGui_ImplAndroid_HandleInputEvent(inputEvent);
}

void android_main(struct android_app* app) {
    app->onAppCmd = handleAppCmd;
    app->onInputEvent = handleInputEvent;

    while (true) {
        int out_events;
        struct android_poll_source* out_data;

        // Poll all events. If the app is not visible, this loop blocks until g_Initialized == true.
        while (ALooper_pollAll(g_Initialized ? 0 : -1, nullptr, &out_events, (void**)&out_data) >= 0) {
            // Process one event
            if (out_data != nullptr) {
                out_data->process(app, out_data);
            }
            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0) {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (!g_Initialized) {
                    shutdown();
                }
                return;
            }
        }

        glClearColor(.5,.5,.5, 1);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        drawShader();
        drawImgui();
        // Swap buffers
        eglSwapBuffers(g_EglDisplay, g_EglSurface);
    }
}

// Unfortunately, there is no way to show the on-screen input from native code.
// Therefore, we call ShowSoftKeyboardInput() of the main activity implemented in MainActivity.kt via JNI.
static int ShowSoftKeyboardInput() {
    JavaVM* java_vm = g_App->activity->vm;
    JNIEnv* java_env = nullptr;

    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, nullptr);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    if (native_activity_clazz == nullptr)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "showSoftInput", "()V");
    if (method_id == nullptr)
        return -4;

    java_env->CallVoidMethod(g_App->activity->clazz, method_id);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}

// Unfortunately, the native KeyEvent implementation has no getUnicodeChar() function.
// Therefore, we implement the processing of KeyEvents in MainActivity.kt and poll
// the resulting Unicode characters here via JNI and send them to Dear ImGui.
static int PollUnicodeChars() {
    JavaVM* java_vm = g_App->activity->vm;
    JNIEnv* java_env = nullptr;

    jint jni_return = java_vm->GetEnv((void**)&java_env, JNI_VERSION_1_6);
    if (jni_return == JNI_ERR)
        return -1;

    jni_return = java_vm->AttachCurrentThread(&java_env, nullptr);
    if (jni_return != JNI_OK)
        return -2;

    jclass native_activity_clazz = java_env->GetObjectClass(g_App->activity->clazz);
    if (native_activity_clazz == nullptr)
        return -3;

    jmethodID method_id = java_env->GetMethodID(native_activity_clazz, "pollUnicodeChar", "()I");
    if (method_id == nullptr)
        return -4;

    // Send the actual characters to Dear ImGui
    ImGuiIO& io = ImGui::GetIO();
    jint unicode_character;
    while ((unicode_character = java_env->CallIntMethod(g_App->activity->clazz, method_id)) != 0)
        io.AddInputCharacter(unicode_character);

    jni_return = java_vm->DetachCurrentThread();
    if (jni_return != JNI_OK)
        return -5;

    return 0;
}

// Helper to retrieve data placed into the assets/ directory (android/app/src/main/assets)
static int GetAssetData(const char* filename, void** outData) {
    int num_bytes = 0;
    AAsset* asset_descriptor = AAssetManager_open(g_App->activity->assetManager, filename, AASSET_MODE_BUFFER);
    if (asset_descriptor) {
        num_bytes = AAsset_getLength(asset_descriptor);
        *outData = IM_ALLOC(num_bytes);
        int64_t num_bytes_read = AAsset_read(asset_descriptor, *outData, num_bytes);
        AAsset_close(asset_descriptor);
        IM_ASSERT(num_bytes_read == num_bytes);
    }
    return num_bytes;
}
