## Unified Function IO
#### 2018-02-24

### Intro
Functions/methods are an essential part of most programming languages, and one of the aspects likely to offer different capabilities depending on language. This post describes the steps taken by [Cixl](https://github.com/basic-gongfu/cixl) towards unifying its syntax and semantics for function arguments and results.

### Baby Steps
[Cixl](https://github.com/basic-gongfu/cixl) started out with support for multiple dispatch and single results, returning the top of the stack on exit.

```
   func: int-add(x y Int) $x $y +;
   | 1 2 int-add
...
[3]
```

The first improvement was support for referring to previous arguments types. The function below only matches when the second argument is type compatible with the first.

```
   func: same-add(x Num y Arg0) $x $y +;
   | 1 2 same-add
...
[3]
```

Since I've long been a fan of declarative approaches such as Haskell's pattern matching, the next thing added was support for literal arguments.

```
   func: is-fortytwo1(x Int) #f;
   func: is-fortytwo1(42) #t;
   | 21 is-fortytwo1
...
[#f]

   | 42 is-fortytwo1
...
[#t]
```
Next up was multiple results, which added the requirement of specifying result lists for all functions. Result types are checked, but don't take part in dispatch. The function below matches any two values, returns them in reverse order and checks that the resulting types are reversed.

```
   func: flip(x y)(Arg1 Arg0) $y $x;
   | 1 2 flip
...
[2 1]
```

### One Step Back
At this point the code for dealing with arguments and results was turning complex enough that the duplication began to bother me, so I took a step back and started combing through the differences.

#### Anonymous Arguments
Forth encourages using the stack and not naming values; since Cixl supports some of the same techniques, extending that support to functions seemed like step in the right direction. Anonymous arguments may be declared using ```_``` and are pushed on the function stack in order of appearance.

```
   func: repeat(_ Int c Char)(_ Str) {_ $c} map str;
...| 3 @a repeat
...
['aaa'r1]
```

#### Named Results
Named results is one of those features that people either love or hate; I guess it depends on which particular implementation of the idea you've been confronted with. Cixl reads the specified variable and pushes the value on the calling stack.

```
   func: double(x Int)(result Int)
...  let: result $x 2 *;
...  [$x '*2 = ' $result] say;
...| 21 double
...
21*2 = 42
[42]
```

#### Literal Results
While I wouldn't have thought of adding support for literal results unless pushed in that direction; the more I thought about it, the more arbitrary it seemed to only support literal arguments. The specified value is pushed on the calling stack. 

```
   func: is-fortytwo2(_ Int)(#f) _;
...func: is-fortytwo2(42)(#t);
...| 21 is-fortytwo2
...
[#f]

...| 42 is-fortytwo2
...
[#t]
```

### Limitations
Referring to literal arguments by index signals an error, since that's most probably not what was indended. Referring to arguments by index in a result list requires the argument to be named; since Cixl needs to be able to get its type at the point of return.

Give me a yell if something is unclear, wrong or missing. And please consider helping out with a donation via [paypal](https://paypal.me/basicgongfu) or [liberapay](https://liberapay.com/basic-gongfu/donate) if you find this worthwhile, every contribution counts. You may find more posts in the same spirit [here](https://github.com/basic-gongfu/cixl/tree/master/devlog).