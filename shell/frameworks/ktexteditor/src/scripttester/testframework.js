/*
    SPDX-FileCopyrightText: 2024 Jonathan Poelen <jonathan.poelen@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/// Value used in blockSelection configuration to run the test first with block
/// selection disabled, then, if the test does not fail, with block selection enabled.
export const DUAL_MODE = 2;

/// Same as DUAL_MODE, but the 2 modes are always executed.
export const ALWAYS_DUAL_MODE = 3;

/// Special value for \c expectedOutput from \c cmd() which indicates that
/// the expected output is the same as the input value.
export const EXPECTED_OUTPUT_AS_INPUT = Symbol('EXPECTED_OUTPUT_AS_INPUT');

/// with \c TestCaseSequence.chain() and \c TestCaseSequence.withInput(),
/// reuse the previous input.
export const REUSE_LAST_INPUT = Symbol('REUSE_LAST_INPUT');

/// with \c TestCaseSequence.chain() and \c TestCaseSequence.withInput(),
/// use the previous expected output as input.
export const REUSE_LAST_EXPECTED_OUTPUT = Symbol('REUSE_LAST_EXPECTED_OUTPUT');


let clipboardHistory = [];

function setClipboardHistory(newHistory) {
    clipboardHistory = newHistory.slice(0, 10);
}

// editor object of KTextEditor
export const editor = {
    clipboardText: function() {
        return clipboardHistory[0] || '';
    },

    clipboardHistory: function() {
        return clipboardHistory;
    },

    setClipboardText: function(text) {
        if (clipboardHistory.length === 10) {
            clipboardHistory.shift();
        }
        clipboardHistory.unshift(text);
    },
};

export class LazyBase {};

/**
 * Lazy evaluation for \c cmd() function parameters.
 */
export class LazyFunc extends LazyBase {
    constructor(fn, ...args) {
        super();
        this.fn = fn;
        this.args = args;
    }

    toString() {
        return functionCallToString(this.fn, this.args);
    }
}

/**
 * Lazy evaluation for function parameter with \c cmd().
 */
export class LazyArg extends LazyBase {
    constructor(arg) {
        super();
        this.arg = arg;
    }

    toString() {
        return toComparableString(this.arg);
    }
}

function evaluateArg(value) {
    if (!(value instanceof LazyBase)) {
        return value;
    }

    if (value instanceof LazyFunc) {
        return executeFunc(value.fn, value.args);
    }

    value = value.arg;

    if (Array.isArray(value)) {
        const args = [];
        for (const arg of value) {
            args.push(evaluateArg(arg))
        }
        return args;
    }

    const o = {};
    for (const k in value) {
        o[k] = evaluateArg(value[k]);
    }
    return o;
}

function executeFunc(fn, args) {
    const evaluatedArgs = [];
    for (const arg of args) {
        evaluatedArgs.push(evaluateArg(arg));
    }
    return fn.apply(fn, evaluatedArgs);
}

/**
 * Convert a value to a String
 * @param value Any: value to convert
 * @param codeFragments[out] Array[String]: array containing the strings representing the value
 * @param ancestors used for cyclic reference detection
 * @param mustBeStable Boolean: indicates whether the result should be stable.
 *                              For example, by sorting object keys.
 * @return undefined
 *
 * This function also has 4 members for storing conversion functions:
 *  - \c .registerClassName(name, addString) when `value.constructor.name == name`
 *  - \c .registerClass(type, addString) when `value instanceof type`
 *  - \c .registerSymbol(symbol, addString) when `value[symbol]`
 *  - \c .register(addString) when `addString(...) == true`
 * The parameters of \c addString are the same as those of \c addStringCodeFragments.
 */
export const addStringCodeFragments = (function(){
    const replacement = function(c) {
        return c === '\n' ? '\\n'
             : c === '\t' ? '\\t'
             : '\\' + c
    };

    // format a string as a literal string
    const pushJSString = function(codeFragments, str) {
        // when no quote or single and double quote, prefer single quote
        if (str.indexOf("'") === -1 || str.indexOf('"') !== -1) {
            codeFragments.push("'");
            codeFragments.push(str.replace(/[\n\t\'\\]/g, replacement));
            codeFragments.push("'");
        }
        else {
            codeFragments.push('"');
            codeFragments.push(str.replace(/[\n\t\"\\]/g, replacement));
            codeFragments.push('"');
        }
    };

    const addStringCodeFragmentByName = {};
    const addStringCodeFragmentByBehavior = [];

    const addStringCodeFragments = function(value, codeFragments, ancestors, mustBeStable) {
        const t = typeof value;
        // String
        if (t === 'string')
            pushJSString(codeFragments, value);
        // undefined
        else if (value === undefined)
            codeFragments.push('undefined');
        // null
        else if (value === null)
            codeFragments.push('null');
        // Function
        else if (t === 'function')
            // .toString() is always `function funcname() { [native code] }`.
            // Ignore body function and add number of parameter
            codeFragments.push(value.name
                ? `function ${value.name}(${value.length} params)`
                : `function(${value.length} params)`
            );
        // Number, Boolean, Symbol, BigInt (not implemented in QJSEngine)
        else if (t === 'symbol')
            // force toString() so as not to have an error with Symbol type
            codeFragments.push(value.toString());
        else if (t !== 'object')
            codeFragments.push(value);
        // circular call
        else if (ancestors.includes(value))
            codeFragments.push("[Circular]");
        // LazyFunc / LazyBase
        else if (value instanceof LazyBase) {
            if (value instanceof LazyArg)
                addStringCodeFragments(value.arg, codeFragments, ancestors, mustBeStable);
            else
                addFunctionCallExprFragments(value.fn, value.args,
                                             codeFragments, ancestors, mustBeStable);
        }
        else {
            // convert object if registered
            const name = value.constructor?.name;
            const addString = addStringCodeFragmentByName[name];
            if (addString) {
                addString(value, codeFragments, ancestors, mustBeStable);
                return;
            }
            for (const addString of addStringCodeFragmentByBehavior) {
                if (addString(value, codeFragments, ancestors, mustBeStable)) {
                    return;
                }
            }

            // assume array or array like
            if (value[Symbol.iterator]) {
                ancestors.push(value);

                const isArray = Array.isArray(value);
                if (isArray)
                    codeFragments.push('[');
                else {
                    const name = value.constructor.name;
                    codeFragments.push('new ');
                    codeFragments.push(value.constructor.name);
                    codeFragments.push('([');
                }

                const codeFragments2 = [];
                const codeFragmentsLen = codeFragments.length;
                for (const o of value) {
                    const subCodeFragment = [];
                    addStringCodeFragments(o, subCodeFragment, ancestors, mustBeStable);
                    codeFragments2.push(subCodeFragment.join(''));
                }
                if (mustBeStable && !isArray) {
                    codeFragments2.sort();
                }
                codeFragments.push(codeFragments2.join(', '));

                codeFragments.push(isArray ? ']' : '])');

                ancestors.pop();
            }
            // Error
            else if (value instanceof Error) {
                codeFragments.push(`${value.name}: ${value.message}`);
            }
            // RegExp
            else if (value instanceof RegExp) {
                codeFragments.push(value);
            }
            // other Object
            else {
                ancestors.push(value);

                codeFragments.push('{');
                const codeFragmentsLen = codeFragments.length;
                const keys = Object.keys(value);
                for (const key of mustBeStable ? keys.sort() : keys) {
                    if (/^[a-zA-Z$][\w\d$]*/.test(key))
                        codeFragments.push(key);
                    else
                        pushJSString(codeFragments, key);
                    codeFragments.push(': ');
                    addStringCodeFragments(value[key], codeFragments, ancestors, mustBeStable);
                    codeFragments.push(', ');
                }
                // remove last ', '
                if (codeFragmentsLen !== codeFragments.length)
                    codeFragments.pop();
                codeFragments.push('}');

                ancestors.pop();
            }
        }
    };

    // Register function for unknown object
    // @{
    addStringCodeFragments.registerClassName = function(name, addString) {
        addStringCodeFragmentByName[name] = addString;
    };

    addStringCodeFragments.registerClass = function(type, addString) {
        addStringCodeFragmentByBehavior.push((value, codeFragments, ancestors, mustBeStable) => {
            if (value instanceof type) {
                addString(value, codeFragments, ancestors, mustBeStable);
                return true;
            }
            return false;
        });
    };

    addStringCodeFragments.registerSymbol = function(symbol, addString) {
        addStringCodeFragmentByBehavior.push((value, codeFragments, ancestors, mustBeStable) => {
            if (value[symbol]) {
                addString(value, codeFragments, ancestors, mustBeStable);
                return true;
            }
            return false;
        });
    };

    addStringCodeFragments.register = function(addString) {
        addStringCodeFragmentByBehavior.push(addString);
    };
    // @}

    return addStringCodeFragments;
})();

/**
 * Uses \c addStringCodeFragments() on all parameters and returns them
 * separated with \p sep.
 * @param args Iterator
 * @param sep String
 * @return String
 */
export function formatArgs(args, sep) {
    const codeFragments = [];
    for (const arg of args) {
        if (typeof arg === 'string') {
            codeFragments.push(arg);
        }
        else {
            addStringCodeFragments(arg, codeFragments, []);
        }
        codeFragments.push(sep);
    }
    codeFragments.pop();
    return codeFragments.join('');
}

/**
 * Convert a value to a String.
 * @param value Any: value to convert
 * @return String
 */
export function toComparableString(value) {
    const codeFragments = [];
    addStringCodeFragments(value, codeFragments, [], true);
    return codeFragments.join('');
}

function addFunctionCallExprFragments(fn, args, codeFragments, ancestors, mustBeStable) {
    // the Fonction object has a name, but may be empty
    const name = (typeof fn === 'function') ? (fn.name || '/*Unamed Function*/')
        : (fn === undefined) ? 'undefined'
        : (fn === null) ? 'null'
        : fn.toString();
    codeFragments.push(name);

    if (args.length === 0) {
        codeFragments.push('()');
    } else {
        codeFragments.push('(');
        for (const arg of args) {
            addStringCodeFragments(arg, codeFragments, ancestors, mustBeStable);
            codeFragments.push(', ');
        }
        codeFragments[codeFragments.length - 1] = ')';
    }
}

/**
 * Convert a function and its arguments to a String.
 * @param fn Function
 * @param args Array
 * @return String
 */
export function functionCallToString(fn, args) {
    const codeFragments = [];
    addFunctionCallExprFragments(fn, args, codeFragments, []);
    return codeFragments.join('');
}

// As `===` but with `NaN === NaN` evaluated to true
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Equality_comparisons_and_sameness#comparing_equality_methods
function sameValueZero(x, y) {
  if (typeof x === "number" && typeof y === "number") {
    // x and y are equal (may be -0 and 0) or they are both NaN
    return x === y || (x !== x && y !== y);
  }
  return x === y;
}

/**
 * Operators available for \c test.
 * The key corresponds to the name used for the variable \c op of \c test.
 * The value is a comparison function of the form:
 * (expected: Any) -> {
 *          \c error: Boolean: indicates that the operator is expecting an exception.
 *          \c opname: name of the operator displayed, otherwise the key name is used
 *          \c expr: represents the test expression. If a sub-string '{input}' exists,
 *                   it will be replaced by the command.
 *          \c comp: (result: Any, expected: Any, exception: Any) -> Boolean | {
 *              \c ok: Boolean: indicates whether the comparison is a success or a failure
 *              \c result: represents the result of the command that will be displayed
 *              \c expected: represents the expected result that will be displayed
 *          }
 *      }
 *      All properties expect \c comp are optional
 */
export const operators = {
    /// Comparison on values converted to String.
    eqv: (_) => ({
        comp: (result, expected, exception) => {
            result = toComparableString(result);
            expected = toComparableString(expected);
            return {result, expected, ok: !exception && result === expected};
        },
    }),

    /// Double not ; equivalent to `Boolean(expr)`.
    '!!': (expected) => expected
        ? {
            expr: '!!({input})',
            comp: (result, expected, exception) => !exception && !!result,
        } : {
            expr: '!({input})',
            comp: (result, expected, exception) => !exception && !result,
        },

    /// Javascript operators with `===` and `!==` treating \c NaN as equivalent.
    /// (https://developer.mozilla.org/en-US/docs/Web/JavaScript/Equality_comparisons_and_sameness#same-value-zero_equality)
    /// @{
    '===': (_) => ({
        comp: (result, expected, exception) => !exception && sameValueZero(result, expected)
    }),
    '==': (_) => ({
        comp: (result, expected, exception) => !exception && result == expected
    }),
    '!==': (_) => ({
        comp: (result, expected, exception) => !exception && !sameValueZero(result, expected)
    }),
    '!=': (_) => ({
        comp: (result, expected, exception) => !exception && result != expected
    }),
    '<': (_) => ({
        comp: (result, expected, exception) => !exception && result < expected
    }),
    '>': (_) => ({
        comp: (result, expected, exception) => !exception && result > expected
    }),
    '<=': (_) => ({
        comp: (result, expected, exception) => !exception && result <= expected
    }),
    '>=': (_) => ({
        comp: (result, expected, exception) => !exception && result >= expected
    }),

    'instanceof': (_) => ({
        comp: (result, expected, exception) => !exception && result instanceof expected
    }),
    'in': (_) => ({
        comp: (result, expected, exception) => {
            try {
                return !exception && result in expected;
            }
            catch (_) {
                return false;
            }
        },
    }),
    /// @}

    /// typeof operator.
    'is type': (_) => ({
        comp: (result, expected, exception) => !exception && typeof result === expected
    }),

    /// Object.is
    'is': (_) => ({
        expr: 'Object.is({input}, {expected})',
        comp: (result, expected, exception) => !exception && Object.is(result, expected),
    }),

    /// Check that the \c exception matches \c expected.
    /// If \p exception is an \c Error type, then \p result must be an \c Error
    /// type with the identical \c name and \c message properties. Otherwise \c === is used.
    error: (_) => ({
        error: true,
        expr: 'error of {input} === {expected}',
        comp: (result, expected, exception) => {
            const isError = expected instanceof Error;
            return exception && isError === (exception instanceof Error)
                && (isError
                    ? exception.name === expected.name && exception.message === expected.message
                    : exception === expected)
        },
    }),
    /// Check that the \c exception message matches \c expected.
    /// If \c exception is an \c Error type, \c exception.message is used.
    /// Otherwise `exception.toString()` used.
    errorMsg: (_) => ({
        error: true,
        expr: 'error message of {input} === {expected}',
        comp: (result, expected, exception) =>
            exception && (exception instanceof Error
                ? exception.message === expected
                : exception.toString() === expected),
    }),
    /// Check that the \c exception type matches \c expected.
    /// If \c exception is an \c Error type, \c exception.name is used.
    /// Otherwise `typeof exception` is used.
    errorType: (_) => ({
        error: true,
        expr: 'error type of {input} === {expected}',
        comp: (result, expected, exception) =>
            exception && (exception instanceof Error
                ? exception.name === expected
                : typeof exception === expected)
    }),
    /// Check that an exception is thrown.
    hasError: (expected) => ({
        error: expected,
        expr: expected ? 'check error for {input}' : 'check not error for {input}',
        comp: (result, expected, exception) => ({
            expected: `has error = ${expected}`,
            ok: !exception === !expected,
        }),
    }),

    /// Equivalent to `expected[0] <= result && result < expected[1]`.
    between: (_) => ({
        expr: '{expected[0]} <= {input} < {expected[1]}',
        comp: (result, expected, exception) =>
            !exception && expected[0] <= result && result < expected[1]
    }),

    /// Equivalent to `expected[0] <= result && result <= expected[1]`.
    inclusiveBetween: (_) => ({
        expr: '{expected[0]} <= {input} <= {expected[1]}',
        comp: (result, expected, exception) =>
            !exception && expected[0] <= result && result <= expected[1]
    }),

    /// Equivalent to `expected in result`.
    key: (_) => ({
        expr: '{expected} in {input}',
        comp: (result, expected, exception) => {
            try {
                return !exception && expected in result;
            } catch (_) {
                return false;
            }
        },
    }),
};

export const BREAK_CASE_ERROR = Symbol('BREAK_CASE_ERROR');
export const STOP_CASE_ERROR = Symbol('STOP_CASE_ERROR');

let internal;
let env;
let ctx;
let contexts = [];

/**
 * @param op Function | String: key of \c operators or \c Function with
 *                              optional \p opname property. See the description
 *                              of \c comp of \c operators for the function signature.
 * @return Object: see \c operators
 */
function getComparatorInfo(op, expected) {
    if (typeof op === 'function') {
        return {comp: op, opname: op.name};
    }

    const mkcomp = operators[op];
    if (!mkcomp) {
        internal.incrementError();
        const names = Object.keys(operators).join(', ');
        throw new ReferenceError(`Unknown operator '${op}', expected: ${names} or Function`);
    }
    return mkcomp(expected);
}

/**
 * transforms \p command into 2 functions: the first to execute the program,
 * the second to retrieve it as a string.
 * @param command Function | String | Array[Function | String, ...arguments] | LazyFunc
 * @return Array[program: () -> Any, programToString: () -> String]
 */
function toProgram(command) {
    if (typeof command === 'function') {
        return [
            command,
            () => command.name ? command.name + '()' : '/*Unamed Function*/',
        ];
    }

    if (typeof command === 'string') {
        return [
            () => internal.evaluate(command),
            () => command,
        ];
    }

    if (Array.isArray(command)) {
        const args = command.slice(1);
        let fn = command[0];

        /*
         * Transform string to function.
         * Useful with the form `obj.func` which requires that the parameter
         * be `obj.func.bind(obj)` or `(...arg) => obj.func(...arg)` so as not
         * to lose test which makes it impossible to display the function used
         * in the test message.
         */
        if (typeof fn === 'string') {
            fn = internal.evaluate(`(function(){ return ${fn}(...arguments); })`)
        }
        else if (fn instanceof LazyFunc) {
            return [
                () => executeFunc(command.fn, executeFunc.args)(args),
                () => functionCallToString(fn, args),
            ];
        }

        for (const arg of args) {
            if (arg instanceof LazyBase) {
                return [
                    () => executeFunc(fn, args),
                    () => functionCallToString(command[0], args),
                ];
            }
        }

        return [
            () => fn.apply(undefined, args),
            () => functionCallToString(command[0], args),
        ];
    }

    if (command instanceof LazyFunc) {
        return [
            () => executeFunc(command.fn, command.args),
            () => functionCallToString(command.fn, command.args),
        ];
    }

    internal.incrementError();
    throw new TypeError("Invalid command type. Expected Function, String, Array or LazyFunc");
}

/**
 * Execute \p program.
 * @param program Function() -> Any
 * @return Array[result: Any, exception: Any]
 */
function executeProgram(program) {
    const savedClipboardHistory = clipboardHistory.slice();
    let r;
    try {
        r = [program(), undefined];
    } catch (e) {
        r = [undefined, e];
    }
    clipboardHistory = savedClipboardHistory;
    return r;
}

/**
 * Return the program expression
 * @param info undefined | {
 *          \c expr: Optional[String]
 *          \c program: Optional[String]
 *      }
 * @param compInfo undefined | {
 *          \c expr: Optional[String]
 *          \c opname: Optional[String]
 *      }
 * @param opname String
 * @param getProgram () -> String
 * @param useExpectedResultAsPlaceholder Boolean: replace '{expected}' with '{expectedResult}'
 * @return String
 */
function getInputExpression(info, compInfo, opname, getProgram, useExpectedResultAsPlaceholder) {
    const program = info?.program || getProgram();
    let expr = info?.expr || compInfo?.expr;
    if (expr) {
        if (useExpectedResultAsPlaceholder) {
            expr = expr.replace('{expected}', '{expectedResult}')
        }
        return expr.replace('{input}', program);
    }

    const placeholder = useExpectedResultAsPlaceholder ? 'expectedResult' : 'expected';
    return `${program} ${compInfo?.opname || opname} {${placeholder}}`;
}

/**
 * @param msgOrInfo String | {\c msg: String}
 * @return String
 */
function getMessage(msgOrInfo) {
    return typeof msgOrInfo === 'string' ? msgOrInfo : msgOrInfo?.msg;
}

const OUTPUT_IS_OK = (1 << 0);
const CONTAINS_RESULT_OR_ERROR = 1 << 1;
const EXPECTED_ERROR_BUT_NO_ERROR = (1 << 2) | CONTAINS_RESULT_OR_ERROR;
const EXPECTED_NO_ERROR_BUT_ERROR = (1 << 3) | CONTAINS_RESULT_OR_ERROR;
const IS_RESULT_NOT_ERROR = (1 << 4) | CONTAINS_RESULT_OR_ERROR;
const IS_ERROR_NOT_RESULT = CONTAINS_RESULT_OR_ERROR;
const SAME_RESULT_OR_ERROR = (1 << 5) | CONTAINS_RESULT_OR_ERROR;
const IGNORE_INPUT_OUTPUT = (1 << 6);
const RESULT_OK = (1 << 5);

/**
 * Return the program expression
 * @param cmpRes undefined | {expected: Optional[String], result: Optional[String], error: Boolean}:
 * @param result Any
 * @param expected Any
 * @param exception Any
 * @return {
 *      \c displableExpected: String
 *      \c displayableResult: String
 *      \c opt: Number
 * }
 */
function comparatorResultToPrintableInfo(cmpRes, result, expected, exception) {
    return {
        displableExpected: cmpRes?.expected || toComparableString(expected),
        displayableResult: cmpRes?.result || exception || toComparableString(result),
        opt: cmpRes?.error
            ? (exception ? IS_ERROR_NOT_RESULT : EXPECTED_ERROR_BUT_NO_ERROR)
            : (exception ? EXPECTED_NO_ERROR_BUT_ERROR : IS_RESULT_NOT_ERROR),
    };
}

/**
 * Check \p command result and display the result if it fails.
 * @param name String: test name
 * @param program value for \c executeProgram()
 * @param getProgram Function() -> String
 * @param compInfo undefined | Object: see operator \c getComparatorInfo()
 * @param op undefined | String: value for \c getInputExpression()
 * @param expectedResult: Any
 * @param msgOrInfo value for \c getInputExpression() and \c getMessage()
 * @param xcheck Boolean: true for an expected failure
 * @return Boolean: \c true when no errors, otherwise \c false
 */
function checkCommand(name, program, getProgram, compInfo, op, expectedResult, msgOrInfo, xcheck) {
    const displayMode = internal.startTest();
    if (displayMode & 2) {
        internal.writeTestExpression(
            name, xcheck ? 'xcmd' : 'cmd', 4, compInfo
                ? getInputExpression(msgOrInfo, compInfo, op, getProgram, true)
                : (msgOrInfo?.program || getProgram())
        );
    }

    const [result, exception] = executeProgram(program);
    const isSame = internal.checkOutput();
    const compRes = compInfo
        ? compInfo.comp(result, expectedResult, exception)
        : !exception;
    const expectedResultIsOk = (compRes === true || compRes?.ok);

    /*
     * Displays a message if there is an error
     */
    const ok = internal.incrementCounter(isSame && expectedResultIsOk, xcheck);
    if (!ok || displayMode & 1) {
        const inputExpr = program || getProgram();
        const printableInfo = compInfo
            ? comparatorResultToPrintableInfo(compRes, result, expectedResult, exception)
            : {};
        const opt = ok
            ? OUTPUT_IS_OK | (compInfo ? RESULT_OK : 0)
            : printableInfo.opt
            | (isSame ? OUTPUT_IS_OK : 0)
            | (compInfo
                ? (expectedResultIsOk ? SAME_RESULT_OR_ERROR : 0)
                : (exception ? EXPECTED_NO_ERROR_BUT_ERROR : 0)
            );
        internal.writeTestResult(
            name, xcheck ? 'xcmd' : 'cmd', 4, compInfo
                ? getInputExpression(msgOrInfo, compInfo, op, getProgram, true)
                : (msgOrInfo?.program || getProgram()),
            getMessage(msgOrInfo), exception,
            printableInfo.displayableResult,
            printableInfo.displableExpected,
            opt,
        );
    }

    internal.endTest(ok, true);
    return ok;
}

/**
 * Set the expected output then execute and check \p command.
 * The command can be executed 2 times if the \c blockSelection configuration contains a \c *DUAL_MODE value.
 * @param command value for \c toProgram()
 * @param expectedOutput String | EXPECTED_OUTPUT_AS_INPUT
 * @param msgOrExpectedResultInfo undefined | String | {
 *          \c msg: undefined | String: a message displayed in case of error
 *          \c program undefined | String: string representing the command to be displayed
 *                                         rather than the one automatically calculated
 *          \c expr: represents the test expression. If a sub-string '{input}' exists,
 *                   it will be replaced by the command.
 *          \c op String | Function: see \c getComparatorInfo(). Default is '==='
 *          \c expected: Any
 *          \c error: Boolean: check if an exception is thrown
 *                  | String: check exception message
 *                  | Any: check exception message and class name
 *                  Overwrite property op
 *          \c blockSelection: Boolean
 *      }
 * @param xcheck Boolean:
 */
function testCommand(command, expectedOutput, msgOrExpectedResultInfo, xcheck) {
    let op;
    let compInfo;
    let expectedResult;
    let hasBlockSelectionConfig = false;
    if (typeof msgOrExpectedResultInfo === 'object') {
        /*
         * Extract result operator comparison
         */
        const hasExpected = ('expected' in msgOrExpectedResultInfo);
        const hasError = ('error' in msgOrExpectedResultInfo);
        if (hasExpected || hasError) {
            if (hasExpected && hasError) {
                internal.incrementError();
                throw Error('expected and error property are both defined');
            }

            if (hasExpected) {
                expectedResult = msgOrExpectedResultInfo.expected;
                op = msgOrExpectedResultInfo.op || '===';
            }
            else /*if (hasError)*/ {
                expectedResult = msgOrExpectedResultInfo.error;
                op = msgOrExpectedResultInfo.op;
                if (!opname) {
                    const t = typeof expectedResult;
                    op = (t === 'string') ? 'errorMsg'
                       : (t === 'boolean') ? 'hasError'
                       : 'error';
                }
            }
            compInfo = getComparatorInfo(op, expectedResult);
        }

        hasBlockSelectionConfig = 'blockSelection' in msgOrExpectedResultInfo;
    }

    const blockSelection = ctx.blockSelection;
    const blockSelectionOutput = hasBlockSelectionConfig
        ? msgOrExpectedResultInfo.blockSelection
        : (blockSelection == 1);

    /*
     * Init exepected output
     */
    if (expectedOutput === EXPECTED_OUTPUT_AS_INPUT) {
        internal.copyInputToExpectedOutput(blockSelectionOutput);
    }
    else if (expectedOutput === REUSE_LAST_EXPECTED_OUTPUT) {
        internal.reuseExpectedOutput(blockSelectionOutput);
    }
    else {
        internal.setExpectedOutput(expectedOutput, blockSelectionOutput);
    }

    /*
     * Run and check command
     */
    const [program, getProgram] = toProgram(command);

    let ok = true;
    // when dual mode
    if (blockSelection === DUAL_MODE || blockSelection === ALWAYS_DUAL_MODE) {
        ok = checkCommand(ctx.name, program, getProgram, compInfo, op, expectedResult, msgOrExpectedResultInfo, xcheck);
        if (!ok && blockSelection === DUAL_MODE) {
            checkBreakOnError();
            internal.writeDualModeAborted(ctx.name, 2);
            return;
        }
        if (!internal.reuseInputWithBlockSelection()) {
            return;
        }
        if (!hasBlockSelectionConfig) {
            internal.reuseExpectedOutput(true);
        }
    }
    if (!(checkCommand(ctx.name, program, getProgram, compInfo, op, expectedResult, msgOrExpectedResultInfo, xcheck) && ok)) {
        checkBreakOnError();
    }
}

const tagPrefix = /(^|\n)[^>]*> ?/g;

/**
 * Remove `.*> ` in a tagged template.
 * That
 * \code
 *  t.cmd(enter, 'void foo(){|}', sanitizeTag`> void foo(){
 *                                           ->   |
 *                                            > }`)
 * \endcode
 * is equivalent to
 * \code
 *  t.cmd(enter, 'void foo(){|}', 'void foo(){\n  |\n  }')
 * \endcode
 */
export function sanitizeTag(strings, ...values) {
    if (strings.length === 1) {
        return strings[0].replace(tagPrefix, '$1');
    }

    const a = [strings[0]];
    for (let i = 0; i < values.length; ++i) {
        a.push(values[i], strings[i+1]);
    }
    return a.join('').replace(tagPrefix, '$1');
}

/**
 * Class to test document text and function results.
 */
class TestCase {
    constructor(name, breakOnError, blockSelection) {
        this.name = name;
        this.breakOnError = breakOnError;
        this.blockSelection = blockSelection;
    }

    cmd(command, input, expectedOutput, msgOrExpectedResultInfo, xcheck) {
        internal.setInput(input, this.blockSelection == 1);
        testCommand(command, expectedOutput, msgOrExpectedResultInfo, xcheck);
    }

    resume() {
    }
}

const invalidState = Symbol();

/**
 * Class to test document text and function results.
 * The \c input is reused each time \c cmd() is called.
 */
class TestCaseWithInput {
    constructor(name, breakOnError, blockSelection, input) {
        this.name = name;
        this.breakOnError = breakOnError;
        this.blockSelection = blockSelection;
        this.state = input;
        this.input = input;
    }

    cmd(command, expectedOutput, msgOrExpectedResultInfo, _, xcheck) {
        this._initInput(expectedOutput);
        testCommand(command, expectedOutput, msgOrExpectedResultInfo, xcheck);
    }

    resume() {
        this.state = (typeof this.input === 'string') ? this.input : invalidState;
    }

    _initInput(expectedOutput) {
        if (this.state === REUSE_LAST_INPUT) {
            internal.reuseInput(this.blockSelection == 1);
        }
        else if (this.state === REUSE_LAST_EXPECTED_OUTPUT) {
            internal.moveExpectedOutputToInput(this.blockSelection == 1);
            this.state = REUSE_LAST_INPUT;
        }
        else if (this.state === invalidState) {
            this._invalidStateError();
        }
        else {
            internal.setInput(this.input, this.blockSelection == 1);
            this.state = REUSE_LAST_INPUT;
        }
    }

    _invalidStateError() {
        internal.incrementError();
        throw Error(this.constructor.name + ' is not initialized with a string, which will prevent restoring the input');
    }
}

/**
 * Class to test document text and function results.
 * The \c expectedOutput text will be used as \c input the next time \c cmd() is called.
 */
class TestCaseSequence extends TestCaseWithInput {
    _initInput(expectedOutput) {
        if (this.state === REUSE_LAST_EXPECTED_OUTPUT) {
            internal.moveExpectedOutputToInput(this.blockSelection == 1);
        }
        else if (this.state === REUSE_LAST_INPUT) {
            internal.reuseInput(this.blockSelection == 1);
            this.state = REUSE_LAST_EXPECTED_OUTPUT;
        }
        else if (this.state === invalidState) {
            this._invalidStateError();
        }
        else {
            internal.setInput(this.input, this.blockSelection == 1);
            this.state = REUSE_LAST_EXPECTED_OUTPUT;
        }

        if (expectedOutput !== EXPECTED_OUTPUT_AS_INPUT) {
            this.input = expectedOutput;
        }
    }
}

const ProxyAccess = function(target, prop) {
    const o = target[prop];
    if (typeof o === 'function') {
        const funcName = `${this.name ? this.name : toComparableString(obj)}.${prop.toString()}`;
        // create a function with function.name = funcName and this preservation
        return {[funcName]: (...args) => o.apply(target, args)}[funcName];
    }
    return o;
};

/**
 * Ensures that each method has `${name}.${funcName}` as its name property value.
 * @param name String | falsy: name of \p obj. When falsy, uses a string representation of \p obj.
 * @param obj Object: wrapped object
 * @return Object
 */
export function calleeWrapper(name, obj) {
    return new Proxy(obj, {name, get: ProxyAccess});
}

/**
 * @param config Object: {
 *          // kate config
 *          \c syntax: String,
 *          \c blockSelection: Boolean | DUAL_MODE | ALWAYS_DUAL_MODE,
 *          \c indentationWidth: Number,
 *          \c tabWidth: Number,
 *          \c replaceTabs: Boolean,
 *          \c autoBrackets: Boolean,
 *
 *          // editor state
 *          \c clipboardHistory: Array[String],
 *
 *          // Placeholders in input, output and expected output.
 *          // Only the first letter is used.
 *          // If the string is empty, the placeholder is deactivated.
 *          \c cursor: String,
 *          \c secondaryCursor: String,
 *          \c virtualText: String,
 *
 *          // Placeholders in input, output and expected output.
 *          // The first letter corresponds to selection opening, the second to closing.
 *          // If there is only one letter, opening and closing will have the same value.
 *          // If the string is empty, the placeholder is deactivated.
 *          \c selection: String,
 *          \c secondarySelection: String,
 *
 *          // Placeholders used only in display when normal versions are
 *          // disabled or there is a display conflict (same value to
 *          // represent the same thing)
 *          // Empty strings are replaced by the default value.
 *          \c cursor2: String,
 *          \c secondaryCursor2: String,
 *          \c virtualText2: String,
 *          \c selection2: String,
 *          \c secondarySelection2: String,
 *
 *          \c breakOnError: Boolean
 *      }
 * @return Self
 */
export function config(config) {
    internal.setConfig(config);
    if ('blockSelection' in config) {
        ctx.blockSelection = config.blockSelection;
    }
    if ('breakOnError' in config) {
        ctx.breakOnError = config.breakOnError;
    }
    if (config.clipboardHistory) {
        setClipboardHistory(config.clipboardHistory);
    }
};

/**
 * Uses \c formatArgs(args, ' ') and displays with global \c debug() function.
 */
export function debug(...args) {
    internal.debug(formatArgs(args, ' '));
}

/**
 * Uses \c formatArgs(args, ' ') and displays.
 */
export function print(...args) {
    internal.print(formatArgs(args, ' '));
}

/**
 * Uses \c formatArgs(args, sep) and displays.
 */
export function printSep(sep, ...args) {
    internal.print(formatArgs(args, sep));
}

function subCase(name, fn, Class, input) {
    let newConfig;
    if (name && typeof name === 'object') {
        newConfig = name;
        name = newConfig.name;
    }

    if (name) {
        if (ctx.name) {
            name = `${ctx.name}:${name}`;
        }
        if (!internal.startTestCase(name, 2)) {
            return;
        }
    }
    else {
        name = ctx.name;
    }

    internal.pushConfig();
    const cbHist = clipboardHistory.slice();

    const breakOnError = ctx.breakOnError;
    const newCtx = new Class(name, breakOnError, ctx.blockSelection, input);
    contexts.push(ctx);
    ctx = newCtx;
    if (newConfig) {
        config(newConfig);
    }
    try {
        fn.call(ctx, env);
    }
    catch (e) {
        if (e !== BREAK_CASE_ERROR || breakOnError) {
            throw e;
        }
    }
    finally {
        ctx = contexts.pop();
        ctx.resume();
        clipboardHistory = cbHist;
        internal.popConfig();
    }
}

/**
 * Starts a test of type TestCase.
 * @param nameOrConfig Optional[String|Object]: name is used for filter test.
 *      If it is an object, it is passed to \c config(). An object can contain the name property.
 * @param fn Function(globalThis) with this as TestCase
 */
export function testCase(nameOrConfig, fn) {
    if (!fn) {
        fn = nameOrConfig;
        nameOrConfig = null;
    }
    subCase(nameOrConfig, fn, TestCase);
};

/**
 * Starts a test of type TestCaseSequence.
 * @param nameOrConfig Optional[String|Object]: see \c testCase
 * @param input String
 * @param fn Function(globalThis) with this as TestCaseSequence
 */
export function sequence(nameOrConfig, input, fn) {
    if (!fn) {
        fn = input;
        input = nameOrConfig;
        nameOrConfig = null;
    }
    subCase(nameOrConfig, fn, TestCaseSequence, input);
};

/**
 * Starts a test of type TestCaseWithInput.
 * @param nameOrConfig Optional[String|Object]: see \c testCase
 * @param input String
 * @param fn Function(globalThis) with this as TestCaseWithInput
 */
export function withInput(nameOrConfig, input, fn) {
    if (!fn) {
        fn = input;
        input = nameOrConfig;
        nameOrConfig = null;
    }
    subCase(nameOrConfig, fn, TestCaseWithInput, input);
};

function checkBreakOnError() {
    if (internal.hasTooManyErrors()) {
        throw STOP_CASE_ERROR;
    }
    if (ctx.breakOnError) {
        internal.incrementBreakOnError();
        throw BREAK_CASE_ERROR;
    }
}

/**
 * Test the indentation of all files in the \p dataDir folder.
 * This folder must contain subfolders with \c origin and \c expected files.
 * If the result is different, a file named \c actual will be written and
 * the difference will be displayed.
 * @param nameOrDataDir String: name used for filter test and \p dataDir when this parameter is absent
 * @param dataDir String: path of directory
 */
export function indentFiles(nameOrDataDir, dataDir) {
    dataDir = dataDir || nameOrDataDir;
    if (nameOrDataDir) {
        if (ctx.name) {
            nameOrDataDir = `${ctx.name}:${nameOrDataDir}`;
        }
    }
    if (!internal.testIndentFiles(nameOrDataDir, dataDir, 1, ctx.breakOnError)) {
        checkBreakOnError();
    }
}

function _test(command, op, expected, msgOrInfo, xcheck) {
    const compInfo = getComparatorInfo(op, expected);
    const [program, getProgram] = toProgram(command);
    const displayMode = internal.startTest();
    if (displayMode & 2) {
        internal.writeTestExpression(
            ctx.name, xcheck ? 'xtest' : 'test', 2,
            getInputExpression(msgOrInfo, compInfo, op, getProgram)
        );
    }

    const [result, exception] = executeProgram(program);
    const res = compInfo.comp(result, expected, exception);
    const ok = internal.incrementCounter(res === true || res?.ok, xcheck);
    if (!ok || displayMode & 1) {
        const printableInfo = comparatorResultToPrintableInfo(res, result, expected, exception);
        internal.writeTestResult(
            ctx.name, xcheck ? 'xtest' : 'test', 2,
            getInputExpression(msgOrInfo, compInfo, op, getProgram),
            getMessage(msgOrInfo), exception,
            printableInfo.displayableResult,
            printableInfo.displableExpected,
            IGNORE_INPUT_OUTPUT | (ok ? RESULT_OK : CONTAINS_RESULT_OR_ERROR | printableInfo.opt),
        );
    }

    internal.endTest(ok);

    if (!ok) {
        checkBreakOnError();
    }
}

/**
 * Executes \p command and uses the \p op operator to compare it to \p expected.
 * @param command value for \c toProgram()
 * @param op String | Function: key of \c operators or \c Function with
 *                              optional \p opname property. See the description
 *                              of \c operators for the function signature.
 * @param msgOrInfo undefined | String | {
 *          \c msg: Optional[String]
 *          \c expr: Optional[String]
 *          \c program: Optional[String]
 *      }
 * @return Self
 */
export function test(command, op, expected, msgOrInfo) { _test(command, op, expected, msgOrInfo); }

/**
 * Same as \c test(), but for an expected failure.
 */
export function xtest(command, op, expected, msgOrInfo) { _test(command, op, expected, msgOrInfo, true); }

/// Shortcut for test() function.
/// @{
export function eqvTrue(command, msgOrInfo) { _test(command, '!!', true, msgOrInfo); }
export function eqvFalse(command, msgOrInfo) { _test(command, '!!', false, msgOrInfo); }
export function eqTrue(command, msgOrInfo) { _test(command, '===', true, msgOrInfo); }
export function eqFalse(command, msgOrInfo) { _test(command, '===', false, msgOrInfo); }

export function error(command, expected, msgOrInfo) { _test(command, 'error', expected, msgOrInfo); }
export function errorMsg(command, expected, msgOrInfo) { _test(command, 'errorMsg', expected, msgOrInfo); }
export function errorType(command, expected, msgOrInfo) { _test(command, 'errorType', expected, msgOrInfo); }
export function hasError(command, msgOrInfo) { _test(command, 'hasError', true, msgOrInfo); }

export function eqv(command, expected, msgOrInfo) { _test(command, 'eqv', expected, msgOrInfo); }
export function is(command, expected, msgOrInfo) { _test(command, 'is', expected, msgOrInfo); }
export function eq(command, expected, msgOrInfo) { _test(command, '===', expected, msgOrInfo); }
export function ne(command, expected, msgOrInfo) { _test(command, '!=', expected, msgOrInfo); }
export function lt(command, expected, msgOrInfo) { _test(command, '<', expected, msgOrInfo); }
export function gt(command, expected, msgOrInfo) { _test(command, '>', expected, msgOrInfo); }
export function le(command, expected, msgOrInfo) { _test(command, '<=', expected, msgOrInfo); }
export function ge(command, expected, msgOrInfo) { _test(command, '>=', expected, msgOrInfo); }
/// @}

/**
 * @param command Function | String | Array[Function | String, ...args] | LazyFunc
 * @param input String: this parameter does not exist for context sequence() and withInput()
 * @param expectedOutput String | EXPECTED_OUTPUT_AS_INPUT
 * @param msgOrExpectedResultInfo undefined | String | {
 *          \c msg: undefined | String: a message displayed in case of error
 *          \c program undefined | String: string representing the command to be displayed
 *                                         rather than the one automatically calculated
 *          \c op String | Function: see \c getComparatorInfo(). Default is '==='
 *          \c expected: Any
 *          \c error: Boolean: check if an exception is thrown
 *                  | String: check exception message
 *                  | Any: check exception message and class name
 *                  Overwrite property op
 *          \c blockSelection: Boolean
 *      }
 * @return Self
 */
export function cmd(command, input, expectedOutput, msgOrExpectedResultInfo) {
    ctx.cmd(command, input, expectedOutput, msgOrExpectedResultInfo);
}

/**
 * Same as \c cmd(), but for an expected failure.
 */
export function xcmd(command, input, expectedOutput, msgOrExpectedResultInfo) {
    ctx.cmd(command, input, expectedOutput, msgOrExpectedResultInfo, true);
}

let kbd_enter;
let kbd_type;

export function keys(str) {
    return str === '\n' ? kbd_enter : [kbd_type, str];
}

export function type(str, input, expectedOutput, msgOrExpectedResultInfo) {
    ctx.cmd(keys(str), input, expectedOutput, msgOrExpectedResultInfo);
}

export function xtype(str, input, expectedOutput, msgOrExpectedResultInfo) {
    ctx.cmd(keys(str), input, expectedOutput, msgOrExpectedResultInfo, true);
}

export function init(internal_, env_, blockSelection) {
    internal = internal_;
    env = env_;
    ctx = new TestCase('', false, blockSelection);
    const kbd = calleeWrapper('kbd', internal);
    kbd_enter = kbd.enter;
    kbd_type = kbd.type;
    return kbd;
}
