#pragma once
#include "SceneControlMenu.h"
#include "BasicScene.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <string>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <android/log.h>
#include <glm/glm.hpp>
#include "Shader.h"

class App {
public:
    // Data
    EGLDisplay g_EglDisplay = EGL_NO_DISPLAY;
    EGLSurface g_EglSurface = EGL_NO_SURFACE;
    EGLContext g_EglContext = EGL_NO_CONTEXT;
    struct android_app *g_App = nullptr;
    bool g_Initialized = false;
    std::string g_LogTag = "ImGuiExample";
    std::string g_IniFilename = "";

    SceneControlMenu mSceneMenu_;
    BasicScene *testScene = nullptr;

    App(ANativeWindow *pWindow);

    void update();

    void render();

    void initializeOpenGL(struct android_app *pApp);

    bool getG_Initialized() const;

    void shutdown();

    android_app *getAndroidApp() const;

    int ShowSoftKeyboardInput() const;

    int pollUnicodeChars() const;

    BasicScene* getScene();

    int GetAssetData(const char *filename, void **outData) const;

};