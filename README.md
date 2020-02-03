# SquiceLang

SquiceLang is simple dynamically typed programing language. It was meant to be something new but at the end it's more like featureless JS cousine. It's main advantage is engine source code simplicity.

Code consist of two parts: compiler and virtual machine. Compiler parse input and emit bytecode. VM interpret bytecode and take care of memory menagment using simple GC implementation.

## Example

```javascript

let a = 2 * 2;
let b = a * 4;

if(b >= a) {
    print("b >= a");
} else {
    print("b < a");
}

let i = 0;
while(i < 10) {
    print(i);
    i = i + 1;
}

let array = [1, 2, 3, 4, 5, 6];
//or
//let array = list(1, 2, 3, 4, 5, 6);

print("lenght or array = ", len(array));

let i = 0;
while(i < len(array)) {
    print("array[", i, "] = ", array[i]);
    i = i + 1;
}

fn fib(x) {
    if(x == 0) return 0;
    if(x == 1) return 1;
    return fib(x - 1) + fib(x - 2);
}

let i = 0;
while(i < 10) {
    i = i + 1;
    print("fib(", i, ") = ", fib(i));
}

let d = dict();
d.field = 1;
d.field2 = 2;

print("d.field = ", d.field);
print("d.field2 = ", d.field2);

fn test() {
	print("this.field2 = ", this.field2);
}

d.test = test;
d.test();

let a = { a:1,b:2 };
print(a.a);
print(a.b);
```
