# Advent of Code 2019 solutions

Code written in C++, the goal was to explore new (C++17/C++20) language features.

Things I learned:

## Coroutines
I didn't really go into into the technical details because I didn't have to.
Coroutines are really cool and they work mostly as you would expect from other languages,
but the lack of online resources makes them a bit confusing in some cases.
(could figure out how to write a for loop, but not how to get just one element)

## Structured bindings
Again, I didn't really go into the gory details because they just worked as I wanted them to.
These construct make code immensely more readable, I like them a lot.

## Floating point template parameters
Are not allowed by the standard. 

## Modulo vs. remainder
% means remainder in C. The sign of `a rem b` is that of `a`, while in `a mod b` the sign is that of b.

## `priority_queue`
Is a priority queue in C++ (obviously) and by default the value with the highest priority is on top. This means
that if you want to want model something based on `cost` instead of `priority`, you usually need to invert the 
comparator logic.

## OOP makes sense
Somehow I'm used to writing a lot of free functions and more of procedural-oriented style of code.

But a lot of solutions I wrote could have been much cleaner if I bothered to put things in classes,
even if just for prettier-syntax sake.

## Using `using`
I should have used `using` declarations for type aliasing because:

1. things swiftly go out of hand and become unreadable due to long type names
2. when a task forces you to change things (e.g. someone tells you IntCode now has infinite memory and you
have to switch from using a `vector` to a `map`)
refactoring is much easier if you only need to change a single line

## Reference to temporary variables
Make your code go boom. 
So don't accidentaly return `type&` from a function unless you are sure that's realy what you want.

## Competitive programming
I'm not a fan of it. These problems were cool up to a certain point, but after a while it just got annoying and
I don't think knowing some of these tricks will benefit me in real-world programming.
