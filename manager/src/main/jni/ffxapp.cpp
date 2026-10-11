#include <jni.h>
#include <string>
#include <vector>
#include <cstdio>
#include <android/log.h>

#define LOG_TAG "FFX_SECURITY"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// =========================================================================
// ANTI-HEX OBFUSCATOR (String dirakit di RAM, tidak ada teks di dalam .so)
// =========================================================================

std::string decryptStr(const int* secret, int size) {
    std::string res;
    res.reserve(size);
    for (int i = 0; i < size; i++) {
        res += (char)(secret[i] - 10);
    }
    return res;
}

std::string getReleaseSha() {
    const int data[] = {65, 78, 58, 60, 66, 58, 64, 66, 64, 63, 77, 61, 79, 63, 62, 63, 79, 64, 66, 66, 67, 78, 75, 64, 67, 80, 61, 58, 80, 64, 79, 75, 64, 60, 62, 63, 67, 58, 62, 77, 63, 78, 76, 77, 58, 76, 66, 66, 59, 79, 58, 75, 58, 77, 66, 66, 59, 66, 61, 67, 80, 75, 64, 65};
    return decryptStr(data, 64);
}

std::string getPlaystoreSha() {
    const int data[] = {63, 59, 64, 61, 60, 80, 75, 64, 62, 64, 64, 66, 66, 58, 65, 67, 77, 64, 60, 63, 65, 75, 67, 67, 63, 58, 60, 80, 60, 75, 66, 65, 67, 63, 67, 66, 66, 78, 60, 61, 77, 61, 59, 64, 62, 61, 75, 63, 75, 66, 78, 77, 64, 66, 79, 59, 76, 67, 60, 64, 75, 58, 80, 80};
    return decryptStr(data, 64);
}

std::string getAdmobInterstitial() {
    const int data[] = {109, 107, 55, 107, 122, 122, 55, 122, 127, 108, 55, 60, 58, 59, 61, 65, 60, 66, 60, 63, 63, 63, 61, 61, 60, 58, 63, 57, 59, 63, 58, 63, 65, 60, 60, 60, 65, 67};
    return decryptStr(data, 38);
}

std::string getAdmobRewarded() {
    const int data[] = {109, 107, 55, 107, 122, 122, 55, 122, 127, 108, 55, 60, 58, 59, 61, 65, 60, 66, 60, 63, 63, 63, 61, 61, 60, 58, 63, 57, 67, 61, 61, 63, 62, 58, 65, 66, 58, 65};
    return decryptStr(data, 38);
}

void printSecurityWarning() {
    const int data[] = {93, 115, 113, 120, 107, 126, 127, 124, 111, 42, 119, 115, 125, 119, 107, 126, 109, 114, 43};
    LOGE("%s", decryptStr(data, 19).c_str());
}

// =========================================================================
// NATIVE METHOD: checkSignature
// =========================================================================
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cyrust_fftoolspromaxnew_MainActivity_checkSignature(JNIEnv *env, jobject thiz, jobject context) {
    if (context == nullptr) return JNI_FALSE;

    jclass contextClass = env->GetObjectClass(context);
    if (contextClass == nullptr) return JNI_FALSE;

    jmethodID getPackageManagerId = env->GetMethodID(contextClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    if (getPackageManagerId == nullptr) return JNI_FALSE;
    jobject packageManager = env->CallObjectMethod(context, getPackageManagerId);
    if (packageManager == nullptr) return JNI_FALSE;

    jmethodID getPackageNameId = env->GetMethodID(contextClass, "getPackageName", "()Ljava/lang/String;");
    if (getPackageNameId == nullptr) return JNI_FALSE;
    jstring packageName = (jstring) env->CallObjectMethod(context, getPackageNameId);
    if (packageName == nullptr) return JNI_FALSE;

    jclass packageManagerClass = env->GetObjectClass(packageManager);
    if (packageManagerClass == nullptr) return JNI_FALSE;

    jmethodID getPackageInfoId = env->GetMethodID(packageManagerClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
    if (getPackageInfoId == nullptr) return JNI_FALSE;

    // Flag: GET_SIGNING_CERTIFICATES (134217728) | GET_SIGNATURES (64) = 134217792
    jobject packageInfo = env->CallObjectMethod(packageManager, getPackageInfoId, packageName, 134217792);
    if (env->ExceptionCheck() || packageInfo == nullptr) {
        env->ExceptionClear();
        // Fallback jika perangkat lawas menolak flag kombinasi
        packageInfo = env->CallObjectMethod(packageManager, getPackageInfoId, packageName, 64);
        if (env->ExceptionCheck() || packageInfo == nullptr) {
            env->ExceptionClear();
            return JNI_FALSE;
        }
    }

    jclass packageInfoClass = env->GetObjectClass(packageInfo);
    jobjectArray signatures = nullptr;

    // 1. Coba baca dari signingInfo (Android 9 / API 28 ke atas)
    jfieldID signingInfoFieldId = env->GetFieldID(packageInfoClass, "signingInfo", "Landroid/content/pm/SigningInfo;");
    if (signingInfoFieldId != nullptr) {
        jobject signingInfo = env->GetObjectField(packageInfo, signingInfoFieldId);
        if (signingInfo != nullptr) {
            jclass signingInfoClass = env->GetObjectClass(signingInfo);
            jmethodID hasMultipleSignersId = env->GetMethodID(signingInfoClass, "hasMultipleSigners", "()Z");
            jboolean hasMultiple = env->CallBooleanMethod(signingInfo, hasMultipleSignersId);

            if (hasMultiple) {
                jmethodID getSignersId = env->GetMethodID(signingInfoClass, "getApkContentsSigners", "()[Landroid/content/pm/Signature;");
                signatures = (jobjectArray) env->CallObjectMethod(signingInfo, getSignersId);
            } else {
                jmethodID getHistoryId = env->GetMethodID(signingInfoClass, "getSigningCertificateHistory", "()[Landroid/content/pm/Signature;");
                signatures = (jobjectArray) env->CallObjectMethod(signingInfo, getHistoryId);
            }
        }
    }

    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    // 2. Fallback ke signatures lama (Android 8 ke bawah atau jika signingInfo kosong)
    if (signatures == nullptr) {
        jfieldID signaturesFieldId = env->GetFieldID(packageInfoClass, "signatures", "[Landroid/content/pm/Signature;");
        if (signaturesFieldId != nullptr) {
            signatures = (jobjectArray) env->GetObjectField(packageInfo, signaturesFieldId);
        }
    }

    if (env->ExceptionCheck() || signatures == nullptr || env->GetArrayLength(signatures) == 0) {
        env->ExceptionClear();
        return JNI_FALSE;
    }

    jobject signature = env->GetObjectArrayElement(signatures, 0);
    if (signature == nullptr) return JNI_FALSE;

    jclass signatureClass = env->GetObjectClass(signature);
    jmethodID toByteArrayId = env->GetMethodID(signatureClass, "toByteArray", "()[B");
    if (toByteArrayId == nullptr) return JNI_FALSE;
    jbyteArray signatureBytes = (jbyteArray) env->CallObjectMethod(signature, toByteArrayId);
    if (signatureBytes == nullptr) return JNI_FALSE;

    jclass messageDigestClass = env->FindClass("java/security/MessageDigest");
    if (messageDigestClass == nullptr) {
        env->ExceptionClear();
        return JNI_FALSE;
    }

    jmethodID getInstanceId = env->GetStaticMethodID(messageDigestClass, "getInstance", "(Ljava/lang/String;)Ljava/security/MessageDigest;");
    if (getInstanceId == nullptr) {
        env->ExceptionClear();
        return JNI_FALSE;
    }

    jstring sha256String = env->NewStringUTF("SHA-256");
    jobject messageDigest = env->CallStaticObjectMethod(messageDigestClass, getInstanceId, sha256String);
    if (messageDigest == nullptr) return JNI_FALSE;

    jmethodID digestId = env->GetMethodID(messageDigestClass, "digest", "([B)[B");
    if (digestId == nullptr) return JNI_FALSE;

    jbyteArray digestBytes = (jbyteArray) env->CallObjectMethod(messageDigest, digestId, signatureBytes);
    if (digestBytes == nullptr) return JNI_FALSE;

    jbyte *digestElements = env->GetByteArrayElements(digestBytes, nullptr);
    if (digestElements == nullptr) return JNI_FALSE;

    jsize digestLength = env->GetArrayLength(digestBytes);
    if (digestLength != 32) {
        env->ReleaseByteArrayElements(digestBytes, digestElements, JNI_ABORT);
        return JNI_FALSE;
    }

    char hexString[65];
    for (int i = 0; i < digestLength; i++) {
        sprintf(&hexString[i * 2], "%02X", (unsigned char) digestElements[i]);
    }
    hexString[64] = '\0';

    env->ReleaseByteArrayElements(digestBytes, digestElements, JNI_ABORT);
    std::string currentAppHash(hexString);

    if (currentAppHash == getReleaseSha() || currentAppHash == getPlaystoreSha()) {
        return JNI_TRUE;
    }

    printSecurityWarning();
    return JNI_FALSE;
}

// =========================================================================
// NATIVE METHOD: getSecureAdId
// =========================================================================
extern "C"
JNIEXPORT jstring JNICALL
Java_com_cyrust_fftoolspromaxnew_AdmobManager_getSecureAdId(JNIEnv *env, jobject thiz, jobject context, jint type) {
    std::string adId = "";
    
    if (type == 1) {
        adId = getAdmobInterstitial();
    } else if (type == 2) {
        adId = getAdmobRewarded();
    } else {
        adId = getAdmobInterstitial(); 
    }

    return env->NewStringUTF(adId.c_str());
}
