#define DllExport extern "C" __declspec(dllexport)

DllExport void RunStopHook(bool State, HINSTANCE hInstance);

DllExport LRESULT WINAPI MouseCBK(int nCode, WPARAM wParam, LPARAM lParam);

DllExport LRESULT WINAPI WinCreateCBK(int nCode, WPARAM wParam, LPARAM lParam);

DllExport void Cazzo(int i);
DllExport void  setHook(HINSTANCE hinstDLL);