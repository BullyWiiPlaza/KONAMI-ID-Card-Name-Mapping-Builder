# KONAMI ID -> Card Name Mapping Builder
This C++ project allows you to build a KONAMI ID -> card name mapping based on the [YGOProDeck](https://ygoprodeck.com/api-guide) API. A C++ header file is output which you can easily drop into your C++ code.

# Compiling
Load the project `*.sln` file in Visual Studio. Make sure you installed the required libraries annotated in the source code.

## Performance
On my machine, the debug build takes about 14s to complete while the release build takes 3.4s so make sure to compile in release mode for good performance. Most of the performance is "wasted" on parsing the JSON in the debug build. In the release build, the downloading dominates the runtime with about 3s since the downloaded JSON is relatively big. Optimized parsing however is very efficient now with only ~400ms runtime.
