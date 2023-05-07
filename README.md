# <b>JVM-Hook</b>

JVM-Hook is a helper library for creating native applications that interact with the JVM. It aims to ease up the process and make the process more user friendly.

---

## <b>Features</b>

- <b>Hooking</b>: You can easily hook JVM functions with the help of MinHook. It's also easy to edit if you want to implement your own hooking library.
- <b>Easy integration</b>: Provides a powerful wrapper for the JNI functionality.
- <b>Coverage</b>: Works both for internal as well as external projects.

---

## <b>Getting started</b>

### <b>Prerequisites</b>

- `Have Java Development Kit (JDK) installed.`
- `C++ compiler that supports C++11 or later.`

### <b>Installation</b>

To use JVM-Hook in your project, simply download `jvmhook.h` and `jvmhook.cpp` and include the header.

### <b>Usage</b>

To use JVM-Hook in your project, you first need to initialize the library by calling the JVMHook::Init() function. This function initializes the JVM and sets up the JNI environment. You can specify a handle to the JVM if you're not injecting your binary into the JVM.

```cpp
JVMHook MyJVM;
MyJVM.Init();
```

It's important that you call the `JVMHook::Exit()` before exiting your process or else the JVM might crash.

```cpp
MyJVM.Exit();
```

### Obtaining function pointers

To obtain a pointer to a Java function in memory you call the `JVMHook::GetFunctionPointer()` function. There are multiple function overrides to suite different scenarios but the one shown below takes the function name, class, JNI signature as well as whether the function is static or not.

```cpp
void* myFunctionPtr = MyJVM.GetFunctionPointer("TestClass", "TestFunction", "()V", true);
```

### Hooking functions

To hook Java functions with JVMHook you call the `JVMHook::HookFunction()` function. This is also the function you want to edit if you would like to change out the MinHook hooking method for your own solution.

```cpp
using f_myFunction = void(__stdcall*)(void**, void**);
inline f_myFunction testFunctionOriginal;
void __stdcall testFunctionHook(void** p1, void** p2)
{
    // Place le code in here

    return testFunctionOriginal(p1, p2);
}

// ...

void* myFunctionPtr = MyJVM.GetFunctionPointer("TestClass", "TestFunction", "()V", true);

if (MyJVM.HookFunction(myFunctionPtr, &testFunctionHook, &testFunctionOriginal))
    // Passed
else
    // Failed
```

---

## <b>Contribute</b>

JVM-Hook is an open-source library, and we welcome contributions from the community. If you'd like to contribute to JVM-Hook, here are some ways you can help:

- <b>Submit bug reports</b>: If you encounter any bugs or issues while using JVM-Hook, please let me know! You can submit bug reports and issues through the project's GitHub issue tracker.
- <b>Submit pull requests</b>: If you've made changes or improvements to JVM-Hook, we encourage you to submit a pull request. Please make sure that your changes are well-tested and that they adhere to the project's coding standards.

When contributing to JVM-Hook, please be respectful and professional in all communications, all contributions are appreciated!
