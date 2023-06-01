# KONAMI ID -> Card Name Mapping Builder
This C++ project allows you to build a KONAMI ID -> card name mapping based on the [YGOProDeck](https://ygoprodeck.com/api-guide) API. A C++ header file is output which you can easily drop into your C++ code.

# Compiling
Load the project `*.sln` file in Visual Studio. Upon building, a vcpkg installation (if you ran `vcpkg integrate install` before) will install the required libraries automatically (according to the `vcpkg.json` file). If you want to use global libraries, you can disable [manifest mode](https://vcpkg.readthedocs.io/en/latest/users/manifests) in the Visual Studio project settings since installing e.g. [Boost](https://www.boost.org) can take quite long).

## Performance
On my machine, the debug build takes about 14s to complete while the release build takes 3.4s so make sure to compile in release mode for good performance. Most of the performance is "wasted" on parsing the JSON in the debug build. In the release build, the downloading dominates the runtime with about 3s since the downloaded JSON is relatively big. Optimized parsing however is very efficient now with only ~400ms runtime.
