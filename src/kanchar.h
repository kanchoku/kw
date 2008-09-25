#define EXPORT extern "C" __declspec(dllexport)

EXPORT BOOL IsHooking(void);
EXPORT HHOOK MySetHook(void);
EXPORT int MyEndHook(void);
EXPORT LRESULT CALLBACK msgHookProc(int, WPARAM, LPARAM);

