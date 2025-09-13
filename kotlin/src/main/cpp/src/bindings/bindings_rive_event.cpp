//
// Created by Peter G Hayes on 23/08/2023.
//

#include "jni_refs.hpp"
#include "helpers/general.hpp"
#include "helpers/jni_exception_handler.hpp"
#include "rive/event.hpp"
#include "rive/open_url_event.hpp"
#include "rive/custom_property_boolean.hpp"
#include "rive/custom_property_string.hpp"
#include "rive/custom_property_number.hpp"
#include <jni.h>
#include <cassert>

// Assert in debug/dev builds, silent in release.
#ifndef RIVE_EXODUS_HARDEN_ASSERT
#  ifndef NDEBUG
#    define RIVE_EXODUS_HARDEN_ASSERT(MSG) assert(!"Rive URL events disabled: " MSG)
#  else
#    define RIVE_EXODUS_HARDEN_ASSERT(MSG) ((void)0)
#  endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    using namespace rive_android;

    const char* GetTargetValue(rive::OpenUrlEvent* urlEvent)
    {
        switch (urlEvent->targetValue())
        {
            case 0:
                return "_blank";
            case 1:
                return "_parent";
            case 2:
                return "_self";
            case 3:
                return "_top";
        }
        return "_blank";
    };

    jobject GetProperties(JNIEnv* env, rive::Event* event)
    {
        jmethodID putMethod = env->GetMethodID(
            GetHashMapClass(),
            "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

        jobject propertiesObject =
            env->NewObject(GetHashMapClass(), GetHashMapConstructorId());

        if (event == nullptr)
        {
            return propertiesObject;
        }

        for (auto child : event->children())
        {
            if (child->is<rive::CustomProperty>())
            {
                if (!child->name().empty())
                {
                    jstring jKey = env->NewStringUTF(child->name().c_str());
                    switch (child->coreType())
                    {
                        case rive::CustomPropertyBoolean::typeKey:
                        {
                            jboolean jValueBoolean =
                                child->as<rive::CustomPropertyBoolean>()
                                    ->propertyValue();
                            jobject booleanValue =
                                env->NewObject(GetBooleanClass(),
                                               GetBooleanConstructor(),
                                               jValueBoolean);
                            JNIExceptionHandler::CallObjectMethod(
                                env,
                                propertiesObject,
                                putMethod,
                                jKey,
                                booleanValue);

                            env->DeleteLocalRef(booleanValue);
                            break;
                        }
                        case rive::CustomPropertyString::typeKey:
                        {

                            jstring jValueString = env->NewStringUTF(
                                child->as<rive::CustomPropertyString>()
                                    ->propertyValue()
                                    .c_str());
                            JNIExceptionHandler::CallObjectMethod(
                                env,
                                propertiesObject,
                                putMethod,
                                jKey,
                                jValueString);

                            env->DeleteLocalRef(jValueString);
                            break;
                        }
                        case rive::CustomPropertyNumber::typeKey:
                        {

                            jfloat jValueFloat =
                                child->as<rive::CustomPropertyNumber>()
                                    ->propertyValue();
                            jobject floatValue =
                                env->NewObject(GetFloatClass(),
                                               GetFloatConstructor(),
                                               jValueFloat);
                            JNIExceptionHandler::CallObjectMethod(
                                env,
                                propertiesObject,
                                putMethod,
                                jKey,
                                floatValue);

                            env->DeleteLocalRef(floatValue);
                            break;
                        }
                    }

                    env->DeleteLocalRef(jKey);
                }
            }
        }

        return propertiesObject;
    }

    JNIEXPORT jstring JNICALL
    Java_app_rive_runtime_kotlin_core_RiveOpenURLEvent_cppURL(JNIEnv* env,
                                                              jobject thisObj,
                                                              jlong ref)
    {
        // Dev: assert; Release: strip
        RIVE_EXODUS_HARDEN_ASSERT("cppURL");
        return env->NewStringUTF("");
    }

    JNIEXPORT jstring JNICALL
    Java_app_rive_runtime_kotlin_core_RiveOpenURLEvent_cppTarget(
        JNIEnv* env,
        jobject thisObj,
        jlong ref)
    {
        // Dev: assert; Release: benign default
        RIVE_EXODUS_HARDEN_ASSERT("cppTarget");
        return env->NewStringUTF("_blank");
    }

    JNIEXPORT jstring JNICALL
    Java_app_rive_runtime_kotlin_core_RiveEvent_cppName(JNIEnv* env,
                                                        jobject thisObj,
                                                        jlong ref)
    {
        rive::Event* event = reinterpret_cast<rive::Event*>(ref);
        return env->NewStringUTF(event->name().c_str());
    }

    JNIEXPORT jshort JNICALL
    Java_app_rive_runtime_kotlin_core_RiveEvent_cppType(JNIEnv* env,
                                                        jobject thisObj,
                                                        jlong ref)
    {
        rive::Event* event = reinterpret_cast<rive::Event*>(ref);
        return event->coreType();
    }

    JNIEXPORT jobject JNICALL
    Java_app_rive_runtime_kotlin_core_RiveEvent_cppProperties(JNIEnv* env,
                                                              jobject thisObj,
                                                              jlong ref)
    {

        rive::Event* event = reinterpret_cast<rive::Event*>(ref);
        return GetProperties(env, event);
    }

    JNIEXPORT jobject JNICALL
    Java_app_rive_runtime_kotlin_core_RiveEvent_cppData(JNIEnv* env,
                                                        jobject thisObj,
                                                        jlong ref)
    {
        jclass hashMapClass = GetHashMapClass();
        jmethodID hashMapConstructor = GetHashMapConstructorId();
        jmethodID putMethod = env->GetMethodID(
            hashMapClass,
            "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        jobject eventObject = env->NewObject(hashMapClass, hashMapConstructor);

        rive::Event* event = reinterpret_cast<rive::Event*>(ref);

        if (event == nullptr)
        {
            return eventObject;
        }

        JNIExceptionHandler::CallObjectMethod(
            env,
            eventObject,
            putMethod,
            env->NewStringUTF("name"),
            env->NewStringUTF(event->name().c_str()));

        if (event->is<rive::OpenUrlEvent>())
        {
            // Dev: assert; Release: don’t expose url/target
            RIVE_EXODUS_HARDEN_ASSERT("cppData(OpenUrlEvent)");
            jobject type = env->NewObject(GetShortClass(),
                                          GetShortConstructor(),
                                          event->coreType());
            JNIExceptionHandler::CallObjectMethod(env,
                                                  eventObject,
                                                  putMethod,
                                                  env->NewStringUTF("type"),
                                                  type);
        }

        JNIExceptionHandler::CallObjectMethod(env,
                                              eventObject,
                                              putMethod,
                                              env->NewStringUTF("properties"),
                                              GetProperties(env, event));
        return eventObject;
    }

#ifdef __cplusplus
}
#endif