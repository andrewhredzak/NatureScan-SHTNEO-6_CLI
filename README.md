| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-P4 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- | -------- |

# _NatureScan-NEO-6_


This is the simplest buildable example. The example is used by command `idf.py create-project`
that copies the project to user specified path and set it's name. For more information follow the [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project)



## How to use example
We encourage the users to use the example as a template for the new projects.
A recommended way is to follow the instructions on a [docs page](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html#start-a-new-project).

## Example folder contents

The project **sample_project** contains one source file in C language [main.c](main/main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt`
files that provide set of directives and instructions describing the project's source files and targets
(executable, library, or both). 

Below is short explanation of remaining files in the project folder.

## expected output:
Status: Active
UTC Time: 03:01:49.., Date: 14/03/1925
Latitude: 33.426° N, Longitude: 111.952° W
Speed: 7.38 knots, Course: 137.82°, Variation: °

Status: Active
UTC Time: 03:01:50.., Date: 14/03/1925
Latitude: 33.426° N, Longitude: 111.952° W
Speed: 5.76 knots, Course: 132.32°, Variation: °

Status: Active
UTC Time: 03:01:51.., Date: 14/03/1925
Latitude: 33.426° N, Longitude: 111.952° W
Speed: 4.34 knots, Course: 123.13°, Variation: °


## project directory:


NATURESCAN-NEO6/
            |──README.md                
            |──CMakeLists.txt
            |──sdkconfig
            └── main/       
            │    |──CMakeLists.txt
            │    |──main.c
            │    └── Kconfig.projbuild
            └── components/ 
                    |─NEO6/ 
                       |──CMakeLists.txt
                       |──NEO6.c
                       └──NEO6.h
                              




