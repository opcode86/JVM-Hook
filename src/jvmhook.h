#pragma once
#include <windows.h>
#include <type_traits>

#include "jni.h"
#include "jvmti.h"

#include "MinHook.h"


class JVMHook {
public:
	bool Init(HMODULE hJvm = nullptr);
	void Exit() noexcept;
	~JVMHook();

	template <typename FuncSig>
	bool HookFunction(void* func, void* detour, FuncSig original)
	{
		// We assume the pointer supplied is a direct pointer to the function
		if (func == nullptr)
			return false;

		if (MH_CreateHook(reinterpret_cast<void**>(func), detour, reinterpret_cast<void**>(original)) != MH_OK)
			return false;
		if (MH_EnableHook(func) != MH_OK)
			return false;

		return true;
	}

	template <typename FuncSig>
	bool HookMethod(jmethodID func, void* detour, FuncSig original)
	{
		auto func_ptr = GetFunctionPointer(func);

		return HookFunction(func_ptr, detour, original);
	}


	/**
	 * Attempts to retrieve a pointer to the specified function within a class.
	 *
	 * @param class_name Name of the class where the function exists
	 * @param func_name Name of function
	 * @param signature JNI signature of function
	 * @param is_static Whether or not the function is static, default is false
	 *
	 * @param Pointer to function (methodID) if successful, nullptr if something failed
	 */
	void* GetFunctionPointer(const char* class_name, const char* func_name, const char* signature, bool is_static = false);
	void* GetFunctionPointer(jclass cls, const char* func_name, const char* signature, bool is_static = false);
	void* GetFunctionPointer(jmethodID mid);

	jmethodID GetMethodID(jclass cls, const char* func_name, const char* signature, bool is_static = false);
	jmethodID GetMethodID(const char* class_name, const char* func_name, const char* signature, bool is_static = false);


	template <typename Ret, typename ...Args>
	auto CallStaticFunction(jclass cls, jmethodID function, Args... args) -> Ret
	{
		if (function == NULL)
			return NULL;

		if constexpr (std::is_same_v<Ret, jboolean>)
			return env->CallStaticBooleanMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jbyte>)
			return env->CallStaticByteMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jchar>)
			return env->CallStaticCharMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jshort>)
			return env->CallStaticShortMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jint>)
			return env->CallStaticIntMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jlong>)
			return env->CallStaticLongMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jfloat>)
			return env->CallStaticFloatMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jdouble>)
			return env->CallStaticDoubleMethod(cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jobject>)
			return env->CallStaticObjectMethod(cls, function, args...);

		return NULL;
	}

	/**
	 * Calls a function of a class.
	 *
	 * @tparam Ret Type that function should return
	 * @tparam Args Variadic template parameter pack
	 * @param cls Class the function stems from
	 * @param function Function pointer (methodID) of function to be called
	 * @param is_static Whether or not function that is to be called is static or not, default is false
	 * @param args Args to be passed to the function
	 *
	 * @return Value that the function called returns, altertantivly NULL if it failed.
	 */
	template <typename Ret, typename ...Args>
	auto CallFunction(jobject obj, jmethodID function, Args... args) -> Ret
	{
		if (function == NULL)
			return NULL;

		if constexpr (std::is_same_v<Ret, jboolean>)
			return env->CallBooleanMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jbyte>)
			return env->CallByteMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jchar>)
			return env->CallCharMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jshort>)
			return env->CallShortMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jint>)
			return env->CallIntMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jlong>)
			return env->CallLongMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jfloat>)
			return env->CallFloatMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jdouble>)
			return env->CallDoubleMethod(obj, function, args...);

		else if constexpr (std::is_same_v<Ret, jobject>)
			return env->CallStaticObjectMethod(obj, function, args...);


		return NULL;
	};
	template <typename Ret, typename ...Args>
	auto CallFunction(jobject obj, jmethodID function, bool is_static, Args... args) -> Ret
	{
		jclass cls = env->GetObjectClass(obj);

		Ret ret_val;

		if (is_static)
			ret_val = CallStaticFunction<Ret>(cls, function, is_static, args...);
		else
			ret_val = CallFunction<Ret>(obj, function, is_static, args...);

		env->DeleteLocalRef(cls);

		return ret_val;
	}
	template <typename Ret, typename ...Args>
	auto CallFunction(jobject obj, const char* func_name, const char* signature, bool is_static, Args... args) -> Ret
	{
		jclass cls = env->GetObjectClass(obj);
		jmethodID func = env->GetMethodID(cls, func_name, signature);

		Ret ret_val;

		if (is_static)
			ret_val = CallStaticFunction<Ret>(cls, func, args...);
		else
			ret_val = CallFunction<Ret>(obj, func, args...);

		env->DeleteLocalRef(cls);

		return ret_val;
	}
	template <typename Ret, typename ...Args>
	auto CallFunction(const char* class_name, const char* func_name, const char* signature, Args... args) -> Ret
	{
		jclass temp_class = env->FindClass(class_name);
		jmethodID func = GetMethodID(temp_class, func_name, signature, true);
		Ret ret_val = CallStaticFunction<Ret>(temp_class, func, args...);

		env->DeleteLocalRef(temp_class);



		return ret_val;
	}

	template <typename ...Args>
	void CallVoidFunction(jclass cls, jmethodID function, bool is_static, Args... args)
	{
		if (is_static)
			env->CallStaticVoidMethod(cls, function, args...);
		else
			env->CallVoidMethod(cls, function, args...);
	}
	template <typename ...Args>
	void CallVoidFunction(jobject obj, const char* func_name, const char* signature, bool is_static, Args... args)
	{

		jclass temp_class = env->GetObjectClass(obj);

		jmethodID func = GetMethodID(temp_class, func_name, signature, is_static);

		if (is_static)
			env->CallStaticVoidMethod(temp_class, func, args...);
		else
			env->CallVoidMethod(obj, func, args...);

		env->DeleteLocalRef(temp_class);
	}
	template <typename Ret, typename ...Args>
	auto CallVoidFunction(const char* class_name, const char* func_name, const char* signature, bool is_static, Args... args) -> Ret
	{

		jclass temp_class = env->FindClass(class_name);
		jmethodID func = GetMethodID(temp_class, func_name, signature, is_static);
		Ret ret_val = CallVoidFunction<Ret>(temp_class, func, is_static, args...);

		env->DeleteLocalRef(temp_class);

		return ret_val;
	}

	/**
	 * Calls a non-virtual function of an existing object.
	 *
	 * @tparam Ret Type that function should return
	 * @tparam Args Variadic template parameter pack
	 * @param obj Object to call the function from
	 * @param function Function pointer (methodID) of function to be called
	 * @param args Args to be passed to the function
	 *
	 * @return Value that the function called returns, altertantivly NULL if it failed.
	 */
	template <typename Ret, typename ...Args>
	auto CallNonVirtualFunction(jobject obj, jclass cls, jmethodID function, Args... args) -> Ret
	{
		if (function == NULL)
			return NULL;

		if constexpr (std::is_same_v<Ret, jboolean>)
			return env->CallNonvirtualBooleanMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jbyte>)
			return env->CallNonvirtualByteMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jchar>)
			return env->CallNonvirtualCharMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jshort>)
			return env->CallNonvirtualShortMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jint>)
			return env->CallNonvirtualIntMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jlong>)
			return env->CallNonvirtualLongMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jfloat>)
			return env->CallNonvirtualFloatMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jdouble>)
			return env->CallNonvirtualDoubleMethod(obj, cls, function, args...);

		else if constexpr (std::is_same_v<Ret, jobject>)
			return env->CallNonvirtualObjectMethod(obj, cls, function, args...);

		return NULL;
	};
	template <typename Ret, typename ...Args>
	auto CallNonVirtualFunction(jobject obj, const char* func_name, const char* signature, Args... args) -> Ret
	{
		jclass cls = env->GetObjectClass(obj);
		jmethodID func = env->GetMethodID(cls, func_name, signature);
		Ret retVal = CallNonVirtualFunction<Ret>(obj, cls, func, args...);

		env->DeleteLocalRef(cls);

		return retVal;
	}


	template <typename ...Args>
	void CallNonVirtualVoidFunction(jobject obj, jclass cls, jmethodID function, Args... args)
	{
		env->CallNonvirtualVoidMethod(obj, cls, function, args...);
	}
	template <typename ...Args>
	void CallNonVirtualVoidFunction(jobject obj, const char* func_name, const char* signature, Args... args)
	{
		jclass cls = env->GetObjectClass(obj);
		jmethodID func = env->GetMethodID(cls, func_name, signature);

		CallNonVirtualVoidFunction(obj, cls, func, args...);

		env->DeleteLocalRef(cls);
	}


	template <typename Ret>
	auto GetStaticField(jobject obj, jfieldID field) -> Ret
	{
		if (field == NULL)
			return NULL;

		if constexpr (std::is_same_v<Ret, jboolean>)
			return env->GetStaticBooleanField(obj, field);

		else if constexpr (std::is_same_v<Ret, jbyte>)
			return env->GetStaticByteField(obj, field);

		else if constexpr (std::is_same_v<Ret, jchar>)
			return env->GetStaticCharField(obj, field);

		else if constexpr (std::is_same_v<Ret, jshort>)
			return env->GetStaticShortField(obj, field);

		else if constexpr (std::is_same_v<Ret, jint>)
			return env->GetStaticIntField(obj, field);

		else if constexpr (std::is_same_v<Ret, jlong>)
			return env->GetStaticLongField(obj, field);

		else if constexpr (std::is_same_v<Ret, jfloat>)
			return env->GetStaticFloatField(obj, field);

		else if constexpr (std::is_same_v<Ret, jdouble>)
			return env->GetStaticDoubleField(obj, field);

		else if constexpr (std::is_same_v<Ret, jobject>)
			return env->GetStaticObjectField(obj, field);

		return NULL;
	}


	/**
	 * Attepts to retrieve the data stored in the specified field.
	 *
	 * @tparam Ret Type that the field is
	 * @param obj Object that the field is in
	 * @param field Field to retrieve data from
	 * @param is_static Whether or not the field is static
	 *
	 * @return Value of the field
	 */
	template <typename Ret>
	auto GetField(jobject obj, jfieldID field, bool is_static = false) -> Ret
	{
		if (field == NULL)
			return NULL;

		if (is_static)
			return GetStaticField(obj, field);

		if constexpr (std::is_same_v<Ret, jboolean>)
			return env->GetBooleanField(obj, field);

		else if constexpr (std::is_same_v<Ret, jbyte>)
			return env->GetByteField(obj, field);

		else if constexpr (std::is_same_v<Ret, jchar>)
			return env->GetCharField(obj, field);

		else if constexpr (std::is_same_v<Ret, jshort>)
			return env->GetShortField(obj, field);

		else if constexpr (std::is_same_v<Ret, jint>)
			return env->GetIntField(obj, field);

		else if constexpr (std::is_same_v<Ret, jlong>)
			return env->GetLongField(obj, field);

		else if constexpr (std::is_same_v<Ret, jfloat>)
			return env->GetFloatField(obj, field);

		else if constexpr (std::is_same_v<Ret, jdouble>)
			return env->GetDoubleField(obj, field);

		else if constexpr (std::is_same_v<Ret, jobject>)
			return env->GetObjectField(obj, field);

		return NULL;
	}
	template <typename Ret>
	auto GetField(jobject obj, const char* field_name, const char* signature, bool is_static = false)
	{
		jclass cls = env->GetObjectClass(obj);
		jfieldID fid = env->GetFieldID(cls, field_name, signature);

		env->DeleteLocalRef(cls);

		return GetField<Ret>(obj, fid, is_static);
	}

	JavaVM* GetJVM();

	JNIEnv* GetEnv();

	jvmtiEnv* GetJVMTI();

private:
	JavaVM* jvm;
	JNIEnv* env;
	jvmtiEnv* jvmti;
	bool exited;

	//JVMHookStatus PlaceHook(void* func, void* detour);
};
