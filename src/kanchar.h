#define EXPORT extern "C" __declspec(dllexport)

EXPORT BOOL IsHooking(void);
EXPORT void MySetHook(HHOOK *, HHOOK *);
EXPORT int MyEndHook(void);
EXPORT LRESULT CALLBACK cwpHookProc(int, WPARAM, LPARAM);
EXPORT LRESULT CALLBACK msgHookProc(int, WPARAM, LPARAM);

