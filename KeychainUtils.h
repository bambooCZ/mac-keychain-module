//
//  KeychainUtils.h
//  Test
//
//  Created by Miloslav Mrvík on 23.10.14.
//  Copyright (c) 2014 Miloslav Mrvík. All rights reserved.
//

#ifndef KeychainUtils_h
#define KeychainUtils_h

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include "CFUtils.h"

using namespace std;

namespace KeychainUtils {

    typedef struct {
        string server;
        string username;
        string protocol;
        string path;
        string persistentRef;
        int port;

        string& getURL() {
            string* url = new string(protocol);
            url->append("://");
            url->append(server);
            if (port > 0) {
                url->append(":");

                stringstream ss;
                ss << port << endl;

                string portStr;
                ss >> portStr;
                url->append(portStr);
            }
            url->append(path);

            return *url;
        }
    } InternetRecord;

    string& getStandardProtocol(CFTypeRef protocol) {
		string* stdProto = NULL;

		if (protocol) {
			CFRetain(protocol);

			if (CFEqual(protocol, kSecAttrProtocolFTP)) {
				stdProto = new string("ftp");
			} else if (CFEqual(protocol, kSecAttrProtocolFTPS)) {
				stdProto = new string("ftps");
			} else if (CFEqual(protocol, kSecAttrProtocolHTTP)) {
				stdProto = new string("http");
			} else if (CFEqual(protocol, kSecAttrProtocolHTTPS)) {
				stdProto = new string("https");
			} else {
				stdProto = &CFUtils::getString((CFStringRef)protocol);
			}

			CFRelease(protocol);
		} else {
			stdProto = new string("");
		}

        return *stdProto;
    }

    CFMutableDictionaryRef getQuery() {
        CFMutableDictionaryRef query = CFDictionaryCreateMutable(kCFAllocatorDefault, 8, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFDictionarySetValue(query, kSecClass, kSecClassInternetPassword);
        CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitAll);
        CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanTrue);
        CFDictionarySetValue(query, kSecReturnData, kCFBooleanFalse);
        CFDictionarySetValue(query, kSecReturnPersistentRef, kCFBooleanTrue);
        CFDictionarySetValue(query, kSecReturnRef, kCFBooleanFalse);

        return query;
    }

    CFMutableDictionaryRef getQueryForService(const char* service) {
        CFMutableDictionaryRef query = getQuery();

        CFStringRef cfService = CFStringCreateWithCString(kCFAllocatorDefault, service, kCFStringEncodingUTF8);
        CFDictionarySetValue(query, kSecAttrServer, cfService);

        return query;
    }

    CFMutableDictionaryRef getQueryForServiceAndAccount(const char* service, const char* account) {
        CFMutableDictionaryRef query = getQueryForService(service);

        CFStringRef cfAccount = CFStringCreateWithCString(kCFAllocatorDefault, account, kCFStringEncodingUTF8);
        CFDictionarySetValue(query, kSecAttrAccount, cfAccount);

        return query;
    }

    CFTypeRef performQuery(const CFMutableDictionaryRef query) {
        CFTypeRef result = NULL;
        OSStatus status = SecItemCopyMatching(query, &result);

        if (status == errSecSuccess) {
            return result;
        } else if (status == errSecItemNotFound) {
            return NULL;
        } else {
            CFStringRef cfErrorString = SecCopyErrorMessageString(status, NULL);
            string error;
            if (cfErrorString) {
                error = CFUtils::getString(cfErrorString);
                CFRelease(cfErrorString);
            } else {
                error = "UNKNOWN_ERROR";
            }

            cerr << "[KeychainUtils] KeychainError (Code: " << status << "): " << error << endl;
            return NULL;
        }
    }

    vector<InternetRecord>* getInternetRecordsForQuery(const CFMutableDictionaryRef query) {
        vector<InternetRecord>* output = NULL;

        CFTypeRef result = performQuery(query);

        if (result && CFGetTypeID(result) == CFArrayGetTypeID()) {
            CFArrayRef records = (CFArrayRef)CFRetain(result);
            CFIndex recordsCount = CFArrayGetCount(records);

            output = new vector<InternetRecord>();
            for (CFIndex i = 0; i < recordsCount; ++i) {
                CFTypeRef value = CFRetain(CFArrayGetValueAtIndex(records, i));
                if (value && CFGetTypeID(value) == CFDictionaryGetTypeID()) {
                    InternetRecord rec;
                    rec.protocol = getStandardProtocol(CFDictionaryGetValue((CFDictionaryRef) value, kSecAttrProtocol));
                    if (rec.protocol == "http" || rec.protocol == "https") {
                        rec.username = CFUtils::getString((CFStringRef)CFDictionaryGetValue((CFDictionaryRef) value, kSecAttrAccount));
                        rec.server = CFUtils::getString((CFStringRef)CFDictionaryGetValue((CFDictionaryRef) value, kSecAttrServer));
                        rec.port = (int)CFUtils::getInt((CFNumberRef)CFDictionaryGetValue((CFDictionaryRef) value, kSecAttrPort));
                        rec.path = CFUtils::getString((CFStringRef)CFDictionaryGetValue((CFDictionaryRef) value, kSecAttrPath));
                        rec.persistentRef = CFUtils::getB64String((CFDataRef)CFDictionaryGetValue((CFDictionaryRef) value, kSecValuePersistentRef));
                        output->push_back(rec);
                    }

                } else {
                    cerr << "[KeychainUtils] KeychainUtils::getInternetRecords error: CFArrayGetValueAtIndex did not return valid value at index " << i << endl;
                }
                CFRelease(value);
            }

            CFRelease(records);
        } else {
            output = new vector<InternetRecord>();
        }

        return output;
    }

    vector<InternetRecord>* getInternetRecordsForService(const char* service) {
        CFMutableDictionaryRef query = getQueryForService(service);
        vector<InternetRecord>* output = getInternetRecordsForQuery(query);
        CFRelease(query);
        return output;
    }

    string* getPasswordForItem(const string& persistentRefB64) {
        CFDataRef persistentRef = CFUtils::getCFDataFromB64(persistentRefB64);
        CFMutableArrayRef itemList = CFArrayCreateMutable(kCFAllocatorDefault, 1, &kCFTypeArrayCallBacks);
        CFArrayInsertValueAtIndex(itemList, 0, persistentRef);

        CFMutableDictionaryRef query = getQuery();
        CFDictionarySetValue(query, kSecMatchLimit, kSecMatchLimitOne);
        CFDictionarySetValue(query, kSecMatchItemList, itemList);
        CFDictionarySetValue(query, kSecReturnAttributes, kCFBooleanFalse);
        CFDictionarySetValue(query, kSecReturnData, kCFBooleanTrue);
        CFDictionarySetValue(query, kSecReturnPersistentRef, kCFBooleanFalse);
        CFDictionarySetValue(query, kSecReturnRef, kCFBooleanFalse);

        CFTypeRef result = performQuery(query);

        string* output = NULL;
        if (result && CFGetTypeID(result) == CFDataGetTypeID()) {
            CFDataRef cfPassword = (CFDataRef)CFRetain(result);
            output = &CFUtils::getStringUtf8(cfPassword);
            CFRelease(cfPassword);
        }

        CFRelease(itemList);
        CFRelease(query);

        return output;
    }

}

#endif
