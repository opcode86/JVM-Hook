#include "jvmhook.h"
#include <iostream>
#include <Psapi.h>

using _JNI_GetCreatedJavaVMs = jint(JNICALL*)(JavaVM**, jsize, jsize*);

bool JVMHook::Init(HMODULE hJvm)
{
	exited = false;
	MH_Initialize();

	// If no handle is provided we assume we're a DLL injected into JVM... so we assume jvm.dll is loaded.
	if (hJvm == nullptr)
		hJvm = GetModuleHandleA("jvm.dll");

	jint res;

	JavaVMInitArgs vm_args;
	vm_args.version = JNI_VERSION_1_8;
	vm_args.nOptions = 0;
	vm_args.ignoreUnrecognized = JNI_TRUE;

	jsize nVMs = { 0 };



	if (hJvm == nullptr)
		return false;
	

	_JNI_GetCreatedJavaVMs f_JNI_GetCreatedJavaVMs = (_JNI_GetCreatedJavaVMs)(GetProcAddress(hJvm, "JNI_GetCreatedJavaVMs"));

	if (f_JNI_GetCreatedJavaVMs == nullptr)
		return false;
	

	res = f_JNI_GetCreatedJavaVMs(&jvm, 1, &nVMs);
	if (res != JNI_OK)
		return false;
	

	res = jvm->AttachCurrentThread((void**)&env, NULL);
	if (res != JNI_OK)
		return false;

	res = jvm->GetEnv((void**)&jvmti, JVMTI_VERSION);
	if (res != JNI_OK) 
		return false;

	//jvmti->GetMethodLocation()
	jint classCount = 0;
	jclass* classes;
	jvmti->GetLoadedClasses(&classCount, &classes);
	if (classCount == 0)
		return false;

	return true;
}

void JVMHook::Exit() noexcept
{
	exited = true;

	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	MH_Uninitialize();

	// Unhook before detatching from thread to avoid crash
	jvm->DetachCurrentThread();
}

JVMHook::~JVMHook()
{
	if (!exited)
		Exit(); // Call exit if not called manually
}

void* JVMHook::GetFunctionPointer(const char* class_name, const char* func_name, const char* signature, bool is_static)
{
	auto mid = GetMethodID(class_name, func_name, signature, is_static);
	if (mid == NULL)
		return nullptr;

	return GetFunctionPointer(mid);
}

void* JVMHook::GetFunctionPointer(jclass cls, const char* func_name, const char* signature, bool is_static)
{
	auto mid = GetMethodID(cls, func_name, signature, is_static);
	if (mid == NULL)
		return nullptr;

	return GetFunctionPointer(mid);
}

void* JVMHook::GetFunctionPointer(jmethodID mid)
{
	if (mid == NULL) // In case a non-valid methodID is supplied
		return nullptr;

	return reinterpret_cast<void**>(*(uint64_t*)(*(uint64_t*)mid + 0x40));
}

jmethodID JVMHook::GetMethodID(jclass cls, const char* func_name, const char* signature, bool is_static)
{
	jmethodID method_ID;

	if (is_static) method_ID = env->GetStaticMethodID(cls, func_name, signature);
	else method_ID = env->GetMethodID(cls, func_name, signature);

	// We do not delete local reference to class inside of here, you have to do this yourself.
	return method_ID;
}

jmethodID JVMHook::GetMethodID(const char* class_name, const char* func_name, const char* signature, bool is_static)
{
	jclass temp_jclass = env->FindClass(class_name);
	if (temp_jclass == nullptr)
		return nullptr;
	

	jmethodID ret_val = GetMethodID(temp_jclass, func_name, signature, is_static);

	env->DeleteLocalRef(temp_jclass);

	return ret_val;
}

JavaVM* JVMHook::GetJVM()
{
	return this->jvm;
}

JNIEnv* JVMHook::GetEnv()
{
	return this->env;
}

jvmtiEnv* JVMHook::GetJVMTI()
{
	return this->jvmti;
}