# XeSS Proxy DLL for RDR2 (Educational)

## Steps
1. Install Visual Studio 2022 with "Desktop development with C++".
2. Clone/copy Intel XeSS SDK (https://github.com/intel/xess).
3. Place `proxy_d3d12.cpp`, `d3d12_proxy.vcxproj`, and `build.bat` in a folder.
4. Edit `d3d12_proxy.vcxproj` → set `AdditionalIncludeDirectories` to point to XeSS headers, and `AdditionalDependencies` to link XeSS lib (or use LoadLibrary if no .lib).
5. Run `build.bat` to compile.
6. Copy resulting `d3d12.dll` into RDR2 folder (make backup first).
7. Run RDR2 in **offline singleplayer**. Check with DebugView to see `XeSS context initialized`.

## Notes
- This is a minimal skeleton. To actually apply XeSS, you need to hook SwapChain present and run `xessExecute` each frame.
- Works only for learning/testing. Production-ready integration requires more hooks and motion vectors.
