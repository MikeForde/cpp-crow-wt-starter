# Crow + Wt starter

A C++17 demo with a Crow JSON API and a Wt web UI. The UI calls the API asynchronously using Wt's HTTP client. It supports personalized greetings and creating and viewing tasks. The API also supports updating and deleting tasks.

Tasks are stored in memory and are lost when the API restarts.

## Requirements

- CMake 3.20 or later, a C++17 compiler, Git, and a build tool such as Ninja.
- Wt with its HTTP connector, standalone Asio, and Boost. CMake fetches Crow v1.3.1 from GitHub during configuration.
- Alternatively, use the included VS Code Dev Container (Ubuntu 24.04), which installs the build dependencies and builds Wt 4.14.4. Docker and the VS Code Dev Containers extension are required.

## Build and run

Open the folder in VS Code and select **Dev Containers: Reopen in Container**, or use a local environment with the dependencies installed. Then build:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel 2
```

Start the API in one terminal:

```bash
./build/crow_api
```

Start the UI in another terminal:

```bash
./build/wt_ui --docroot . --http-address 0.0.0.0 --http-port 8080
```

Open http://localhost:8080/. Enter an optional name and click **Call Crow API**. Use **Add Task** to create a task and **Refresh Tasks** to load the current list. The UI does not currently provide controls to update or delete tasks.

The API listens on port 18080; the UI connects to `http://localhost:18080` from the Wt server process. Both processes must therefore share a localhost network or the UI's API base URL must be changed in `frontend/HelloApp.cpp`. The Dev Container forwards ports 8080 and 18080.

## API

| Method | Path | Behavior |
| --- | --- | --- |
| GET | `/api/hello?name=...` | Returns `{"message":"Hello, World!"}` by default, or a greeting for the supplied name. |
| GET | `/api/tasks` | Returns `{"tasks":[...]}`. |
| POST | `/api/tasks` | Creates a task from a JSON `title` and returns it with HTTP 201. |
| PATCH | `/api/tasks/<id>` | Sets a task's `completed` boolean and returns the updated task. |
| DELETE | `/api/tasks/<id>` | Deletes a task and returns HTTP 204 with no body. |

Each task has an integer `id`, a string `title`, and a boolean `completed` (initially `false`). Titles must be 1–200 bytes. Invalid input returns HTTP 400; updating or deleting a nonexistent task returns HTTP 404.

For example, with the API running:

```bash
curl 'http://localhost:18080/api/hello?name=Alex'
curl http://localhost:18080/api/tasks
curl -X POST http://localhost:18080/api/tasks \
  -H 'Content-Type: application/json' -d '{"title":"Try the demo"}'
curl -X PATCH http://localhost:18080/api/tasks/1 \
  -H 'Content-Type: application/json' -d '{"completed":true}'
curl -X DELETE http://localhost:18080/api/tasks/1
```

The PATCH and DELETE examples assume the newly created task has ID 1; use the returned ID if other tasks have already been created.

## Project layout

- `backend/`: Crow routes, greeting service, and mutex-protected in-memory task repository.
- `frontend/`: Wt application, asynchronous API client, and UI task model.
- `CMakeLists.txt`: dependency configuration and the `crow_api` and `wt_ui` targets.
- `.devcontainer/`: development container image and VS Code configuration.
