#include <jni.h>
#include <unigram/tokenizer.h>
#include <memory>
#include <string>

// Helper to convert jstring to std::string
std::string jstring_to_string(JNIEnv* env, jstring jstr) {
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}

// Helper to convert std::vector<std::string> to jobjectArray
jobjectArray vector_to_jarray(JNIEnv* env, const std::vector<std::string>& vec) {
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray result = env->NewObjectArray(vec.size(), stringClass, nullptr);
    
    for (size_t i = 0; i < vec.size(); ++i) {
        jstring str = env->NewStringUTF(vec[i].c_str());
        env->SetObjectArrayElement(result, i, str);
        env->DeleteLocalRef(str);
    }
    
    return result;
}

// Helper to convert jobjectArray to std::vector<std::string>
std::vector<std::string> jarray_to_vector(JNIEnv* env, jobjectArray jarray) {
    std::vector<std::string> result;
    jsize length = env->GetArrayLength(jarray);
    
    for (jsize i = 0; i < length; ++i) {
        jstring jstr = (jstring)env->GetObjectArrayElement(jarray, i);
        result.push_back(jstring_to_string(env, jstr));
        env->DeleteLocalRef(jstr);
    }
    
    return result;
}

extern "C" {

JNIEXPORT jlong JNICALL Java_com_unigram_Tokenizer_createNative(JNIEnv* env, jobject obj) {
    try {
        auto* tokenizer = new unigram::Tokenizer();
        return reinterpret_cast<jlong>(tokenizer);
    } catch (...) {
        return 0;
    }
}

JNIEXPORT void JNICALL Java_com_unigram_Tokenizer_destroyNative(JNIEnv* env, jobject obj, jlong handle) {
    if (handle != 0) {
        auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
        delete tokenizer;
    }
}

JNIEXPORT jboolean JNICALL Java_com_unigram_Tokenizer_load(JNIEnv* env, jobject obj, jstring filepath) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID fidHandle = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, fidHandle);
    
    auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
    if (!tokenizer) return JNI_FALSE;
    
    std::string path = jstring_to_string(env, filepath);
    return tokenizer->load(path) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_com_unigram_Tokenizer_save(JNIEnv* env, jobject obj, jstring filepath) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID fidHandle = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, fidHandle);
    
    auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
    if (!tokenizer) return JNI_FALSE;
    
    std::string path = jstring_to_string(env, filepath);
    return tokenizer->save(path) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jobjectArray JNICALL Java_com_unigram_Tokenizer_encode(JNIEnv* env, jobject obj, jstring text) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID fidHandle = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, fidHandle);
    
    auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
    if (!tokenizer) return nullptr;
    
    std::string input = jstring_to_string(env, text);
    auto tokens = tokenizer->encode(input);
    
    return vector_to_jarray(env, tokens);
}

JNIEXPORT jintArray JNICALL Java_com_unigram_Tokenizer_encodeAsIds(JNIEnv* env, jobject obj, jstring text) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID fidHandle = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, fidHandle);
    
    auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
    if (!tokenizer) return nullptr;
    
    std::string input = jstring_to_string(env, text);
    auto ids = tokenizer->encode_as_ids(input);
    
    jintArray result = env->NewIntArray(ids.size());
    env->SetIntArrayRegion(result, 0, ids.size(), reinterpret_cast<const jint*>(ids.data()));
    
    return result;
}

JNIEXPORT jstring JNICALL Java_com_unigram_Tokenizer_decode(JNIEnv* env, jobject obj, jobjectArray tokens) {
    jclass cls = env->GetObjectClass(obj);
    jfieldID fidHandle = env->GetFieldID(cls, "nativeHandle", "J");
    jlong handle = env->GetLongField(obj, fidHandle);
    
    auto* tokenizer = reinterpret_cast<unigram::Tokenizer*>(handle);
    if (!tokenizer) return nullptr;
    
    auto token_vec = jarray_to_vector(env, tokens);
    std::string result = tokenizer->decode(token_vec);
    
    return env->NewStringUTF(result.c_str());
}

} // extern "C"
