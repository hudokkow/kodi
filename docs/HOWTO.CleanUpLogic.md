Guide originally published at https://github.com/xbmc/xbmc/pull/13717

This contains the first 7 commits of #12995 unmodified. That PR fixed a bug, but unfortunately it furthered a weak design that should be removed. Still, I realized the cleanup commits would be equally valuable to whomever replaces this design.

I've labelled this PR as a HOW-TO because I explain the process I followed to clean up logic below.

Eventually I'll link to my HOW-TOs from the dev forum. For now I want them to be close to the code so I can incorporate feedback from the review process. If you just want to review the commits, skip this description.

## Why clean logic is important

It's all about readability. We should strive to write code for people, not computers. `if` statements impact the control flow. We want the control flow to be as easy to read and follow as possible.

## What bad `if` and ternary (`? :`) statements look like

Take the [first commit](https://github.com/xbmc/xbmc/commit/2d85c66~6). The previous logic roughly translates to:

```c++
if (A || B || (C &&D) || ((E || (F && G)) && (H && I && J && K)) || L)
  something();
```

Wow.

Ternary logic can also be unwieldy. For example, the previous logic in the second commit translates to:

```c++
X = (A && !B) ? Y : Z;
```

This statement isn't as offensive, but wouldn't it be more readable and maintanable by using a few more lines?

## How to clean up OR's

Generally, OR's are separated by creating a *sequence* of `if`'s on new lines with the same indentation. An initial flag is set, and the `if` statements are used to flip the flag. We can then use a final `if` that only depends on a single flag.

For example:

```c++
if (A || B)
  something()
```

becomes:

```c++
bool flag = false;

if (A)
  flag = true;
else if (B)
  flag = true;

if (flag)
  something();
```

This is demonstrated in the commit:

* [Improve logic for GUI_MSG_QUEUE_NEXT_ITEM in CApplication::OnMessage()](https://github.com/xbmc/xbmc/commit/2d85c66~2)

## How to clean up AND's

Generally, AND's are separated by creating a *series* of `if`'s by embedding indented blocks.

For example:

```c++
if (A && B)
  something()
```

becomes:

```c++
if (A)
{
  if (B)
    something();
}
```

This is demonstrated in the commit:

* [Improve logic in CAutorun::ExecuteAutorun()](https://github.com/xbmc/xbmc/commit/2d85c66~1)

## How to clean up ternary (`? :`) statements

As the name suggests, ternary statements can be split into three lines. For example,

```c++
X = (A && !B) ? Y : Z;
```

becomes:

```c++
X = Y;
if (!A || B)
  X = Z;
```

This is demonstrated in the commit:

* [Improve sleep time logic in CApplication::FrameMove()](https://github.com/xbmc/xbmc/commit/2d85c66~5)

## Extra credit: the `switch` statement

As a bonus, if the logic compares the same value to multiple constants, you can use a `switch` statement for a small performance boost.

For example (notice how I add a tricky OR at the end):

```c++
if (x == A || x == B || x == C || y == D)
  something();
```

becomes:

```
bool flag = false;

switch (x)
{
case A:
case B:
case C:
  flag = true;
  break;
default:
  if (y == D)
    flag = true;
  break;
}

if (flag)
  something();
```

This is demonstrated in the commit:

* [Clean up player notification logic in CApplication::OnAction()](https://github.com/xbmc/xbmc/commit/2d85c66~6)

Switch statements use *jump tables*, which can be faster than `if`s in some cases because branching slows down speculative execution.

## Choosing AND vs. OR

In general, I prefer the flag approach (OR) over the embedding indented blocks approach (AND) because less indentation makes the control flow easier to follow. Boolean algebra can be used to convert AND statements into ORs.

However, when a bunch of ANDs and ORs are combined, sometimes it makes things more complicated to use a single OR operator. It's really a process of trying different algebraic combinations to get the simplest-to-read control flow.

The last commit shows how I was given a convoluted condition and played around with boolean algebra until I eventually settled on what I considered most readable:

* [Improve logic in CGraphicContext::SetFullScreenVideo()](https://github.com/xbmc/xbmc/commit/2d85c66)
