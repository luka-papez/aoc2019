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
