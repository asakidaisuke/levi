# Levi

## Summary

This repository is an implementation of a programming language(I named it Levi which is named after an Attack on Titan charactor) written from scratch.

The main components are a parser, a compiler and a VM, which you can refer to in the src directory.
Those components are written in C++ following this awesome book [Crafting Interpreter](https://craftinginterpreters.com/).

## Quick start
The development was done on Ubuntu20.04 but it should work on other OSs.

The following comannds will build a executable file,

`mkdir build; cd build; cmake ..; make;`

The following comannds will execute a script,

`./levi sample.lev`

The sample files are located in the samples directory, so please refer to them.
