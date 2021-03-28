# SquiceLang

SquiceLang is dynamically typed scripting language. It was meant to be something new but at the end it's more like mix of javascript, python and some weird ideas. It's main advantage is small code base which can be packed into single C header.

Code consist of two parts: compiler and virtual machine. Compiler is parsing input and emits bytecode for it. VM is interpreting this bytecode and take care of memory management using custom GC implementation.

[Try it online!](https://timu5.github.io/SquiceLang/)


## Features

- No external libraries other than libc
- Whole code can be included into project as a single C header
- Less than 4K lines of code
- Easily extensible
- Compiler and runtime(virtual machine) can work separately
- Garbage collector take care of script memory management 
- Types: Null, Number(float64), String, List, Tuple, Dictionary, Function, Reference
- Exceptions handling through try catch blocks
- Classes with inheritance
- String interpolation
- Modules import

## Getting started

1. Download SquiceLang header distribution from [here](https://github.com/Timu5/SquiceLang/releases)
2. Copy it to project directory
3. Create main.c

```c

#include <stdio.h>

#define SL_IMPLEMENTATION
#include "SquiceLang.h"

int main(int argc, char **argv)
{
    sl_ctx_t *ctx = sl_ctx_new(NULL);
    sl_builtin_install(ctx);

    if (!sl_eval_str(ctx, "print(\"Hello World!\");", NULL, NULL))
    {
        puts(sl_ex_msg);
    }

    return 0;
}
```

## Example

```javascript

// single line comment

/*
******************
multi line comment
******************
*/

let a = 2;
let b = a * 4;

let c, d = 3, 5;

print("Hello world :)");
print("a = ", a);
print(f"b = ${b}");

if(b > 0) print("b > 0");

if(b >= a)
{
    print("b >= a");
}
else
{
    print("b < a");
}

let i = 0;
while(i < 10)
{
    print(i);
    i = i + 1;
    if(i == 8)
      break;
}

let array = [1, 2, 3, 4, 5, 6];

print("length or array = ", len(array));

i = 0;
while(i < len(array))
{
    print("array[", i, "] = ", array[i]);
    i = i + 1;
}

fn fib(x)
{
    if(x == 0) return 0;
    if(x == 1) return 1;
    return fib(x - 1) + fib(x - 2);
}

i = 0;
while(i < 10)
{
    print(f"fib(${i}) = ", fib(i));
    i = i + 1;
}

let d = { field: 1, field2: 2 };

print("d.field = ", d.field);
print("d.field2 = ", d.field2);

d.field2 = 100;
print("d.field2 = ", d.field2);

d.fib = fib;
print("d.fib(8) = ", d.fib(8));

class MyClass
{
    fn MyClass()
    {
        this.myfield = 1;
    } 
    
    fn do() {
        return this.myfield + 1;
    }
}

let myobj = MyClass();
print(myobj.myfield);
print(myobj.do());

try
{
    throw "some exception";
}
catch
{
    print(exception);
}

```
