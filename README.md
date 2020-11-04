# GachiC

Gachimuchi-themed programming language (Work in progress)

# Table of Contents

* [Build](#build)
* [Usage](#Usage)
* [Features](#features)
    * [Functions](#functions)
    * [Variables](#variable-initialization)
    * [Loops](#loops)
    * [Сonditional statements](#conditional-statements)
    * [Structures](#structure-types)
    * [Arrays](#arrays)
    * [Memory management](#memory-management)
* [License](License)
* [Mates](#mates)

## Build

### Dependencies

##### POSIX

* cmake >= 3.15
* gcc >= 9.3.0 or clang >= 11.0
* LLVM, Clang, LLD libs == 11.

##### Windows

* cmake >= 3.15
 * Microsoft Visual Studio. Supported versions:
   - 2017 (version 15.8)
   - 2019 (version 16)
* LLVM, Clang, LLD libs == 11.

### Instructions

##### POSIX

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```

##### Windows

* Ah, shit, you have to either compliment from source, or download ready-made clang+llvm+lld libs
* Open "x64 Native Tools Command Prompt"

```sh
cmake .. -Thost=x64 -G "Visual Studio 16 2019" -A x64 -DCMAKE_PREFIX_PATH=path\to\llvm -DCMAKE_BUILD_TYPE=Release
msbuild -p:Configuration=Release ALL_BUILD.vcxproj 
```

* Or you can use wsl(see [POSIX](#posix))

## Usage

First, compile source files into object files
```sh
$ gc -c file.gc
```
Then link them
```sh
$ clang++ file.o helper.o -o program
```
Run 
```sh
$ ./program
```
That's all
## Features
**While the language is being developed, some features may change**
#### Functions

For convenience, there is no "main" function, there is only "master", for which you do not need to describe function declaration ("slave" for others)

```swift
extern swallow(x:*i8) cum nothing;
master() cum i32
{
  swallow("Hello, Gym\n");
  cumming(0);
};
```
To return a value from a function, you must use the unary operator (!) "cumming"

If you can describe a function in one line, then there is no need for a block declaration
```swift
slave Bob(x: i32) cum i32
  cumming(-x);
```

#### Variable initialization

When a variable is declared, it does not need to be assigned a type, its type will be determined automatically depending on the value assigned to it.
```swift
  // i32
  var h = 123;
  // f32
  var e = 123.123;
  // [4]i32
  var l = [1, 2, 3, 4];
  // *i8
  var p = "it's your job to cook for me";
  // Foo
  var m = Foo{e: 123, b: 123.123, a: "string"};
  // [3]*i8
  var e_ = ["what the hell", "where is my breakfast", "don't get smart with me"]; 
```
If you want to set the type of a variable explicitly, then use the casting operator "ass"
```swift
  // i32
  var h32 = 123 ass i32;
  // i64
  var h64 = 123 ass i64;
  // f64
  var h64 = 123 ass f64;
  // f32
  var e   = 123.123 ass f32;
  // i32
  var e_  = 123.123 ass i32;
  // *i8
  var ptr_ = 1234 ass *i8;
```

#### Loops

Syntax:
```swift
  while(cond) |step|
    body;
```
Example:
```swift
  while(i < 5) |i = i + 1|
    swallow("dupl\n");
```

#### Сonditional statements

Syntax:
```swift
  var n = read();

  if(cond){
    true_block
  } else {
    false_block
  };

  if(cond)
    true_block;
```
Example:
```swift
  var n = read();

  if(n % 2 == 0)
    print("That's good\n")
  else
    print("Try again\n");
```

#### Structure types

Example:
```swift
struct Poo{
  a : i32;
};

struct Foo{
  a : *i32;
  b : Poo;
};

master() cum i32
{
  var i = 10;
  var e = Foo{a: &i, b: Poo{a: i}};

  printInt(e.b.a);
  printInt(*(e.a));
  cumming(0);
};
```

#### Arrays

Example:
```swift
  var i = [1, 2, 3, 4];
  printInt(i[2]);
```
The type of array elements determines the type of its first element

```swift
  var i = [123 ass i8, 2, 3, 4];
  printInt(i[0]);
```

#### Memory management

```swift
extern printInt(x: i32) cum nothing;

struct Foo{
  a: *i32;
};

master() cum i32
{
  var tmp = 66;
  var y = Foo{a : &tmp};
  var n = &(y.a);
  **(n ass **i32) = 99;

  printInt(*(y.a));
  printInt((1 + **n));

  cumming(0);
};
```

## License
Distributed under the MIT License. See `LICENSE` for more information.

## Mates
* Good Memes [[VK](https://vk.com/good_memes)]
* Everlusting Suction [[VK](https://vk.com/everlusting_suction)][[Steam](https://steamcommunity.com/sharedfiles/filedetails/?id=1699955374)]

