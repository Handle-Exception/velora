# 📜 Code Style Guidelines

## 1. Naming Conventions

### 1.1 Variables
- Use **`snake_case`** for all local and instance variables.
- Example:
  ```cpp
  int user_id;
  std::string file_path;
  ```

### 1.2 Private Members
- Prefix **private class members** with an underscore `_`.
- Example:
  ```cpp
  class User {
  private:
      int _user_id;
      std::string _user_name;
  };
  ```

### 1.3 Static and Global Variables
- Static class members and global constants must use **`UPPER_SNAKE_CASE`** (all capital letters, underscores between words).
- Example:
  ```cpp
  static int MAX_CONNECTIONS;
  
  const int GLOBAL_TIMEOUT = 30;
  ```

### 1.4 Functions and Methods
- Functions and methods must be named in **`camelCase`**.
- Example:
  ```cpp
  void initializeDatabase();
  int calculateChecksum(const std::string& data);
  ```

### 1.5 Classes, Structs, Enums
- Class, struct, and enum names must use **`PascalCase`** (each word capitalized, no underscores).
- Example:
  ```cpp
  class NetworkManager;
  struct FileDescriptor;
  enum class ConnectionState { Connected, Disconnected };
  ```

### 1.6 Macros
- Preprocessor macros must use **`ALL_CAPS_SNAKE_CASE`**.
- Example:
  ```cpp
  #define MAX_BUFFER_SIZE 1024
  ```

---

## 2. File Naming
- Source files (`.cpp`) and header files (`.hpp`, `.h`) should use **`snake_case`**.
- Example:
  ```
  user_manager.cpp
  file_reader.hpp
  ```

## 3. Constants
- Constants must be defined as `constexpr` or `const` when possible.
- Naming must follow **UPPER_SNAKE_CASE**.

---

## 4. Order of Class Members
Organize class members in the following order:
1. Public methods
2. Public members (if any)
3. Protected methods and members
4. Private methods
5. Private members

---

## 5. Example

```cpp
class FileManager {
public:
    FileManager();
    ~FileManager();

    void openFile(const std::string& file_path);

private:
    int _file_descriptor;
    std::string _file_path;

    static const int _DEFAULT_MODE = 0644;
};
```
