# GachiC

Gachimuchi-themed programming language (Work in progress)

## Building from source

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

## How to use

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
## Examples
For convenience, there is no "main" function, there is only "master", for which you do not need to describe function declaration ("slave" for others)

```swift
extern swallow(x:*i8) cum nothing;
master() cum i32
{
  swallow("Hello, Gym\n\0");
  cumming(0);
};
```
To return a value from a function, you must use the unary operator (!) "cumming"

If you can describe a function in one line, then there is no need for a block declaration
```swift
slave Bob(x: i32) cum i32
  cumming(-x);
```
#### Loops
```swift
extern swallow(x:*i8) cum nothing;
master() cum i32
{
  var i = 0;
  while(i < 5) |i = i + 1|
    swallow("dupl\n\0");

  cumming(0);
};
```

#### Сonditional statements
```swift
extern read() cum i32;
extern print(x:*i8) cum nothing;

master() cum i32
{
  var n = read();

  if(n % 2 < 1){
    print("That's good\n\0");
  } else{
    print("Try again\n\0");
  };

  cumming(0);
};
```

#### Structure types
```swift
extern printInt(x: i32) cum nothing;

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

```swift
  var i = [1, 2, 3, 4];
  printInt(i[2]);
```

#### Pointers

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