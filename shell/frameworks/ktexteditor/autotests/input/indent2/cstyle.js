config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 2,
    indentationWidth: 2,
    syntax: 'C',
    indentationMode: 'cstyle',
    selection: '',
});

function alignCurrentPosition() {
    const cur = view.cursorPosition();
    view.align(new Range(cur, cur));
}

function alignDoc() {
    view.align(document.documentRange());
}

testCase('cstyle.bug_137157', () => {
    sequence(         c`> # 1
                        > do {
                        > }
                        >  while (0);
                        >  |`, () => {

        type('\n'   , c`> # 1
                        > do {
                        > }
                        >  while (0);
                        >  ${''}
                        >  |`);

        type('ok'   , c`> # 1
                        > do {
                        > }
                        >  while (0);
                        >  ${''}
                        >  ok|`);
    });
});

testCase('cstyle.bug_360456', () => {
    sequence(         c`>    int *test;
                        >     /* Here, we assign a value to the pointee object */
                        >     *test = 42;|`, () => {

        type('\n'   , c`>    int *test;
                        >     /* Here, we assign a value to the pointee object */
                        >     *test = 42;
                        >     |`);

        type('test = 0;'
                    , c`>    int *test;
                        >     /* Here, we assign a value to the pointee object */
                        >     *test = 42;
                        >     test = 0;|`);
    });
});

testCase('cstyle.bug_385472', () => {
    type('\n'   , c`> int f()
                    > {\n'
                    >   if (1)
                    >     if (1)
                    >       f();
                    >     else
                    >       f();|`

                , c`> int f()
                    > {\n'
                    >   if (1)
                    >     if (1)
                    >       f();
                    >     else
                    >       f();
                    >   |`);
});

testCase('cstyle.alignbrace', () => {
    cmd(alignCurrentPosition
        , c`>   if ( true ) {
            >
            > |}`

        , c`>   if ( true ) {
            >
            >   |}`);
});

testCase('cstyle.aplist', () => {
    type('\n'   , c`> int main(int argc, char **argv) {
                    >   somefunctioncall(argc,|`

                , c`> int main(int argc, char **argv) {
                    >   somefunctioncall(argc,
                    >                    |`);

    for (const sp of ['', ' ']) {
        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,|${sp}argv,
                        >                    ok,`

                    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    |argv,
                        >                    ok,`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    argv,|${sp}argx,
                        >                    ok,`

                    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    argv,
                        >                    |argx,
                        >                    ok,`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,|${sp}argv,`

                    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               |argv,`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               argv,|${sp}argx,`

                    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               argv,
                        >                               |argx,`);
    }

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok|`, () => {

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok)|`);

        type(','    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),
                        >                    |`);

        type('ok'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),
                        >                    ok|`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),
                        >                    ok)|`);

        type(';'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),
                        >                    ok);|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok),
                        >                    ok);
                        >   |`);
    });

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,|`, () => {

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               |`);

        type('ok'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok|`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok)|`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok))|`);

        type(';'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok));|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    nestedcall(var,
                        >                               ok));
                        >   |`);
    });

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,|`, () => {

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               |`);

        type('ok'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok|`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok)|`);

        type(','    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),
                        >                    |`);

        type('ok'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),
                        >                    ok|`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),
                        >                    ok)|`);

        type(';'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),
                        >                    ok);|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(nestedcall(var,
                        >                               ok),
                        >                    ok);
                        >   |`);
    });

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    ok,
                        >                    argv[0]|`, () => {

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    ok,
                        >                    argv[0])|`);

        type(';'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    ok,
                        >                    argv[0]);|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(argc,
                        >                    ok,
                        >                    argv[0]);
                        >   |`);
    });

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(|`, () => {

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >     |`);

        type('ok\n' , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >     ok
                        >     |`);

        type(')'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >     ok
                        >   )|`);
    });

    sequence(         c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >                    ok
                        >                   )|`, () => {

        type(';'    , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >                    ok
                        >                   );|`);

        type('\n'   , c`> int main(int argc, char **argv) {
                        >   somefunctioncall(
                        >                    ok
                        >                   );
                        >   |`);
    });
});

testCase('cstyle.brace', () => {
    type('\n'   , c`> int f()
                    > { // https://bugs.kde.org/show_bug.cgi?id=309800|`

                , c`> int f()
                    > { // https://bugs.kde.org/show_bug.cgi?id=309800
                    >   |`);

    cmd(alignCurrentPosition, c`> int main(void)
                                > {
                                >   printf("{\\n");
                                > |    return 0;
                                > }`

                            , c`> int main(void)
                                > {
                                >   printf("{\\n");
                                >   |return 0;
                                > }`);
});

testCase('cstyle.closepar', () => {
    sequence(         c`> int main() {
                        >   ok;|`, () => {

        type('\n'   , c`> int main() {
                        >   ok;
                        >   |`);

        type('}'    , c`> int main() {
                        >   ok;
                        > }|`);
    });

    sequence(         c`> int main()
                        > {
                        >   ok;|`, () => {

        type('\n'   , c`> int main()
                        > {
                        >   ok;
                        >   |`);

        type('}'    , c`> int main()
                        > {
                        >   ok;
                        > }|`);
    });

    type('\n'   , c`> int main() {
                    >   ok;|}`

                , c`> int main() {
                    >   ok;
                    >   |
                    > }`);

    type('\n'   , c`> int main() {
                    >   for() {
                    >     x;|}`

                , c`> int main() {
                    >   for() {
                    >     x;
                    >     |
                    >   }`);
});

testCase('cstyle.comma', () => {
    type('\n'   , c`> int fla() {
                    >   double x,|`

                , c`> int fla() {
                    >   double x,
                    >   |`);

    type('\n'   , c`> int fla() {
                    >   double x,|y;`

                , c`> int fla() {
                    >   double x,
                    >   |y;`);

    type('\n'   , c`> int fla() {
                    >   b = 1,|`

                , c`> int fla() {
                    >   b = 1,
                    >   |`);

    type('\n'   , c`> int fla() {
                    >   b = 1,|c = 2;`

                , c`> int fla() {
                    >   b = 1,
                    >   |c = 2;`);

    type('\n'   , 'double x,|'
                , c`> double x,
                    > |`);

    type('\n'   , 'double x,|y;'
                , c`> double x,
                    > |y;`);
});

testCase('cstyle.comment', () => {
    type('\n'   , c`>  int foo() {
                    >     x;
                    > //     y;|`

                , c`>  int foo() {
                    >     x;
                    > //     y;
                    >     |`);

    type('\n'   ,    'foo(); // "comment"|'
                , c`> foo(); // "comment"
                    > |`);
});

testCase('cstyle.dart', () => {
    config({
        syntax: 'Dart',
    });

    sequence(         c`> TextField(
                        >   maxLines: 1,
                        >   style: const TextStyle(fontSize: 30),|
                        > )`, () => {

        type('\n'   , c`> TextField(
                        >   maxLines: 1,
                        >   style: const TextStyle(fontSize: 30),
                        >   |
                        > )`);

        type('controller: _controller,'
                    , c`> TextField(
                        >   maxLines: 1,
                        >   style: const TextStyle(fontSize: 30),
                        >   controller: _controller,|
                        > )`);
    });
});

testCase('cstyle.do', () => {
    type('\n'   , c`> int fla() {
                    >   do|`

                , c`> int fla() {
                    >   do
                    >     |`);

    type('\n'   , c`> int fla() {
                    >   do
                    >     x--;|`

                , c`> int fla() {
                    >   do
                    >     x--;
                    >   |`);

    type('\n'   , c`> int fla() {
                    >   do x();|`

                , c`> int fla() {
                    >   do x();
                    >   |`);
});

testCase('cstyle.doxygen', () => {
    sequence(         c`> class A {
                        >   /**|`, () => {

        type('\n'   , c`> class A {
                        >   /**
                        >    * |`);

        type('constructor'
                    , c`> class A {
                        >   /**
                        >    * constructor|`);

        type('\n'   , c`> class A {
                        >   /**
                        >    * constructor
                        >    * |`);

        type('@param x foo\n'
                    , c`> class A {
                        >   /**
                        >    * constructor
                        >    * @param x foo
                        >    * |`);

        type('/'    , c`> class A {
                        >   /**
                        >    * constructor
                        >    * @param x foo
                        >    */|`);

        type('\n'   , c`> class A {
                        >   /**
                        >    * constructor
                        >    * @param x foo
                        >    */
                        >   |`);
    });

    sequence(         c`> class A {
                        >   /**|`, () => {

        type(' constructor */'
                    , c`> class A {
                        >   /** constructor */|`);

        type('\n'   , c`> class A {
                        >   /** constructor */
                        >   |`);
    });

    type('\n'   , c`> class A {
                    >   int foo(); /** unorthodox doxygen comment */|`

                , c`> class A {
                    >   int foo(); /** unorthodox doxygen comment */
                    >   |`);

    type('\n'   ,    '/** unorthodox doxygen comment */ a;|'
                , c`> /** unorthodox doxygen comment */ a;
                    > |`);

    type('\n'   , c`>  // in CSS this matches all tags, so do not auto-insert a \'*\'
                    >   * a|`

                , c`>  // in CSS this matches all tags, so do not auto-insert a \'*\'
                    >   * a
                    >   |`);

    type('\n'   , c`> /* foo
                    >  * bar */
                    > *buf = 0;|`

                , c`> /* foo
                    >  * bar */
                    > *buf = 0;
                    > |`);

    cmd(alignDoc, c`> |/**
                    > * foo
                    > * bar
                    > */
                    > int foo = 1;`

                , c`> |/**
                    >  * foo
                    >  * bar
                    >  */
                    > int foo = 1;`);
});

testCase('cstyle.for', () => {
    type('\n'   , c`> int main() {
                    >   for (int a = 0;|`

                , c`> int main() {
                    >   for (int a = 0;
                    >        |`);

    sequence(       c`> int main() {
                        >   for (int a = 0;
                        >        b;
                        >        c)|`, () => {

        type(' {',  c`> int main() {
                        >   for (int a = 0;
                        >        b;
                        >        c) {|`);

        type('\n'   , c`> int main() {
                        >   for (int a = 0;
                        >        b;
                        >        c) {
                        >     |`);
    });

    type('\n'   , c`> int fla() {
                    >   for (;0<x;)|`

                , c`> int fla() {
                    >   for (;0<x;)
                    >     |`);

    type('\n'   , c`> int fla() {
                    >   for (;0<x;)
                    >     x--;|`

                , c`> int fla() {
                    >   for (;0<x;)
                    >     x--;
                    >   |`);

    type('\n'   , c`> int fla() {
                    >   for (;0<x;) x();|`

                , c`> int fla() {
                    >   for (;0<x;) x();
                    >   |`);
});

testCase('cstyle.foreign', () => {
    sequence(         c`> // indent-width is 2 but we want to maintain an indentation of 4
                        > int main() {|`, () => {

        type('\n'   , c`> // indent-width is 2 but we want to maintain an indentation of 4
                        > int main() {
                        >   |`);

        type('  bla', c`> // indent-width is 2 but we want to maintain an indentation of 4
                        > int main() {
                        >     bla|`);

        type(';'    , c`> // indent-width is 2 but we want to maintain an indentation of 4
                        > int main() {
                        >     bla;|`);

        type('\n'   , c`> // indent-width is 2 but we want to maintain an indentation of 4
                        > int main() {
                        >     bla;
                        >     |`);
    });
});

testCase('ctyles:js', () => {
    config({
        syntax: 'JavaScript',
    });

    type('\n'   , c`> console.log("hello", function(){
                    >   if (true) do_x();|
                    > });`

                , c`> console.log("hello", function(){
                    >   if (true) do_x();
                    >   |
                    > });`
                , 'func_in_param');

    type('\n'   , c`> const cars = [|
                    >   "mycar"
                    > ];`

                , c`> const cars = [
                    >   |
                    >   "mycar"
                    > ];`
                , 'jsarray');

    type('\n'   , c`> function(['asdf']) {
                    >   'use strict';`

                , c`> function(['asdf']) {
                    >   'use strict';
                    >   `
                , 'usestrict')
});

testCase('ctyles:indentpaste', () => {
    cmd([paste, 'puts("World");']
        , c`> int main() {
            >     puts("Hello!");
            > |
            > }`

        , c`> int main() {
            >     puts("Hello!");
            >     puts("World");|
            > }`);

    cmd([paste, '  puts("World");']
        , c`> int main() {
            >   puts("Hello!");
            >   |
            > }`

        , c`> int main() {
            >   puts("Hello!");
            >     puts("World");|
            > }`);

    cmd([paste  , c`>     if (true) {
                    >         puts("World!");
                    >     }`]

                , c`> int main() {
                    >   puts("Hello!");
                    >   |
                    > }`

                , c`> int main() {
                    >   puts("Hello!");
                    >   if (true) {
                    >     puts("World!");
                    >   }|
                    > }`);

    cmd([paste  , c`> int main(int argc, char const *argv[])
                    > {
                    >   /* code */
                    >   return 0;
                    > }`]

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   /* code */
                    >   return 0;
                    >   |
                    > }`

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   /* code */
                    >   return 0;
                    >   int main(int argc, char const *argv[])
                    >   {
                    >     /* code */
                    >     return 0;
                    >   }|
                    > }`);

    config({
        syntax: 'C++',
    });

    cmd([paste  , c`> const char * multilineindented = R"sometext(
                    >              Here's some multiline text
                    >       whose indentation
                    >              must be preserved perfectly
                    >         like this
                    > )sometext";`]

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   |
                    >   return 0;
                    > }`

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   const char * multilineindented = R"sometext(
                    >              Here's some multiline text
                    >       whose indentation
                    >              must be preserved perfectly
                    >         like this
                    > )sometext";|
                    >   return 0;
                    > }`);

    cmd([paste  , c`> if (true) {
                    > puts();
                    > }`]

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   const char * multilineindented = R"sometext(
                    > )sometext";
                    >   ${''}
                    >   |
                    >   return 0;
                    > }`

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   const char * multilineindented = R"sometext(
                    > )sometext";
                    >   ${''}
                    >   if (true) {
                    >     puts();
                    >   }|
                    >   return 0;
                    > }`);

    config({
        selection: '[]',
        syntax: 'C',
    });
    for (const str of
        [ c`> if(rand() % 2)
            >   if_odd_do_this();
            > else
            >   else_even_do_that();
            > in_any_case_do_this();`

        , c`> if(true)
            >   fork();
            > printf("PID: %d\\n", getpid());`
        ]
    ) {
        cmd([paste, str], `[${str}]`, str);
    }
});

testCase('ctyles:normal', () => {
    type('\n'   , c`> int main() {
                    >     bla;|`

                , c`> int main() {
                    >     bla;
                    >     |`);

    for (const sp of ['', '  ']) {
        type('\n'   , c`> int main() {
                        >     bla;|${sp}blu;`

                    , c`> int main() {
                        >     bla;
                        >     |blu;`);
    }
});

testCase('ctyles:openpar', () => {
    sequence(      'int main()', () => {
        type(' ' , 'int main() |');
        type('{' , 'int main() {|');
        type('\n', 'int main() {\n  |');
    });

    sequence(      'int main()', () => {
        type('\n', 'int main()\n|');
        type('{' , 'int main()\n{|');
        type('\n', 'int main()\n{\n  |');
    });

    type('\n'   , 'int main() {|bla\n'
                , 'int main() {\n  |bla\n');

    type('\n'   , 'int main() {|    bla\n'
                , 'int main() {\n  |bla\n');

    type('\n'   , 'int main() {|foo();\n'
                , 'int main() {\n  |foo();\n');

    type('\n'   , 'int main()\n{|bla'
                , 'int main()\n{\n  |bla');

    type('\n'   , 'int main()\n{|    bla\n'
                , 'int main()\n{\n  |bla\n');

    type('\n'   , 'int main()\n{|foo();\n'
                , 'int main()\n{\n  |foo();\n');

    sequence(      'int main() {\n  if (x) {\n    a;\n  } else|', () => {
        type(' {', 'int main() {\n  if (x) {\n    a;\n  } else {|');
        type('\n', 'int main() {\n  if (x) {\n    a;\n  } else {\n    |');
    });

    sequence(      'int main() {\n  if (x) {\n    a;\n  } else if (y, z)|', () => {
        type(' {', 'int main() {\n  if (x) {\n    a;\n  } else if (y, z) {|');
        type('\n', 'int main() {\n  if (x) {\n    a;\n  } else if (y, z) {\n    |');
    });

    config({autoBrackets: true});

    sequence(      'class X\n{\n  void foo()\n  {\n    if (true)|\n};', () => {
        type('\n', 'class X\n{\n  void foo()\n  {\n    if (true)\n      |\n};');
        type('{' , 'class X\n{\n  void foo()\n  {\n    if (true)\n    {|}\n};');
    });

    sequence(      'class X\n{\n  void foo()|\n};\n', () => {
        type('\n', 'class X\n{\n  void foo()\n  |\n};\n');
        type('{' , 'class X\n{\n  void foo()\n  {|}\n};\n');
        type('\n', 'class X\n{\n  void foo()\n  {\n    |\n  }\n};\n');
    });
});

testCase('ctyles:plist', () => {
    sequence(                'int fla(int x,', () => {

        type('\n'       , c`> int fla(int x,
                            >         `);

        type('short u,' , c`> int fla(int x,
                            >         short u,`);

        type('\n'       , c`> int fla(int x,
                            >         short u,
                            >         `);

        type('char c)'  , c`> int fla(int x,
                            >         short u,
                            >         char c)`);

        type('\n'       , c`> int fla(int x,
                            >         short u,
                            >         char c)
                            > `);

        type('{'        , c`> int fla(int x,
                            >         short u,
                            >         char c)
                            > {`);
    });

    sequence(         c`> int fla(int x,
                        >         short u,
                        >         char c)`, () => {

        type(' {' ,   c`> int fla(int x,
                        >         short u,
                        >         char c) {`);

        type('\n' ,   c`> int fla(int x,
                        >         short u,
                        >         char c) {
                        >   `);
    });

    type('\n'   ,    'uint8_t func( uint8_t p1,| uint8_t p2)'
                , c`> uint8_t func( uint8_t p1,
                    >               |uint8_t p2)`);

    type('\n'   , c`>
                    > uint8_t func( uint8_t p1,| uint8_t p2)`
                , c`>
                    > uint8_t func( uint8_t p1,
                    >               |uint8_t p2)`);

    type('\n'   ,    'int fla(int x,|short u,char c)'
                , c`> int fla(int x,
                    >         |short u,char c)`);

    type('\n'   , c`> int fla(int x,
                    >         short u,|char c)`

                , c`> int fla(int x,
                    >         short u,
                    >         |char c)`);

    xtype('\n'  ,    'int fla(|int x,short u,char c)'
                , c`> int fla(
                    >         |int x,short u,char c)`

                , 'low low prio, maybe wontfix: if the user wants to add a arg, he should do so and press enter afterwards');

    type('\n'   , c`> int fla(
                    >         int x,|short u,char c)`

                , c`> int fla(
                    >         int x,
                    >         |short u,char c)`);

    type('\n'   , c`> int fla(
                    >         int x,
                    >         short u,|char c)`

                , c`> int fla(
                    >         int x,
                    >         short u,
                    >         |char c)`);

    type('\n'   , c`> int fla(
                    >         int x,
                    >         short u,
                    >         char c|)`

                , c`> int fla(
                    >         int x,
                    >         short u,
                    >         char c
                    >         |)`);

    type('\n'   , c`> int fla(
                    >         int x,|short u,char c
                    >        )`

                , c`> int fla(
                    >         int x,
                    >         |short u,char c
                    >        )`);

    type('\n'   , c`> int fla(
                    >         int x,|short long_var_name,char c)`

                , c`> int fla(
                    >         int x,
                    >         |short long_var_name,char c)`);

    type('\n'   , c`> int fla(
                    >         int x,|short long_var_name,
                    >         char c)`

                , c`> int fla(
                    >         int x,
                    >         |short long_var_name,
                    >         char c)`);

    type('\n'   , c`> void flp() {
                    > }
                    >
                    > int fla(
                    >         int x,|short long_var_name,
                    >         char c)`

                , c`> void flp() {
                    > }
                    >
                    > int fla(
                    >         int x,
                    >         |short long_var_name,
                    >         char c)`);

    type('\n'   , c`> int x() {
                    > }
                    > int fla(
                    >         int x,|short u,char c
                    >        )`

                , c`> int x() {
                    > }
                    > int fla(
                    >         int x,
                    >         |short u,char c
                    >        )`);

    type('\n'   , c`> void x() {
                    > }
                    > int fla(
                    >         int x,
                    >         short u,
                    >         char c|)`

                , c`> void x() {
                    > }
                    > int fla(
                    >         int x,
                    >         short u,
                    >         char c
                    >         |)`);

    type('\n'   , c`> int x() {
                    > }
                    > int fla(
                    >         int x,
                    >         short u,|char c)`

                , c`> int x() {
                    > }
                    > int fla(
                    >         int x,
                    >         short u,
                    >         |char c)`);

    type('\n'   , c`> int b() {
                    > }
                    > int fla(
                    >         int x,|short u,char c)`

                , c`> int b() {
                    > }
                    > int fla(
                    >         int x,
                    >         |short u,char c)`);

    type('\n'   , c`> int b() {
                    > }
                    > int flablabberwabber(
                    >                      int lonng,|short lonngearr,char shrt)`

                , c`> int b() {
                    > }
                    > int flablabberwabber(
                    >                      int lonng,
                    >                      |short lonngearr,char shrt)`);

    type('\n'   , c`> int fla(
                    >   int x,
                    >   short u,
                    >   char c|)`

                , c`> int fla(
                    >   int x,
                    >   short u,
                    >   char c
                    > |)`);

    type('\n'   , c`> void x() {
                    > }
                    > int fla(int x,
                    >         short u,|)`

                , c`> void x() {
                    > }
                    > int fla(int x,
                    >         short u,
                    >         |)`);
});

testCase('ctyles:prep', () => {
    type('#'    , c`> int main() {
                    >   |
                    > }`

                , c`> int main() {
                    > #|
                    > }`);

    sequence(         c`>   int foo() {
                        >     x;
                        >     `, () => {

        type('#'    , c`>   int foo() {
                        >     x;
                        > #`);

        type('ifdef FLA\n'
                    , c`>   int foo() {
                        >     x;
                        > #ifdef FLA
                        >     `);
    });
});

testCase('ctyles:string', () => {
    sequence(         c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = "\\|
                        >   return 0;
                        > }`, () => {

        type('\n'   , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = "\\
                        > |
                        >   return 0;
                        > }`);

        type('hello";'
                    , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = "\\
                        > hello";|
                        >   return 0;
                        > }`);
    });

    config({
        syntax: "C++"
    });

    sequence(         c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = R"(|
                        >   return 0;
                        > }`, () => {

        type('\n'   , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = R"(
                        > |
                        >   return 0;
                        > }`);

        type('hello)";'
                    , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinebackslash = R"(
                        > hello)";|
                        >   return 0;
                        > }`);
    });

    sequence(         c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinestringliteral = R"(
                        >        bla bla|
                        >   )";
                        >   return 0;
                        > }`, () => {

        type('\n'   , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinestringliteral = R"(
                        >        bla bla
                        >        |
                        >   )";
                        >   return 0;
                        > }`);

        type('barbapapa'
                    , c`> int main(int argc, char const *argv[])
                        > {
                        >   const char * multilinestringliteral = R"(
                        >        bla bla
                        >        barbapapa|
                        >   )";
                        >   return 0;
                        > }`);
    });

    type('\n'   , c`> int main(int argc, char const *argv[])
                    > {
                    >   const char * multilinestringliteral = R"(
                    >        bla| bla
                    >   )";
                    >   return 0;
                    > }`

                , c`> int main(int argc, char const *argv[])
                    > {
                    >   const char * multilinestringliteral = R"(
                    >        bla
                    >        |bla
                    >   )";
                    >   return 0;
                    > }`);
});

testCase('ctyles:switch', () => {
    sequence(         c`>   int foo() {
                        >     switch (x) {`, () => {

        type('\n'   , c`>   int foo() {
                        >     switch (x) {
                        >       |`);

        type('case 0:'
                    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:|`);

        type('\n'   , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         |`);

        type('k;\n' , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         k;
                        >         |`);

        type('case 1:'
                    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         k;
                        >       case 1:|`);

        type(';\n'  , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         k;
                        >       case 1:;
                        >       |`);

        type('}'    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         k;
                        >       case 1:;
                        >     }|`);
    });

    sequence(         c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >         |`, () => {

        type('case 1:'
                    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >       case 1:|`);

        type(' // bla\n'
                    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >       case 1: // bla
                        >         |`);

        type('default:'
                    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >       case 1: // bla
                        >       default:`);

        type('\n'   , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >       case 1: // bla
                        >       default:
                        >         `);

        type(';'    , c`>   int foo() {
                        >     switch (x) {
                        >       case 0:
                        >       case 1: // bla
                        >       default:
                        >         ;`);
    });

    sequence(         c`>   int foo() {
                        >     switch (x) {
                        >       case '.':
                        >         ok;|`, () => {

        type('\n'   , c`>   int foo() {
                        >     switch (x) {
                        >       case '.':
                        >         ok;
                        >         |`);

        type('case ', c`>   int foo() {
                        >     switch (x) {
                        >       case '.':
                        >         ok;
                        >         case `);

        type("':"   , c`>   int foo() {
                        >     switch (x) {
                        >       case '.':
                        >         ok;
                        >       case ':`);

        type("':"   , c`>   int foo() {
                        >     switch (x) {
                        >       case '.':
                        >         ok;
                        >       case ':':`);
    });

    sequence(     c`>   int foo() {
                    >     switch (x) { // only first symbolic colon may reindent
                    >     case '0':`, () => {

        type(" case '1':"
                , c`>   int foo() {
                    >     switch (x) { // only first symbolic colon may reindent
                    >       case '0': case '1':`
                , 'test for case where cfgSwitchIndent = false; needs proper config-interface');

        type(" case '2':"
                , c`>   int foo() {
                    >     switch (x) { // only first symbolic colon may reindent
                    >       case '0': case '1': case '2':`
                , 'test for case where cfgSwitchIndent = false; needs proper config-interface');
    });

    sequence(         c`>   int foo() {
                        >     switch (x)`, () => {

        type('\n'   , c`>   int foo() {
                        >     switch (x)
                        >       |`);

        type('x--;' , c`>   int foo() {
                        >     switch (x)
                        >       x--;`);

        type('\n'   , c`>   int foo() {
                        >     switch (x)
                        >       x--;
                        >       |`);
    });

    type('\n'   , c`> int fla() {
                    >   switch (x) x();`

                , c`> int fla() {
                    >   switch (x) x();
                    >   |`);
});

sequence('cstyle:tabindent'
                , c`> int main()
                    > {
                    > \tif (true)
                    > \t{
                  --> \t\twhile(true)|
                    > \t}
                    > }`, () => {
    config({
        replaceTabs: false,
        autoBrackets: true,
    });

    type('\n'   , c`> int main()
                    > {
                    > \tif (true)
                    > \t{
                    > \t\twhile(true)
                  --> \t\t\t|
                    > \t}
                    > }`);

    type('{'    , c`> int main()
                    > {
                    > \tif (true)
                    > \t{
                    > \t\twhile(true)
                  --> \t\t{|}
                    > \t}
                    > }`);
});

testCase('cstyle:top', () => {
    for (const before of [
        '',
        'should always indent after opening brace\n',
        ';\n',
        ':\n',
        '{\n',
        '}\n',
        '(\n',
        ')\n',
        'n\n',
    ]) {
        type('\n'   , `${before}int {`
                    , `${before}int {\n  |`);

        type('\n'   , `${before}\nint {`
                    , `${before}\nint {\n  |`);
    }

    type('\n'   , '// leading comment should not cause second line to be indented'
                , '// leading comment should not cause second line to be indented\n');
});

testCase('cstyle:using', () => {
    xtype('\n'  , 'using'
                , 'using\n  '
                , 'this is insane, those who write such code can cope with it :P');

    xtype('\n'  , 'using\n  std::vector;'
                , 'using\n  std::vector;\n'
                , 'this is insane, those who write such code can cope with it :P');

    type('\n'   , 'using std::vector;'
                , 'using std::vector;\n');
});

testCase('cstyle:visib', () => {
    config({
        syntax: 'C++',
    });

    sequence(         c`> class A {
                        >   public:`, () => {

        type('\n'   , c`> class A {
                        >   public:
                        >     |`);

        type('A();\n',c`> class A {
                        >   public:
                        >     A();
                        >     |`);

        xtype('protected:'
                    , c`> class A {
                        >   public:
                        >     A();
                        >   protected:`

                    , 'test for access modifier where cfgAccessModifiers = 1;needs proper config interface');
    });

    xtype('protected:'
                , c`> class A {
                    >   public:
                    >     |`

                , c`> class A {
                    >   public:
                    >   protected:`

                , 'test for access modifier where cfgAccessModifiers = 1;needs proper config interface');

    withInput(            c`> class A {
                            >              public:`, () => {
        for (const after of [
            ' // :',
            ' x(":");',
            " x(':');",
            ' X::x();',
            ' private:',
        ]) {
            type(after  , c`> class A {
                            >              public:${after}`);
        }
    });

});

testCase('cstyle:while', () => {
    sequence(         c`> int fla() {
                        >   while (0<x)`, () => {

        type('\n'   , c`> int fla() {
                        >   while (0<x)
                        >     |`);

        type('x--;\n',c`> int fla() {
                        >   while (0<x)
                        >     x--;
                        >   |`);
    });

    type('\n'   , c`> int fla() {
                    >   while (0<x) x();`

                , c`> int fla() {
                    >   while (0<x) x();
                    >   |`);
});
