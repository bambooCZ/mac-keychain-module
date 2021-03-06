# MacKeychainModule

> I am not a C++ guy, feel free to improve this

Node module for accessing macOS Keychain directly from JavaScript. This way it is more secure than using */usr/bin/security*, because when user selects *Always allow* option, he allows only your app to read the password, not to whoever can launch */usr/bin/security*.

## Building

```bash
cd MacKeychainModule ;
sudo npm install -g nw-gyp ;
nw-gyp configure --target=0.21.4 --arch=x64 ;
nw-gyp build --target=0.21.4;
```

Result will be in *build/Release* directory

## Usage example

```javascript
var MacKeychainModule = require('./build/Release/MacKeychainModule.node');

var getPasswordForAccount = function (host, username) {
    var accounts = MacKeychainModule.getAccountsForHost(host);
    for (var i = 0; i < accounts.length; i++) {
        var account = accounts[i];
        if (account.username == username) {
            return MacKeychainModule.getPasswordForRef(account.ref); // Keychain might ask user to allow access
        }
    }
    return null;
}

var pass = getPasswordForAccount("foo.com", "bar");
```
Variable now contains password for user *bar* at server *foo.com*

## JavaScript API

### MacKeychainModule.getAccountsForHost(host)

- Parameter {String} **host** - hostname of our interest
- Return value {Array} - array of accounts (see account below)

This method will list all stored accounts for given *host*

### MacKeychainModule.getPasswordForRef(ref)

- Parameter {String} **ref** - Base64 encoded persistentRef of the keychain item
- Return value {String|null} - the password

This method will return the password from keychain item identified by *ref*

### account

- account.**username** {String} - Username
- account.**path** {String} - Path part of URL
- account.**port** {Number} - Port number
- account.**protocol** {String} - Protocol part of URL
- account.**ref** {String} - Base64 encoded persistentRef of the keychain item

**No password in account? Why?**

Because reading password can show dialog to grant access to the password. This can be unwanted. Password can be obtained by calling:

```
MacKeychainModule.getPasswordForRef(account.ref)
```
