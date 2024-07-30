# Kate Script Tester

`ktexteditor-script-tester6` is a program used to run unit tests on javascript scripts built into `KTextEditor` or written by users.

## Getting Started

First, we need to load a script via `loadScript`.
The script will be evaluated in the context of the global object.

```js
loadScript(":/ktexteditor/script/commands/utils.js");
```

The path above refers to scripts integrated directly into `KTextEditor`.
In particular, it contains the `moveLinesDown` function, which will be used in what follows.

Next, we need to write a test case using one of the 3 available functions:

- `testCase`
- `sequence`
- `withInput`

Each of these functions takes a optional name as first parameter and a function as last parameter.
The name is displayed in case of failure, and cli options allow you to filter what will or won't be executed.
The last parameter function contains our tests on `moveLinesDown`.

```js
testCase("moveLinesDown test", () => {
    //     command           input           expected
    cmd(moveLinesDown, "[1]\n2\n3\n4\n", "2\n[1]\n3\n4\n");
    cmd(moveLinesDown, "2\n[1]\n3\n4\n", "2\n3\n[1]\n4\n");
    cmd(moveLinesDown, "2\n3\n[1]\n4\n", "2\n3\n4\n[1]\n");
});
```

`cmd()` executes a *command* on a document whose state is represented by *input* and checks that the final document is in the state of *expected*.

The document state is the text and the position of cursors and selections represented by characters called *placeholder*. By default, `|` represents a cursor and `[...]` a selection.

If no cursor position is given, it will automatically be set after the selection or at the end of the document.

```js
cmd(moveLinesDown, "[1]\n2\n3\n4\n", "2\n[1]\n3\n4\n");
// equivalent to
cmd(moveLinesDown, "[1]|\n2\n3\n4\n", "2\n[1]|\n3\n4\n");
```

Running the test gives us:

```
Success: 6  Failure: 0  Duration: 10ms
```

The number of successes is double that expected. This is normal, as by default `cmd()` executes tests twice: once with block selection disabled, a second time with block selection enabled.

With a more verbose display (`ktexteditor-script-tester6 -p test.js`) we can see the following result:

```
test.js:4: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=0] Ok
test.js:4: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=1] Ok
test.js:5: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=0] Ok
test.js:5: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=1] Ok
test.js:6: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=0] Ok
test.js:6: moveLinesDown test: cmd `moveLinesDown()` [blockSelection=1] Ok

Success: 6  Failure: 0  Duration: 11ms
```

The `-B` option disables tests on block selection and `--dual` offers finer control.
It can also be disabled directly in the test file via `config()`:

```js
// disable for all tests
config({blockSelection: false});

testCase("moveLinesDown test", () => {
    // disable for current test case
    config({blockSelection: false});
    // ...
});
```

If we look closely at our test, the expected result corresponds to the next test input. The whole thing can be simplified using `sequence()`:

```js
sequence("moveLinesDown test"
    //                 first input
                     , "[1]\n2\n3\n4\n", () => {
    //     command     expected then next input
    cmd(moveLinesDown, "2\n[1]\n3\n4\n");
    cmd(moveLinesDown, "2\n3\n[1]\n4\n");
    cmd(moveLinesDown, "2\n3\n4\n[1]\n");
});
```

By activating verbose mode (`ktexteditor-script-tester6 -B -V test.js`), we can see that the input to a test takes the expected value from the previous test:

```
test.js:5: moveLinesDown test: OK
cmd `moveLinesDown()` [blockSelection=0]:
  input:    [1]|\n2\n3\n4\n
  output:   2\n[1]|\n3\n4\n

test.js:6: moveLinesDown test: OK
cmd `moveLinesDown()` [blockSelection=0]:
  input:    2\n[1]|\n3\n4\n
  output:   2\n3\n[1]|\n4\n

test.js:7: moveLinesDown test: OK
cmd `moveLinesDown()` [blockSelection=0]:
  input:    2\n3\n[1]|\n4\n
  output:   2\n3\n4\n[1]|\n

Success: 3  Failure: 0  Duration: 8ms
```

Note that a cursor is automatically added to the input.

The default display for block selection is different (controlled by `-F`), here's an extract:

```
file1.js:5: moveLinesDown test: OK
cmd `moveLinesDown()` [blockSelection=1]:
  input:    [1]|↵
            2↵
            3↵
            4↵


  output:   2↵
            [1]|↵
            3↵
            4↵

```

Finally, `withInput()` has an identical interface to `sequence()`, but input doesn't change each time `cmd()` is called.

Note that these functions can be called nested and that the script is started in a `testCase` with an empty name.


## Checking return values

There are 2 ways to check the result of a function:

- either `test(command, op, expected, msgOrInfo)`

```js
function foo() { return 42; }
test(foo, '===', 42);
```

- or `cmd(command, input, expectedOutput, msgOrExpectedResultInfo)` (parameter `input` does not exist with `sequence()` and `withInput()`)

```js
cmd(foo, 'input', 'expected output', {
    expected: 42, // if specified, the value will be checked
    op: '===', // is default

    error: true, // alias of {op: 'hasError', expected: true}
    error: "...", // alias of {op: 'errorMsg', expected: "..."}
    error: OtherValue, // alias of {op: 'error', expected: OtherValue}
});
```

All operators are stored in the `TestFramework.operators` array, which a script can complete. It contains:

- classic operators: `===`, `!=`, `<`, etc
- more advanced operators:
    - `eqv`: converts to string then compares. Conversion is done via a dedicated function explained in the next chapter.
    - `!!`: a double negation or, more simply, `Boolean(result) == expected`.
    - `between` for `expected[0] <= result < expected[1]`.
    - `inclusiveBetween` for `expected[0] <= result <= expected[1]`
    - `is` for `Object.is(result, expected)`
- some keywords:
    - `instanceof`
    - `in`
    - `is type` for `typeof result === expected`.
    - `key` for `expected in result`.
- error operators: `error`, `errorMsg`, `errorType`, `hasError`.

`test()` has different aliases:

- test on Boolean:
    - `eqvTrue(command, msgOrInfo)` = `test(command, '!!', true, msgOrInfo)`
    - `eqvFalse(command, msgOrInfo` = `test(command, '!!', false, msgOrInfo)`
    - `eqTrue(command, msgOrInfo` = `test(command, '===', true, msgOrInfo)`
    - `eqFalse(command, msgOrInfo` = `test(command, '===', false, msgOrInfo)`
- test on errors:
    - `error(command, expected, msgOrInfo` = `test(command, 'error', expected, msgOrInfo)`
    - `errorMsg(command, expected, msgOrInfo` = `test(command, 'errorMsg', expected, msgOrInfo)`
    - `errorType(command, expected, msgOrInfo` = `test(command, 'errorType', expected, msgOrInfo)`
    - `hasError(command, msgOrInfo` = `test(command, 'hasError', true, msgOrInfo)`
- alias for operators:
    - `eqv(command, expected, msgOrInfo` = `test(command, 'eqv', expected, msgOrInfo)`
    - `is(command, expected, msgOrInfo` = `test(command, 'is', expected, msgOrInfo)`
    - `eq(command, expected, msgOrInfo` = `test(command, '===', expected, msgOrInfo)`
    - `ne(command, expected, msgOrInfo` = `test(command, '!=', expected, msgOrInfo)`
    - `lt(command, expected, msgOrInfo` = `test(command, '<', expected, msgOrInfo)`
    - `gt(command, expected, msgOrInfo` = `test(command, '>', expected, msgOrInfo)`
    - `le(command, expected, msgOrInfo` = `test(command, '<=', expected, msgOrInfo)`
    - `ge(command, expected, msgOrInfo` = `test(command, '>=', expected, msgOrInfo)`


### Add a string conversion function

String conversions are used for display and with the `eqv` operator.

The conversion function used is `TestFramework.addStringCodeFragments` which displays basic types or use functions registered with `TestFramework.addStringCodeFragments.{registerClassName, registerClass, registerSymbol, register}`.

See sources in `testframework.js` for details.


## Command parameter

the `command` parameter of `cmd()` and `test()` can be either

- a function
- a string to be executed in the global context (not recommended)
- an array with a function or string as the first parameter and arguments for the following parameters

```js
eq(String, ""); // for String()
eq("String()", ""); // equivalent to eval("String()")
eq([String, 2e33], "2e+33"); // for String(2e33)
eq(["String", 2e33], "2e+33"); // equivalent to eval("String(2e33)")
```

Unfortunately, this doesn't work well with methods because the `this` is lost:

```js
const obj = [1,2,3];
eq(obj.join, "[1,2,3]"); // error: Value is undefined
```

To get around this problem, we can use `calleeWrapper()`, which retains the `this` and adds an object name to the function name display:

```js
const obj = calleeWrapper("myArray", [1,2,3]);
eq(obj.join, "1,2,3"); // obj.join() ; display: myArray.join()
eq([obj.join, ", "], "1, 2, 3"); // obj.join(", ") ; display: myArray.join(", ")
```

If the first parameter of `calleeWrapper` is falsy, then a string representation of the object will be used.

```js
const obj = calleeWrapper("", [1,2,3]);
eq(obj.join, "1,2,3"); // display: [1, 2, 3].join()
eq([obj.join, ", "], "1, 2, 3"); // display: [1, 2, 3].join(", ")
```

Note that `calleeWrapper` is not required with `view`, `document` and `editor`.

For lazy evaluation of function calls as parameters, we can use `lazyfn(fn, ...args)` or `fn(fn, ...args)`:

```js
function getSep() { return "|"; }
eq([obj.join, fn(getSep)], "1|2|3"); // obj.join("|") ; display: myArray.join(getSep())
```

To use a lazy function call in an array or object, we can use `lazyarg(Array|Object)` or `arg(Array|Object)`:

```js
function foo(a, b) { return `{${a};${b}}`; }
eq([foo, arg([fn(foo, "0", 1), 2]), 3], "{{0;1},2;3}"); // foo([foo("0", 1), 2], 3)
```


## Placeholders

Placeholders are used with `cmd()` to insert cursors or selections in the input and expected output. They are configured via `config()`.

There are a total of 7 placeholders distributed in 5 configuration variables:

- `cursor`: represents the primary cursor. The default is `'|'`.
- `selection`: represents the start and end of a selection. Default is `'[]'`.
- `virtualText`: used with block selection to introduce spaces to place a cursor outside the document text. It must always be followed by a placeholder or new line.
- `secondaryCursor`: secondary cursor in multi-cursor. No default value.
- `secondarySelection`: secondary selection in multi-cursor. No default value.

If the placeholder is an empty string, it will not be used.

If `cursor` and `secondaryCursor` have the same value, then the first cursor found will be the primary cursor. The same applies to selection.

In some cases, a placeholder may not be configured, but used in the result display. This is configured via the `-S` command line option or with the following 5 configuration variables:

- `cursor2`: the default is `'|'`
- `selection2`: the default is `'[]'`
- `virtualText2`: the default is `'·'`
- `secondaryCursor2`: the default is `'┆'`
- `secondarySelection2`: the default is `'❲❳'`

These variables are also used when a placeholder has a display conflict with another placeholder.

Unlike the first 5, setting an empty value reset to default value.


## Multi-line string parameter

When parameters contain several line breaks, the text may be difficult to read. For example:

```js
type('{', 'class X\n{\n  void foo()\n  {|}\n};\n'
        , 'class X\n{\n  void foo()\n  {\n    |\n  }\n};\n');
});
```

This can be rewritten with concatenations in this form:

```js
type('{', 'class X\n'
        + '{\n'
        + '  void foo()\n'
        + '  {|}\n'
        + '};'

        , 'class X\n'
        + '{\n'
        + '  void foo()\n'
        + '  {\n'
        + '    |\n'
        + '  }\n'
        + '};');
});
```

But the use of `'\n` makes reading a little confusing. To overcome this problem, a tagged template named `c` exists:

```js
type('{', c`> class X
            > {
            >   void foo()
           ->   {|}
            > }`

        , c`> class X
            > {
 the text   >   void foo()
 here is   ->   {
 ignored   ->     |
           ->   }
            > }`);
});
```

Characters at the beginning of a line up to `> ` will be ignored.


## Constants

There are 5 special constants:

- `DUAL_MODE`: used with the `blockSelection` configuration variable so that `cmd()` runs the test first with block selection disabled, then, if the test does not fail, with block selection enabled.
- `ALWAYS_DUAL_MODE`: Same as `DUAL_MODE`, but the 2 modes are always executed.
- `AS_INPUT`: Special value for `expectedOutput` from `cmd()` which indicates that the expected output is the same as the input value.
- `REUSE_LAST_INPUT`: in parameter of `sequence()` and `withInput()`, reuse the previous input.
- `REUSE_LAST_EXPECTED_OUTPUT`: in parameter of `sequence()` and `withInput()`, use the previous expected output as input.


## Interface

- `loadScript(path)`: load a javascript file in the context of the global object.
- `loadModule(path)`: load a javascript file as a module.
- `paste(str)`: paste `str` to the current position.

- `kdb.type(str)`: insert `str` into current documment. To be used as a command: `cmd([kbd.type, str], ...)`
- `kdb.enter()`: insert a new line into current documment. To be used as a command: `cmd(enter, ...)`
- `keys(str) -> kdb.enter | [kbd.type, str]`: To be used as a command: `cmd(keys(str), ...)`

For full documentation, see the functions and classes exported from `testframework.js` (those prefixed with `export`).

- `TestFramework.operators`
- `TestFramework.formatArgs(args, sep)`
- `TestFramework.toComparableString(value)`
- `TestFramework.functionCallToString(fn, args)`
- `TestFramework.addStringCodeFragments(value, codeFragments, ancestors, mustBeStable)`
- `TestFramework.addStringCodeFragments.registerClassName(name, addString)`
- `TestFramework.addStringCodeFragments.registerClass(type, addString)`
- `TestFramework.addStringCodeFragments.registerSymbol(symbol, addString)`
- `TestFramework.addStringCodeFragments.register(addString)`

Unless otherwise indicated, these following functions are also found in `TestFramework` under the same name:

- `DUAL_MODE`
- `ALWAYS_DUAL_MODE`
- `AS_INPUT` = `TestFramework.EXPECTED_OUTPUT_AS_INPUT`
- `REUSE_LAST_INPUT`
- `REUSE_LAST_EXPECTED_OUTPUT`
- `calleeWrapper(name, obj, newObj)`
- `LazyFunc(fn, ...args)` ; alias: `fn`, `lazyfn`
- `LazyArg(Array|Object)` ; alias: `arg`, `lazyarg`
- `c(strings, ...values)` (= `TestFramework.sanitizeTag`): tagged templates prefix sanitizer

- `config(object)`
- `testCase(nameOrConfig: Optional[String|Object], fn)`: name is used for filter test. If it is an object, it is passed to `config()`. An object can contain the `name` property.
- `sequence(nameOrConfig: Optional[String|Object], input, fn)`
- `withInput(nameOrConfig: Optional[String|Object], input, fn)`
- `print(...args)`: equivalent to `printSep(' ', ...args)`
- `printSep(sep, ...args)`

- `cmd(command, input, expectedOutput: String | AS_INPUT, msgOrExpectedResultInfo: undefined | String | Object)`: inside `sequence()` and `withInput()`, the `input` parameter does not exist
- `xcmd(...)`: Same as `cmd()`, but for an expected failure.
- `type(str, input, expectedOutput, msgOrExpectedResultInfo)`: alias for `cmd(keys(str), input, expectedOutput, msgOrExpectedResultInfo)`.
- `xtype(...)`: Same as `type()`, but for an expected failure.
- `indentFiles(dataDir)`: execute indentation on `${dataDir}/*/origin`, compare with `${dataDir}/*/expected` and write failure results in `${dataDir}/*/actual`. If the `${dataDir}/.kateconfig` file exists, it will be used for each test. Config variables (`kate: ...`) can also be specified at the beginning and end of each test file.

- `test(command, op: String, expected, msgOrInfo)`
- `xtest(...)`: Same as `test()`, but for an expected failure.
- `eqvTrue(command, msgOrInfo)`
- `eqvFalse(command, msgOrInfo)`
- `eqTrue(command, msgOrInfo)`
- `eqFalse(command, msgOrInfo)`
- `error(command, expected, msgOrInfo)`
- `errorMsg(command, expected, msgOrInfo)`
- `errorType(command, expected, msgOrInfo)`
- `hasError(command, msgOrInfo)`
- `eqv(command, expected, msgOrInfo)`
- `is(command, expected, msgOrInfo)`
- `eq(command, expected, msgOrInfo)`
- `ne(command, expected, msgOrInfo)`
- `lt(command, expected, msgOrInfo)`
- `gt(command, expected, msgOrInfo)`
- `le(command, expected, msgOrInfo)`
- `ge(command, expected, msgOrInfo)`
