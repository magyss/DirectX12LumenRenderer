#include "Core/DX12App.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    DX12App app(hInstance);

    if (!app.Initialize()) {
        return -1;
    }

    return app.Run();
}