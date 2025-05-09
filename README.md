# 🚀 Velora

## 📦 Dependencies

This project requires the following libraries:

- [**Asio**](https://think-async.com/Asio/)
- [**spdlog**](https://github.com/gabime/spdlog)
- [**GLM**](https://github.com/g-truc/glm)
- [**Protobuf**](https://protobuf.dev/)

## ⚙️ Configuring the project

To configure the project using CMake, run the following command:

```sh
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=install
```

- `-S .` : Specifies the source directory.
- `-B build` : Specifies the build directory.
- `-G "Visual Studio 17 2022"` : Use Visual Studio generator
- `-A x64` : Sets the target architecture to 64-bit
- `-DBUILD_TESTING=ON` : Enables tests.
- `-DCMAKE_INSTALL_PREFIX=install` : Specifies install directory.

## 🛠️ Building the project

To build the project in **Debug Mode**, use:

```sh
cmake --build build --config Debug
```

This will compile the project with debugging enabled.

To build the project in **Release Mode with Debug info enabled**, use:

```sh
cmake --build build --config RelWithDebInfo
```

To build the project in **Release Mode**, use:

```sh
cmake --build build --config Release
```

## 🛠️ Installing the Project

To install project in **Debug Mode**, use:

```sh
cmake --build build --config Debug --target install
```

This will install the project with debugging enabled.

To install project in **Release Mode**, use:

```sh
cmake --build build --config Release --target install
```

## 🖥️ Start the executable

```sh
./install/bin/Velora.exe
```
