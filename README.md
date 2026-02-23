# Onion Logger

A lightweight C++20 logging utility that intercepts `std::cout` and `std::cerr`
and duplicates them into a log file.

It automatically prefixes each output line with a timestamp and thread ID.

---

## Features

* C++20 compliant
* Automatically intercepts `std::cout` and `std::cerr`
* Writes output both to terminal and log file (tee behavior)
* Adds timestamp + thread ID prefix per line
* `std::cerr` lines are printed in red in terminal (ANSI)
* Log file output remains plain (no colors)
* Clean RAII restore of original stream buffers
* Depends on `onion::datetime`

---

## Adding to Your CMake Project

If the library is included in your repository:

```cmake
add_subdirectory(libs/datetime)
add_subdirectory(libs/logger)

target_link_libraries(your_target
    PRIVATE
        onion::logger
)
````

> Note: `onion::logger` requires the `onion::datetime` target to already exist.

---

## Basic Usage

```cpp
#include <onion/Logger.hpp>
#include <iostream>

int main()
{
    onion::Logger logger("logs.txt");

    std::cout << "Hello from std::cout" << std::endl;
    std::cerr << "Hello from std::cerr" << std::endl;

    return 0;
}
```

---

## Output Format

Each line is automatically prefixed like:

```
17-02-2026 18:30:01 [T:12345] : LOG : Hello from std::cout
17-02-2026 18:30:01 [T:12345] : ERR : Hello from std::cerr
```

* `LOG` is used for `std::cout`
* `ERR` is used for `std::cerr`
* Thread ID is the `std::this_thread::get_id()` output

---

## Multi-thread Example

```cpp
#include <onion/Logger.hpp>

#include <chrono>
#include <iostream>
#include <thread>

static void Speak()
{
    for (int i = 0; i < 5; i++)
    {
        std::cout << "Thread speaking: " << i << std::endl;

        if (i == 2)
            std::cerr << "Fake error occurred" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    onion::Logger logger("./logs.txt");

    std::jthread t1(Speak);
    std::jthread t2(Speak);

    return 0;
}
```

---

## Enable Demo

Enable demo during configuration:

```bash
cmake -DONION_BUILD_DEMO=ON ..
```

---

## Design Notes

* Internally replaces the `std::cout` and `std::cerr` stream buffers (`rdbuf()`).
* Uses a custom `std::streambuf` implementation to duplicate output into:

  * terminal stream buffer
  * log file stream buffer
* Prefix is injected automatically at the start of each new line.
* ANSI escape codes are used to display errors in red in the terminal.
* Destruction restores the original stream buffers automatically.

---

## Dependency

This library requires:

* **onion::datetime**

The build will fail if the CMake target `onion::datetime` is not available.

