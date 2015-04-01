//
//  CFUtils.h
//  Test
//
//  Created by Miloslav Mrvík on 23.10.14.
//  Copyright (c) 2014 Miloslav Mrvík. All rights reserved.
//

#ifndef Test_CFUtils_h
#define Test_CFUtils_h

#include <cstdio>
#include <iostream>
#include <string>
#include <CoreFoundation/CoreFoundation.h>
#include "base64.h"

using namespace std;

namespace CFUtils {

    char* getCString(CFStringRef cfString, bool* error = NULL) {

        char* output = NULL;
        if (error) *error = true;

        if (cfString) {
            CFRetain(cfString);

            assert(CFGetTypeID(cfString) == CFStringGetTypeID());
            CFIndex len = CFStringGetLength(cfString);



            if (len > 0) {
                char* buffer = (char*)malloc(((2 * len) + 1) * sizeof(char)); //UTF8 může být až 2-bajtové, takže len * 2
                if (CFStringGetCString(cfString, buffer, (2 * len) + 1, kCFStringEncodingUTF8)) {
                    size_t sLen = strlen(buffer);
                    output = (char*)malloc((sLen + 1) * sizeof(char));
                    strcpy(output, buffer);
                    if (error) *error = false;
                } else {
                    cerr << "[CFUtils] CFUtils::getCString error: CFStringGetCString returned an error!!!" << endl;
                }
                free(buffer);
            }

            CFRelease(cfString);
        }

        if (output == NULL) {
            output = (char*)malloc(sizeof(char));
            output[0] = 0;
        }

        return output;
    }

    string& getString(CFStringRef cfString, bool *error = NULL) {
        char* cString = getCString(cfString, error);
        string* output = new string(cString, strlen(cString));
        free(cString);
        return *output;
    }

    string& getB64String(CFDataRef cfData, bool *error = NULL) {
        CFIndex dataLength = CFDataGetLength(cfData);
        string* output = NULL;
        if (dataLength > 0) {
            UInt8 buffer[dataLength];
            CFDataGetBytes(cfData, CFRangeMake(0,dataLength), buffer);
            output = new string(base64_encode(buffer, dataLength));
        } else {
            output = new string("");
        }

        return *output;
    }

    string& getStringUtf8(CFDataRef cfData, bool *error = NULL) {
        CFIndex dataLength = CFDataGetLength(cfData);
        string* output = NULL;
        if (dataLength > 0) {
            UInt8 buffer[dataLength];
            CFDataGetBytes(cfData, CFRangeMake(0,dataLength), buffer);
            CFStringRef str = CFStringCreateWithBytes(kCFAllocatorDefault, buffer, dataLength, kCFStringEncodingUTF8, false);
            output = &getString(str, error);
            CFRelease(str);
        } else {
            output = new string("");
        }

        return *output;
    }

    CFDataRef getCFDataFromB64(const string& b64, bool *error = NULL) {
        string decoded = base64_decode(b64);
        return CFDataCreate(kCFAllocatorDefault, (const UInt8 *)decoded.c_str(), decoded.length());
    }

    long long getInt(CFNumberRef cfNumber, bool *error = NULL) {
        long long output = 0;
        if (error) *error = true;
        if (cfNumber) {
            if (error) *error = false;
            CFRetain(cfNumber);

            assert(CFGetTypeID(cfNumber) == CFNumberGetTypeID());

            if (!CFNumberGetValue(cfNumber, kCFNumberLongLongType, &output)) {
                cerr << "[CFUtils] CFUtils::getInt error: CFNumberGetValue returned an error!!!" << endl;
                if (error) *error = true;
            }

            CFRelease(cfNumber);
        }
        return output;
    }

    double getDouble(CFNumberRef cfNumber, bool *error = NULL) {
        double output = 0;
        if (error) *error = true;
        if (cfNumber) {
            if (error) *error = false;
            CFRetain(cfNumber);

            assert(CFGetTypeID(cfNumber) == CFNumberGetTypeID());

            if (!CFNumberGetValue(cfNumber, kCFNumberDoubleType, &output)) {
                cerr << "[CFUtils] CFUtils::getInt error: CFNumberGetValue returned an error!!!" << endl;
                if (error) *error = true;
            }
        }

        return output;
    }

}

#endif
