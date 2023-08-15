// dear imgui: standalone example application for Android + OpenGL ES 3
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.

#include "App.h"
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/asset_manager.h>
#include <string>

App* myApp = nullptr;

void init(android_app* app) {
    myApp = new App(app->window);
    myApp->initializeOpenGL(app);
}

void shutdown() {
    if (myApp != nullptr) {
        myApp->shutdown();
    }
}

static void handleAppCmd(struct android_app* app, int32_t appCmd) {
    switch (appCmd) {
        case APP_CMD_INIT_WINDOW:
            init(app);
            break;
        case APP_CMD_TERM_WINDOW:
            shutdown();
            break;
        case APP_CMD_SAVE_STATE:
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
        while (ALooper_pollAll(myApp != nullptr && myApp->getG_Initialized() ? 0 : -1, nullptr, &out_events, (void**)&out_data) >= 0) {
            // Process one event
            if (out_data != nullptr) {
                out_data->process(app, out_data);
            }
            // Exit the app by returning from within the infinite loop
            if (app->destroyRequested != 0) {
                // shutdown() should have been called already while processing the
                // app command APP_CMD_TERM_WINDOW. But we play save here
                if (myApp != nullptr || !myApp->getG_Initialized()) {
                    shutdown();
                }
                return;
            }
        }
        myApp->render();
        myApp->update();
    }
}


