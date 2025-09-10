cmake_minimum_required(VERSION 3.10)

project(myshell)

add_executable(myshell main.c
        parser.c
        path_search.c
        shell.c
        parser.h
        path_search.h
        shell.h)
