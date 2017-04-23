# tloimageeditor

A simple image editor.

## Build Requirements

* CMake
* C++14 development environment for which CMake can generate build files
* Qt 5

## Clone, Build, and Run

Clone into tloimageeditor directory.

```
$ git clone --branch develop <url/to/tloimageeditor.git>
```

Build.

```
$ mkdir tloimageeditorbuild
$ cd tloimageeditorbuild
$ cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug ../tloimageeditor
$ make
```

Run.

```
$ ./tloimageeditor
```
