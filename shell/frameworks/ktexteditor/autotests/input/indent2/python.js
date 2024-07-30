config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'python',
    indentationMode: 'python',
    selection: '',
});

testCase('python:dedentBreak', () => {
    type('\n'   , c`>     while True:
                    >         break`

                , c`>     while True:
                    >         break
                    >     `);
});

testCase('python:dedentComment', () => {
    type('\n'   , c`>     def some_function():
                    >         return  # comment`

                , c`>     def some_function():
                    >         return  # comment
                    >     `);
});

testCase('python:dedentContinue', () => {
    type('\n'   , c`>     while True:
                    >         continue`

                , c`>     while True:
                    >         continue
                    >     `);
});

withInput('python:dedentElif'
                    , c`> if True:
                        >     1
                        >     `,  () => {

    type('elif '    , c`> if True:
                        >     1
                        > elif `);

    type('\telif '  , c`> if True:
                        >     1
                        >         elif `);
});

testCase('python:dedentElse', () => {
    type('else:', c`> if True:
                    >     1
                    >     `

                , c`> if True:
                    >     1
                    > else:`);

    type('\telse:'
                , c`> if True:
                    >     1
                    >     `

                , c`> if True:
                    >     1
                    >         else:`);

    sequence(         c`> if True:
                        >     a = 1
                        >          ${''}
                        >
                        >
                        >     b = 2`, () => {

        type('\n'   , c`> if True:
                        >     a = 1
                        >          ${''}
                        >
                        >
                        >     b = 2
                        >     `);

        type('else' , c`> if True:
                        >     a = 1
                        >          ${''}
                        >
                        >
                        >     b = 2
                        >     else`);

        type(':'    , c`> if True:
                        >     a = 1
                        >          ${''}
                        >
                        >
                        >     b = 2
                        > else:`);
    });

    sequence(         c`> if True:
                        >     a = 1
                        >          ${''}
                        >     `, () => {

        type('\n', c`> if True:
                        >     a = 1
                        >          ${''}
                        >     ${''}
                        >     `);

        type('\n'   , c`> if True:
                        >     a = 1
                        >          ${''}
                        >     ${''}
                        >     ${''}
                        >     `);

        type('else:', c`> if True:
                        >     a = 1
                        >          ${''}
                        >     ${''}
                        >     ${''}
                        > else:`);
    });

    type('else:', c`> if True:
                    >     a = 1
                    >     # test comment
                    >
                    >     # test comment 2
                    >     `

                , c`> if True:
                    >     a = 1
                    >     # test comment
                    >
                    >     # test comment 2
                    > else:`);

    type('\telse:'
                , c`>     if True:
                    >         a = 1
                    > `

                , c`>     if True:
                    >         a = 1
                    >     else:`);
});

withInput('python:dedentExcept'
                    , c`> try:
                        >     a = 1 + 1
                        >     `, () => {

    type('except '  , c`> try:
                        >     a = 1 + 1
                        > except `);

    type('\texcept ', c`> try:
                        >     a = 1 + 1
                        >         except `);
});

withInput('python:dedentFinally'
                    , c`> try:
                        >     a = 1 + 1
                        >     `, () => {

    type('finally:' , c`> try:
                        >     a = 1 + 1
                        > finally:`);

    type('\tfinally:'
                    , c`> try:
                        >     a = 1 + 1
                        >         finally:`);
});

testCase('python:dedentPass', () => {
    type('\n'   , c`> def some_function():
                    >     pass`

                , c`> def some_function():
                    >     pass
                    > `);
});

testCase('python:dedentRaise', () => {
    type('\n'   , c`> try:
                    >     raise`

                , c`> try:
                    >     raise
                    > `);
});

testCase('python:dedentReturn', () => {
    type('\n'   , c`> def some_function():
                    >     return`

                , c`> def some_function():
                    >     return
                    > `);

    sequence(         c`> def some_function():
                        >     return other(arg1,`, () => {

        type('\n'   , c`> def some_function():
                        >     return other(arg1,
                        >                  `);

        type('arg2)', c`> def some_function():
                        >     return other(arg1,
                        >                  arg2)`);

        type('\n', c`> def some_function():
                        >     return other(arg1,
                        >                  arg2)
                        > `);
    });

    type('\n'   , c`> class Foo(object):
                    >     def some_function():
                    >         return other(arg1,
                    >                      arg2,
                    >                      arg3)`
                , c`> class Foo(object):

                    >     def some_function():
                    >         return other(arg1,
                    >                      arg2,
                    >                      arg3)
                    >     `);
});

testCase('python:dict', () => {
    sequence(            'a_dict = {  # opening dictionary w/out elements should indent once', () => {

        type('\n'   , c`> a_dict = {  # opening dictionary w/out elements should indent once
                        >     `);

        type('0:'   , c`> a_dict = {  # opening dictionary w/out elements should indent once
                        >     0:`);

        type(" 'a',\n"
                    , c`> a_dict = {  # opening dictionary w/out elements should indent once
                        >     0: 'a',
                        >     `);

        type("1: 'b'}"
                    , c`> a_dict = {  # opening dictionary w/out elements should indent once
                        >     0: 'a',
                        >     1: 'b'}`);

        type('\n'   , c`> a_dict = {  # opening dictionary w/out elements should indent once
                        >     0: 'a',
                        >     1: 'b'}
                        > `);
    });

    sequence(            "a_dict = {0: 'a',  # opening dictionary w/ elements indents to opening", () => {

        type('\n'   , c`> a_dict = {0: 'a',  # opening dictionary w/ elements indents to opening
                        >           `);

        type('1:'   , c`> a_dict = {0: 'a',  # opening dictionary w/ elements indents to opening
                        >           1:`);

        type(" 'b', }"
                    , c`> a_dict = {0: 'a',  # opening dictionary w/ elements indents to opening
                        >           1: 'b', }`);

        type('\n'   , c`> a_dict = {0: 'a',  # opening dictionary w/ elements indents to opening
                        >           1: 'b', }
                        > `);
    });
});

testCase('python:func_args_indent', () => {
    type('\n'    , 'foo = long_function_name(var_one, var_two,',
                c`> foo = long_function_name(var_one, var_two,
                  >                          |`);
});

testCase('python:func_args', () => {
    sequence(            'def test2(arg1,  # opening function w/ arguments indents to opening', () => {
        type('\n'   , c`> def test2(arg1,  # opening function w/ arguments indents to opening
                        >           |`);

        type('arg2):'
                    , c`> def test2(arg1,  # opening function w/ arguments indents to opening
                        >           arg2):`);

        type('\n'   , c`> def test2(arg1,  # opening function w/ arguments indents to opening
                        >           arg2):
                        >     `);
    });
});

testCase('python:indentColon', () => {
    type('\n'   , 'def some_function(param, param2):'
                , 'def some_function(param, param2):\n    ');

    type('\n'   , 'def some_function():  # comment'
                , 'def some_function():  # comment\n    ');
});

testCase('python:indentPaste', () => {
    cmd([paste, "if True:\n    print('b')"]
        , c`> def function():
            >     print("a")
            > `

        , c`> def function():
            >     print("a")
            >     if True:
            >         print('b')`);

    cmd([paste
        , c`> def internal():
            >     '''test
            >     docstring
            >
            >     example
            >         foo
            > '''
            >     if True:
            >         return True`]

        , c`> def function():
            >     print("a")
            >     if True:
            >         print("def internal")
            > `

        , c`> def function():
            >     print("a")
            >     if True:
            >         print("def internal")
            >         def internal():
            >             '''test
            >             docstring
            >             ${''}
            >             example
            >                 foo
            >         '''
            >             if True:
            >                 return True`);

    cmd([paste
        , c`> def internal():
            >     a = '''multi
            > line
            >     string
            >     '''
            >     if True:
            >         return True`]

        ,  c`> def function():
            >     print("a")
            >     if True:
            >         print("def internal")
            > `

        , c`> def function():
            >     print("a")
            >     if True:
            >         print("def internal")
            >         def internal():
            >             a = '''multi
            > line
            >     string
            >     '''
            >             if True:
            >                 return True`);
});

testCase('python:keepIndent', () => {
    type('\n'   , c`> def some_function(param, param2):
                    >     a = 5
                    >     b = 7`

                , c`> def some_function(param, param2):
                    >     a = 5
                    >     b = 7
                    >     `);

    type('\n'   , c`> class my_class():
                    >     def my_fun():
                    >         print("Foo")
                    >         print(3)`

                , c`> class my_class():
                    >     def my_fun():
                    >         print("Foo")
                    >         print(3)
                    >         `
);
    type('\n'   , c`> while True:
                    >     returnFunc()
                    >     myVar = 3`

                , c`> while True:
                    >     returnFunc()
                    >     myVar = 3
                    >     `);

    type('\n'   , c`> def some_function():
                    >     pass
                    > `

                , c`> def some_function():
                    >     pass
                    >
                    > `);

    type('\n'   , c`> def some_function(param, param2):
                    >     a = """
                    >     multi
                    > line
                    > """`

                , c`> def some_function(param, param2):
                    >     a = """
                    >     multi
                    > line
                    > """
                    >     `);

    type('\n'   , c`> def some_function(param, param2):
                    >     '''
                    >     docstring
                    >     dedent
                    > '''`

                , c`> def some_function(param, param2):
                    >     '''
                    >     docstring
                    >     dedent
                    > '''
                    >     `);
});

testCase('python:list', () => {
    sequence(            'mylist = [  # opening list w/out elements should indent once', () => {

        type('\n'   , c`> mylist = [  # opening list w/out elements should indent once
                        >     `);

        type('1,'   , c`> mylist = [  # opening list w/out elements should indent once
                        >     1,`);

        type('\n'   , c`> mylist = [  # opening list w/out elements should indent once
                        >     1,
                        >     `);

        type('2]'   , c`> mylist = [  # opening list w/out elements should indent once
                        >     1,
                        >     2]`);

        type('\n'   , c`> mylist = [  # opening list w/out elements should indent once
                        >     1,
                        >     2]
                        > `);
    });

    sequence(            'mylist = [1,  # opening list w/ elements indents to opening', () => {

        type('\n'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           `);

        type('2,'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,`);

        type('\n'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,
                        >           `);

        type('3]'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,
                        >           3]`);

        type('\n'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,
                        >           3]
                        > `);
    });
});

testCase('python:triggerchars', () => {
    type("abcdefghijklmnopqrstuvwxyz123456789;:_-#+-*/~()\{\}?\\"
        , c`> def func():
            >     print(foo)
            > `

        , c`> def func():
            >     print(foo)
            > abcdefghijklmnopqrstuvwxyz123456789;:_-#+-*/~(){}?\\`);
});

testCase('python:tuple', () => {
    sequence(            'a_tuple = (  # opening tuple w/out elements should indent once', () => {

        type('\n'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     `);

        type('1,'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,`);

        type('\n'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,
                        >     `);

        type('2)'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,
                        >     2)`);

        type('\n'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,
                        >     2)
                        > `);
    });

    sequence(            'a_tuple = (1,  # opening tuple w/ elements indents to opening', () => {

        type('\n'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            `);

        type('2,'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,`);

        type('\n'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,
                        >            `);

        type('3)'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,
                        >            3)`);

        type('\n'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,
                        >            3)
                        > `);
    });
});

testCase('python:wrapped_line', () => {
    sequence(            `long_statement = "a" + \\  # doesn't fit on a line`, () => {

        type('\n'   , c`> long_statement = "a" + \\  # doesn't fit on a line
                        >     `);

        type('"b"'  , c`> long_statement = "a" + \\  # doesn't fit on a line
                        >     "b"`);

        type('\n'   , c`> long_statement = "a" + \\  # doesn't fit on a line
                        >     "b"
                        > `);
    });
});
