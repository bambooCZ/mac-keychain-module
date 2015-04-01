{
  'targets': [
    {
      'target_name': 'MacKeychainModule',
      'sources': [ 'MacKeychainModule.cpp','base64.cpp' ],
      'libraries': [ '/System/Library/Frameworks/Security.framework/Security', '/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation' ]
    }
  ]
}
