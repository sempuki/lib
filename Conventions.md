# Conventions

These conventions hold in every project that uses lib. A project's own
`Design.md` adds the rules that only it needs. lib's `.clang-format` and
`.clang-tidy`, which projects symlink, check what a tool can.

## Arguments

**Reference arguments are marked at the call site** with lib's wrappers.

| Wrapper | The callee | Call site |
|---|---|---|
| `Out<T>` | writes the argument, and may ignore it | `parse(text, Out(result))` |
| `InOut<T>` | reads and writes the argument | `scheduler.step(step, InOut(world))` |
| `Depend<T>` | keeps a reference that can dangle | `Driver{timing, Depend(simulation)}` |

Call sites write `Out(x)`, `InOut(x)` or `Depend(x)` and let class template
argument deduction find the type. Only parameters name it.

**The wrappers are written without `lib::`.** Each project declares them in
its own namespace with `using lib::Out;` and the rest, in one header (simon's
`core/argument.hpp`, iopia's `iopia/argument.hpp`).

**Parameters read inputs first, then what the callee writes or keeps, then what
it only writes.** The order is `(In..., InOut..., Out)`, with `Depend` among the
`InOut`s: `integrate(acceleration, dt, InOut(kinematics))`.

A plain `T&` parameter is only for what the language decides, such as
operators. `const T&` needs no wrapper.

**A class that takes `Depend<T>` stores a plain pointer,** so its hot path
doesn't pay for `CheckedPointer`'s null check on every access.

**Return a value or write an `Out`, whichever suits the type.** An `Out`
parameter fills an object the caller already owns, so it suits types that are
large or never move.

## Errors

**Errors are `std::expected<T, lib::Status>`, propagated with lib's macros**
in place of an `if` per call.

```cpp
RETURN_IF_UNEXPECTED(build_world(scenario, Out(world)));
RETURN_OR_ASSIGN_TO(asset, build_scenario(scenario, InOut(world)));
```

`RETURN_IF_UNEXPECTED` returns the error, if any, from the enclosing function.
`RETURN_OR_ASSIGN_TO` does the same and otherwise moves the value into an
existing variable or a new declaration (`Entity asset`). The enclosing function
must return a `std::expected` whose error type the error converts to.
`RETURN_OR_ASSIGN_TO` expands to several statements, so it goes only where a
statement can, at most once per line.

**A project's errors form a status domain,** an `enum class` of conditions
with their messages in a `lib::EnumStatusKindConditionMixin` specialization.
Compare a status with a condition through `lib::watch(Condition::X)`.

**A type that can fail to construct has a factory.** A constructor can't
return an error, so `Type::make(...)` returns `std::expected<Type,
lib::Status>`.

**Contract checks use `CHECK_PRECONDITION`, `CHECK_POSTCONDITION`,
`CHECK_INVARIANT` and `CHECK_UNREACHABLE`.**

## Functions and names

**Functions declare a trailing return type,** `auto f(X) -> Y`, `void` ones
included. Constructors, destructors and conversion operators can't, and
lambdas keep their deduced returns. `.clang-tidy` checks it with
`modernize-use-trailing-return-type`, which skips `void` functions.

**Names avoid abbreviations,** except well-established jargon and names that
come from a C API. `submission`, not `sqe`, and `NoOp`, not `Nop`.

**Accessors are named for what they return, never `get`:** `component_of`,
`store_of`, `name_of`. Where arguments of different strong types ask different
questions, one name is overloaded: `find_name_of(Identity)` and
`find_name_of(Alias)`.

**A getter that may return null is named `maybe_*`.**
`maybe_component_of<Health>(entity)` returns a pointer, null if the entity has
no `Health`. Its non-null counterpart drops the prefix, `component_of`, and
returns a reference.

**A function that does work and may fail is named `try_<verb>`.** The verb
says it does work, and `try_` says it can come back empty-handed:
`try_take_submission` claims a free slot and returns null when none is free.

**Template type parameters end with `Type`** (or `Types` for a pack):
`ComponentType`, `ComponentTypes...`. Concepts are named for the property they
check, as adjectives where they read well: `Spatial`, `Archetypal`.

**Types that are lists of types end with `List`:** `TypeList<...>`,
`SystemList<...>`. Metafunctions that produce one say so:
`write_list_of_t<SystemType>`.

**Unused names say so without comments.** Leave a parameter unnamed when its
type says what it is. Name it with `[[maybe_unused]]` when the name carries
meaning the type doesn't, or when only some instantiations use it. Never write
`/*name*/`. Bind a result kept only to be ignored to C++26's `_`, which may be
declared again in the same scope: `auto _ = world.destroy(self).build();`.

## Types and classes

**Integers have fixed widths.** Write `std::uint32_t`, `std::int64_t` and
their kin where C would write `unsigned` or `long`. Sizes and indices into
containers stay `std::size_t`. A value a C call returns keeps its C type only
until the code converts it.

**A class has one `public:` section, then one `protected:`, then one
`private:`.** Compile-time checks and aliases may come first, before
`public:`, unlabeled.

**A class's copy and move members sit together,** the `DECLARE_COPY_*` and
`DECLARE_MOVE_*` macros or the hand-written constructors and assignments, in
one block with no blank line between them.

**Every pointer is initialized,** `Type* pointer = nullptr;`, even when a
constructor always sets it.

**Implementation details live in a namespace named `internal`.**

## Files

**Headers hold what must be inline; `.cpp` files hold the rest.** Templates,
and hot-path code the inliner must see, stay in headers. Setup, string
formatting and code that runs rarely go in `.cpp` files.

**Headers are included from the repository root,** as in `#include
"iopia/ring.hpp"`.

**Documents live beside their code.** A package's `Design.md`, any
`Design.<topic>.md`, and its `Using.md` sit in the package's directory.
`Design.md` records the architecture, the decisions and their reasons.
`Using.md` shows how to use the package.

## C APIs

**C symbols are written with the global namespace qualifier,**
`::io_uring_get_sqe(...)` and `::io_uring_sqe*`, so a reader can tell a C API
from the project's own names. Macros such as `EINVAL` can't be qualified and
stay as they are. A function-like macro gets a comment saying it is a macro.

## Comments and prose

**A comment states the fact and stops.** It leaves out consequences a reader
can infer from the fact: `// A liburing macro.`

**Documents are written plainly, the way a knowledgeable person would talk.**
Short, direct sentences lead with the point. Four constructions are out.

| Avoid | Write |
|---|---|
| A cleft: "What makes this fast is the cache." | "The cache makes this fast." |
| "Not X, but Y": "This isn't a bug, it's a design decision." | "This is a design decision." |
| A colon that withholds the point: "There's one catch: the index is rebuilt on every write." | "The index is rebuilt on every write, which is the catch." |
| An em-dash aside: "The parser — which nobody has touched — still works." | "The parser still works, though nobody has touched it in years." |

## Tests

**Tests are Catch2, through `@lib//base:testing`.** One `TEST_CASE` per type,
`SECTION("ShouldXGivenY")` inside it, and each section split into
`// Preconditions.`, `// Under Test.` and `// Postconditions.`
