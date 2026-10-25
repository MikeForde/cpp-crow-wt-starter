# Crow + Wt starter

C++17 / CMake / VS Code Dev Containers / Linux ARM64 or x86-64.

## Start

1. Open this folder in VS Code.
2. Run **Dev Containers: Reopen in Container**.
3. Open a terminal and run:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
```

4. Run the API in terminal 1:

```bash
./build/crow_api
```

5. Test the API in terminal 2:

```bash
curl http://localhost:18080/api/hello
```

6. Run the UI in terminal 2:

```bash
./build/wt_ui --docroot . --http-address 0.0.0.0 --http-port 8080
```

7. Open http://localhost:8080/ and click **Call Crow API**.

The UI uses libcurl synchronously for its *first* learning example. Later, replace this with Wt's asynchronous HTTP client to avoid blocking the Wt event handler.
