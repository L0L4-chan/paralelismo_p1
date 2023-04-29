cmake_minimum_required(VERSION 3.25)
project(paralelismo_p1 C)

set(CMAKE_C_STANDARD 11)

add_executable(paralelismo_p1
        main.c paralelismo1.c paralelismo2a.c paralelismo2b.c paralelismo1.c paralelismo2b.c paralelismo2b.c cmake-build-debug/paralelismo3.c cmake-build-debug/paralelismo3.3.c paralelismo3.3.c)
