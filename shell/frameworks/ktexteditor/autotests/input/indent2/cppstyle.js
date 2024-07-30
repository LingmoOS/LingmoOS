config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'ISO C++',
    indentationMode: 'cppstyle',
    selection: '',
});

testCase('cppstyle:autobrackets', () => {
    config({autoBrackets: true});

    sequence(         c`> class X
                        > {
                        >     void foo()|
                        > };`, () => {

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     |
                        > };`);

        type('{'    , c`> class X
                        > {
                        >     void foo()
                        >     {|}
                        > };`);

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         |
                        >     }
                        > };`);
    });

    sequence(         c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)|
                        > };`, () => {

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >             |
                        > };`);

        type('{'    , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {|}
                        > };`);

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >             |
                        >         }
                        > };`);
    });

    sequence(         c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >             `, () => {

        type('}'    , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >         }`);

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >         }
                        >         `);

        type('}'    , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >         }
                        >     }`);

        type('\n'   , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >         }
                        >     }
                        >     `);

        type('}'    , c`> class X
                        > {
                        >     void foo()
                        >     {
                        >         if (true)
                        >         {
                        >         }
                        >     }
                        > };`);
    });
});

testCase('cppstyle:colon', () => {
    type(':', 'class test |', 'class test : |');
    type(':', 'class test\n|', 'class test\n  : |');
    type(':', 'for (auto i |', 'for (auto i : |');
    type(':', ';ec|', ';ec:|');
    type(':', 'case some|', 'case some:|');

    sequence(      'some<T>|', () => {
        type(':', 'some<T>::|');
        type(':', 'some<T>::|');
    });

    sequence(      'std|', () => {
        type(':', 'std::|');
        type(':', 'std::|');
    });

    for (const kw of ['public', 'protected', 'private', 'public slots', 'private Q_SLOTS']) {
        type(`${kw}:`   , c`> class test
                            > {
                            >     |
                            > };`

                        , c`> class test
                            > {
                            > ${kw}:|
                            > };`);

        type(`${kw}:`   , c`> class test
                            > {
                            >     void ok();
                            >     |
                            > };`

                        , c`> class test
                            > {
                            >     void ok();
                            >
                            > ${kw}:|
                            > };`);

        type(`${kw}:`   , c`>     class test
                            >     {
                            >         |
                            >     };`

                        , c`>     class test
                            >     {
                            >     ${kw}:|
                            >     };`);

        type(`${kw}:`   , c`>     class test
                            >     {
                            >         void ok();
                            >         |
                            >     };`

                        , c`>     class test
                            >     {
                            >         void ok();
                            >
                            >     ${kw}:|
                            >     };`);
    }
});

testCase('cppstyle:comma', () => {
    type(',', 'some|', 'some, |');
    type(',', 'some|, ok', 'some, |, ok');
    type(',', 'some |', 'some, |');
    type(',', 'some |', 'some, |');
    type(',', 'some | other', 'some, | other');
    type(',', 'some, ok\nsome |', 'some, ok\nsome, |');

    type(',', 'foo(ok|);\n', 'foo(ok, |);\n');

    type(','    , c`>     foo(
                    >         ok
                    >         |
                    >       );`

                , c`>     foo(
                    >         ok
                    >       , |
                    >       );`);

    sequence(         c`> class test
                        >   : public base`, () => {

        type('\n'   , c`> class test
                        >   : public base
                        >   `);

        type(','    , c`> class test
                        >   : public base
                        >   , `);
    });
});

testCase('cppstyle:comment.single', () => {
    sequence(     '', () => {
        type('/', '/');
        type('/', '// ');
        type('/', '/// ');
        type('/', '////');
        type('/', '/////');
    });

    sequence(     '    ', () => {
        type('/', '    /');
        type('/', '    // ');
        type('/', '    /// ');
        type('/', '    ////');
        type('/', '    /////');
    });

    sequence(     'const char* tmp = "', () => {
        type('/', 'const char* tmp = "/');
        type('/', 'const char* tmp = "//');
        type('/', 'const char* tmp = "///');
        type('/', 'const char* tmp = "////');
    });

    sequence(       '|// some', () => {
        type('ok',  'ok|// some');
        type(' ',   'ok |                                                         // some');
        type('ok;', 'ok ok;|                                                      // some');
    });

    sequence(     'int smth;', () => {
        type('/', 'int smth;/');
        type('/', 'int smth;                                                   // ');
        type('/', 'int smth;                                                   ///< ');
    });

    type('/',     'int smth;                                                   //'
            ,     'int smth;                                                   ///< ');

    sequence(     'int a = 12;    ', () => {
        type('/', 'int a = 12;    /');
        type('/', 'int a = 12;                                                 // ');
    });

    sequence(             'some very very looooooooooooooooooooooooooooooooooong line | // some', () => {
        type('ok'   ,     'some very very looooooooooooooooooooooooooooooooooong line ok| // some');
        type(' '    , c`> // some\n'
                        > some very very looooooooooooooooooooooooooooooooooong line ok |`);
    });

    sequence(               'auto i = |                                                   // some', () => {
        type('ok()', 'auto i = ok()|                                               // some');
        type('==',   'auto i = ok() == |                                              // some');
        type('ok',   'auto i = ok() == ok|                                              // some');
        type('(',    'auto i = ok() == ok(|                                        // some');
        type(')',    'auto i = ok() == ok()|                                       // some');
    });

    type('@'    ,    '    // '

                , c`>     //@{
                    >     |
                    >     //@}`);
});

testCase('cppstyle:comment.multi', () => {
    sequence(          '/*\n * |\n*/', () => {
        type('/', '/*\n * /|\n*/');
        type('/', '/*\n * //|\n*/');
        type('/', '/*\n * ///|\n*/');
        type('/', '/*\n * ////|\n*/');
    });

    type('\n', '/* some */|', '/* some */\n|');

    type('\n', c`>     /* some| */`

             , c`>     /* some
                >      * |
                >      */`);

    type('\n'   , c`> /*
                    >      *
                    >      */|`

                , c`> /*
                    >      *
                    >      */
                    >     |`);

    sequence(         c`>     /*
                        >      *
                        >      */
                        >     |class some;`, () => {

        type('/'    , c`>     /*
                        >      *
                        >      */
                        >     /|class some;`);

        type('/'    , c`>     /*
                        >      *
                        >      */
                        >     //|class some;`);
    });

    sequence(            '/**|', () => {

        type('\n'   , c`> /**
                        >  * |
                        >  */`);

        type('\n'   , c`> /**
                        >  * ${''}
                        >  * |
                        >  */`);
    });

    sequence(            '    /**|', () => {

        type('\n'   , c`>     /**
                        >      * |
                        >      */`);

        type('\n'   , c`>     /**
                        >      * ${''}
                        >      * |
                        >      */`);
    });

    type('\n'   , c`> /**
                    >  * o|k
                    >  */`

                , c`> /**
                    >  * o
                    >  * |k
                    >  */`);
});

testCase('cppstyle:do', () => {
    type('\n', 'do', 'do\n    |');
});

testCase('cppstyle:dot', () => {
    type('.', 'some()', 'some().');

    sequence(     'catch', () => {
        type(' ', 'catch (|)');
        type('.', 'catch (...)|');
    });

    sequence(     'template <typename| Args>', () => {
        type('.', 'template <typename.| Args>');
        type('.', 'template <typename...| Args>');
        type('.', 'template <typename...| Args>');
    });

    sequence(     'some(const char* const fmt, Args| args)', () => {
        type('.', 'some(const char* const fmt, Args.| args)');
        type('.', 'some(const char* const fmt, Args...| args)');
        type('.', 'some(const char* const fmt, Args...| args)');
    });

    sequence(     '    auto sz = sizeof|(args);', () => {
        type('.', '    auto sz = sizeof.|(args);');
        type('.', '    auto sz = sizeof...|(args);');
        type('.', '    auto sz = sizeof...|(args);');
    });

    sequence(     '    return some(sz, std::forward<Args>(args)|);', () => {
        type('.', '    return some(sz, std::forward<Args>(args).|);');
        type('.', '    return some(sz, std::forward<Args>(args)...|);');
        type('.', '    return some(sz, std::forward<Args>(args)...|);');
    });

    sequence(     'some(const char* const fmt, |)', () => {
        type('.', 'some(const char* const fmt, .|)');
        type('.', 'some(const char* const fmt, ...|)');
        type('.', 'some(const char* const fmt, ...|)');
    });

    sequence(      '    ok', () => {
        type('.',  '    ok.');
        type('ok', '    ok.ok');
        type('.',  '    ok.ok.');
        type('.',  '    ok.ok...');
        type('.',  '    ok.ok...');
    });

    // bug 333696
    sequence(     '#include <|>', () => {
        type('.', '#include <.|>');
        type('.', '#include <..|>');
        type('/', '#include <../|>');
    });

    // bug 333696
    sequence(     'auto literal = "some | text";', () => {
        type('.', 'auto literal = "some .| text";');
        type('.', 'auto literal = "some ..| text";');
        type('.', 'auto literal = "some ...| text";');
    });

    // bug 333696
    sequence(     '// comment', () => {
        type('.', '// comment.');
        type('.', '// comment..');
        type('.', '// comment...');
    });
});

testCase('cppstyle:equal', () => {
    type('\n'   ,    'int a ='
                , c`> int a =
                    >     |`);

    type('\n'   ,    'int a| ='
                , c`> int a
                    >   |=`);

    sequence(     'a', () => {
        type('=', 'a = ');
        type('=', 'a == ');
    });

    sequence(     'a', () => {
        type('!', 'a !');
        type('=', 'a != ');
    });

    sequence(     'a', () => {
        type('*', 'a*');
        type('=', 'a *= ');
    });

    sequence(     'a', () => {
        type('/', 'a/');
        type('=', 'a /= ');
    });

    sequence(     'a', () => {
        type('%', 'a % ');
        type('=', 'a %= ');
    });

    config({cursor: ''});
    sequence(     'a', () => {
        type('|', 'a | ');
        type('=', 'a |= ');
    });
    config({cursor: '|'});

    sequence(     'a', () => {
        type('&', 'a&');
        type('=', 'a &= ');
    });

    sequence(     'a', () => {
        type('^', 'a ^ ');
        type('=', 'a ^= ');
    });

    sequence(     'a', () => {
        type('<', 'a<|>');
        type('=', 'a <= ');
    });

    sequence(     'a', () => {
        type('>', 'a>');
        type('=', 'a >= ');
    });

    sequence(     'a', () => {
        type('+', 'a+');
        type('=', 'a += ');
    });

    sequence(     'a', () => {
        type('+', 'a+');
        type('+', 'a++');
        type('=', 'a++ = ');
    });

    sequence(     'a', () => {
        type('+', 'a+');
        type('+', 'a++');
        type('+', 'a+++');
        type('=', 'a++ += ');
    });

    sequence(     'a', () => {
        type('-', 'a-');
        type('=', 'a -= ');
    });

    sequence(     'a', () => {
        type('-', 'a-');
        type('-', 'a--');
        type('=', 'a-- = ');
    });

    sequence(     'a', () => {
        type('-', 'a-');
        type('-', 'a--');
        type('-', 'a---');
        type('=', 'a-- -= ');
    });
});

testCase('cppstyle:exclamation', () => {
    type('!'    , '    '
                , '    !');

    type('!'    , '    if ('
                , '    if (!');

    type('!'    , '    if (!ok &&'
                , '    if (!ok && !');

    type('!'    , '    foo<'
                , '    foo<!');

    type('!'    , '    foo <'
                , '    foo < !');
});

testCase('cppstyle:for', () => {
    sequence(            '    for', () => {
        type('('    ,    '    for (');
        type('\n'   , c`>     for (
                        >         |`);
    });

    sequence(         c`>     for (
                        >         int i = 0`, () => {

        type('\n'   , c`>     for (
                        >         int i = 0
                        >         `);

        type(';'    , c`>     for (
                        >         int i = 0
                        >       ; `);
    });
});

testCase('cppstyle:lab', () => {
    type('<', 'static_cast', 'static_cast<|>');
    type('<', 'template ', 'template <|>');
    type('<', 'operator<', 'operator<<(|)');
    type('<', 'some|,', 'some<|>,');
    type('<', 'smth ', 'smth <');
    type('<', '1smth', '1smth<');
    type('<', '// some', '// some<');
    type('std::map<', '|std::string', 'std::map<|std::string');

    xtype('<', 'some<', 'some << ');

    sequence(      'some', () => {
        type('<',  'some<|>');
        type('<',  'some << ');
    });

    sequence(      'std::cout', () => {
        type('<',  'std::cout<|>');
        type('<',  'std::cout << ');
    });
});

testCase('cppstyle:normal', () => {
    sequence(         c`> int main() {
                        >     bla;`, () => {

        type('\n'   , c`> int main() {
                        >     bla;
                        >     `);

        type('ok;'  , c`> int main() {
                        >     bla;
                        >     ok;`);
    });

    sequence(      'const char* tmp = "', () => {
        type('{',  'const char* tmp = "{');
        type('ok', 'const char* tmp = "{ok');
        type('}',  'const char* tmp = "{ok}');
        type('"',  'const char* tmp = "{ok}"');
        type(';',  'const char* tmp = "{ok}";');
    });

    sequence(      '///', () => {
        type('{',  '///{');
        type('ok', '///{ok');
        type('}',  '///{ok}');
    });

    type('{"ok"};'  , 'auto s = std::string|\n'
                    , 'auto s = std::string{"ok"};|\n');

    type('\n'   , 'namespace {'
                , 'namespace {\n');

    type('\n'   , c`> int main() {
                    >     bla;|blu;`

                , c`> int main() {
                    >     bla;
                    >     |blu;`);

    type('\n'  ,  c`>     class some
                    >     {
                    >         void foo();
                    >         ${''}
                    >     |public:`

                , c`>     class some
                    >     {
                    >         void foo();
                    >         ${''}
                    >     ${''}
                    >     |public:`);

    type('\n'   , c`>     foo(
                    >         bar
                    >       );|`

                , c`>     foo(
                    >         bar
                    >       );
                    >     |`);

    type('\n'  , c`>     foo({
                    >         bar
                    >       });|`

                , c`>     foo({
                    >         bar
                    >       });
                    >     |`);

    type('\n'   , c`>     typedef foo<
                    >         bar
                    >       > some;|`

                , c`>     typedef foo<
                    >         bar
                    >       > some;
                    >     |`);

    type('\n'   , c`>     for (auto i : container)
                    >         if (some)
                    >             break;|`

                , c`>     for (auto i : container)
                    >         if (some)
                    >             break;
                    >         |`);

    type('\n'   , c`>     for (auto i : container)
                    >         if (some)
                    >             break;
                    >         else if (other)
                    >             continue;|`

                , c`>     for (auto i : container)
                    >         if (some)
                    >             break;
                    >         else if (other)
                    >             continue;
                    >         |`);

    type('\n'   , c`> switch (some)
                    > {
                    >     case 1:
                    >         break;|`

                , c`> switch (some)
                    > {
                    >     case 1:
                    >         break;
                    >     |`);

    type('\n'   , c`>     if (cond)
                    >         do_smth();
                    >         else|`

                , c`>     if (cond)
                    >         do_smth();
                    >     else
                    >         |`);

    type('\n'   , c`>     if (some)
                    >     {
                    >         some;
                    >     }
                    >     else|`

                , c`>     if (some)
                    >     {
                    >         some;
                    >     }
                    >     else
                    >         |`);

    type('\n'   ,    `    return EXIT_SUCCESS;`
                , c`>     return EXIT_SUCCESS;
                    >     `);

    type('\n'   ,    '    return|'
                , c`>     return
                    >         |`);

    type('\n'   ,    '    auto some = []()|'

                , c`>     auto some = []()
                    >     {
                    >         |
                    >     };`);

    type('\n'   , c`>         std::foreach(
                    >             begin(c)
                    >           , end(c)
                    >           , [=](const value_type& v)|
                    >           );`

                , c`>         std::foreach(
                    >             begin(c)
                    >           , end(c)
                    >           , [=](const value_type& v)
                    >             {
                    >                 |
                    >             }
                    >           );`);

    type('\n'   , c`> int main() {
                    >     bla;|  blu;`

                , c`> int main() {
                    >     bla;
                    >     |blu;`);

    type('}'    , c`>     if (smth)
                    >     {
                    >         |`

                , c`>     if (smth)
                    >     {
                    >     }|`);

    sequence(         c`>     if (smth)
                        >     {
                        >         smth_else();
                        >     |`, () => {

        type('}'    , c`>     if (smth)
                        >     {
                        >         smth_else();
                        >     }|`);

        type('\n'   , c`>     if (smth)
                        >     {
                        >         smth_else();
                        >     }
                        >     |`);
    });

    sequence(         c`>     for (
                        >         int i = 0
                        >       ; i < 123
                        >       ; ++i
                        >       )|`, () => {

        type('\n'   , c`>     for (
                        >         int i = 0
                        >       ; i < 123
                        >       ; ++i
                        >       )
                        >       |`);

        type('{'    , c`>     for (
                        >         int i = 0
                        >       ; i < 123
                        >       ; ++i
                        >       )
                        >     {|`);

        type('\n'   , c`>     for (
                        >         int i = 0
                        >       ; i < 123
                        >       ; ++i
                        >       )
                        >     {
                        >         |`);
    });

    sequence(        'int a[123] =|', () => {
        type('{',    'int a[123] = {|');
        type('ok};', 'int a[123] = {ok};|');
    });

    sequence(     'enum test|', () => {
        type('{', 'enum test {|');
        type('}', 'enum test {};|');
    });

    type('}'    , c`> class test
                    > {
                    > |`

                , c`> class test
                    > {
                    > };|`);
});

testCase('cppstyle:parens', () => {
    sequence(            '    foo(', () => {
        type('\n'   , c`>     foo(
                        >         |`);
        type(')'    , c`>     foo(
                        >       )|`);
    });
});

testCase('cppstyle:pipe', () => {
    config({cursor: ''});

    type('|', '0  some'
            , '0  some | ');

    type('|', '1  some '
            , '1  some | ');

    type('|', '2  some|'
            , '2  some || ');

    type('|', '3  some |'
            , '3  some || ');

    type('|', '4  some| '
            , '4  some || ');

    type('|', '5  some | '
            , '5  some || ');

    type('|', '6a some|| '
            , '6a some || ');

    type('|', '6c some||'
            , '6c some || ');

    type('|', '6b some ||'
            , '6b some || ');
});

testCase('cppstyle:plist', () => {
    sequence(          '    foo(|);', () => {

        type('\n' , c`>     foo(
                      >         |
                      >       );`);

        type('ok' , c`>     foo(
                      >         ok|
                      >       );`);

        type('\n' , c`>     foo(
                      >         ok
                      >         |
                      >       );`);
    });

    type('\n'   ,    'typedef boost::mpl::eval_if<|>'
                , c`> typedef boost::mpl::eval_if<
                    >     |
                    >   >`);
});

testCase('cppstyle:preprocessor', () => {
    sequence(                 '    ', () => {
        type('#' ,        '#');
        type('ifdef' ,    '#ifdef');
        type(' ' ,        '#ifdef ');
        type('NDEBUG' ,   '#ifdef NDEBUG');
        type('\n' ,       '#ifdef NDEBUG\n');
        type('    ' ,     '#ifdef NDEBUG\n    ');
        type('#' ,        '#ifdef NDEBUG\n# ');
        type('define' ,   '#ifdef NDEBUG\n# define');
        type(' ' ,        '#ifdef NDEBUG\n# define ');
        type('OK' ,       '#ifdef NDEBUG\n# define OK');
    });

    sequence(         c`> #ifdef NDEBUG
                        > # ifdef linux
                        > |
                        > # endif
                        > #endif`, () => {

        type('#'    , c`> #ifdef NDEBUG
                        > # ifdef linux
                        > #   |
                        > # endif
                        > #endif`);
        type('define OK'
                    , c`> #ifdef NDEBUG
                        > # ifdef linux
                        > #   define OK|
                        > # endif
                        > #endif`);
    });

    sequence(         '#define MACRO', () => {
        type('\\'   , '#define MACRO \\');
        type('\n'   , c`> #define MACRO \\
                        >     `);
        type('ok'   , c`> #define MACRO \\
                        >     ok`);
        type('\\'   , c`> #define MACRO \\
                        >     ok        \\`);
        type('\n'   , c`> #define MACRO \\
                        >     ok        \\
                        >     `);
    });

    sequence(         c`> #define MACRO \\
                        >     define smth long`, () => {

        type('\\'   , c`> #define MACRO        \\
                        >     define smth long \\`);

        type('\n'   , c`> #define MACRO        \\
                        >     define smth long \\
                        >     `);
    });
});

testCase('cppstyle:quote', () => {
    type('"', 'auto a = "R|'
            , 'auto a = "R"|');

    type('"', 'auto a = R|'
            , 'auto a = R"~(|)~"');

    type('"', 'auto a = u8R|'
            , 'auto a = u8R"~(|)~"');

    type('"', 'LR|'
            , 'LR"~(|)~"');

    type('"', 'auto a = |'
            , 'auto a = "|"');

    type('"', 'auto b = |;'
            , 'auto b = "|";');

    type('"', 'auto c = some(|);'
            , 'auto c = some("|");');
});

testCase('cppstyle:semicolon', () => {
    type(';'    , c`> for (blah)
                    > break|`

                , c`> for (blah)
                    >     break;|`);

    type(';'    , c`> for (blah)
                    >     if (some)
                    > continue|`

                , c`> for (blah)
                    >     if (some)
                    >         continue;|`);

    type(';'    , c`>     switch (blah)
                    >     {
                    >     case 1:
                    >     break|`

                , c`>     switch (blah)
                    >     {
                    >     case 1:
                    >         break;|`);

    type(';'    , 'auto var = some_call(arg1, arg2|)\n'
                , 'auto var = some_call(arg1, arg2);|\n');

    type(';'    , 'auto var = some_call(arg1, arg2|);\n'
                , 'auto var = some_call(arg1, arg2;|);\n');

    type(';'    , 'for (auto it = begin(list)|)\n'
                , 'for (auto it = begin(list);|)\n');

    type(';'    , 'some      |\n'
                , 'some;|      \n');
    type(';'    , 'some;|      \n'
                , 'some;|      \n');

    type(';'    , 'some[call(|)]\n'
                , 'some[call()];|\n');
});

testCase('cppstyle:shortcut', () => {
    withInput(    'some(|)\n', () => {
        type(';', 'some();|\n');
        type('^', 'some() ^ |\n');
        type('?', 'some() ? |\n');
        type(',', 'some(), |\n');
        type('>', 'some()>|\n');
        type('<', 'some() <|\n');
        type('.', 'some().|\n');
        type(']', 'some()]|\n');
    });

    withInput(    'some{|}\n', () => {
        type(';', 'some{;|}\n'); // no shortcut
        type('^', 'some{} ^ |\n');
        type('?', 'some{} ? |\n');
        type(',', 'some{}, |\n');
        type('>', 'some{}>|\n');
        type('<', 'some{} <|\n');
        type('.', 'some{}.|\n');
        type(']', 'some{}]|\n');
    });

    type('%', 'some|\n',     'some % |\n');
    type('%', 'some |\n',    'some % |\n');
    type('%', 'some%|\n',    'some % |\n');
    type('%', 'some% |\n',   'some % |\n');
    type('%', 'some %|\n',   'some % |\n');
    type('%', 'some % |\n',  'some % |\n');
});

testCase('cppstyle:space', () => {
    type(' ', 'return',  'return |;');
    type(' ', 'some<|>',  'some < |');
});

testCase('cppstyle:string', () => {
    type('\n'   ,    'auto some = "|some really long text here we wanted to split";'
                , c`> auto some = ""
                    >     "|some really long text here we wanted to split";`);

    type('\n'   ,    'auto some = "some really long text |here we wanted to split";'
                , c`> auto some = "some really long text "
                    >     "|here we wanted to split";`);

    type('\n'   , c`> auto some = "some really long text "
                    >     "here we wanted |to split";`

                , c`> auto some = "some really long text "
                    >     "here we wanted "
                    >     "|to split";`);

    type('\n'   ,    'auto some = "some really long text here we wanted to split|";'

                , c`> auto some = "some really long text here we wanted to split"
                    >     "|";`);

    type('\n'   ,    'auto some = |"some really long text here we wanted to split";'

                , c`> auto some = ${''}
                    >     |"some really long text here we wanted to split";`);


    type('\n'   , c`> auto some =
                    >     "some really long text here we wanted to split"|;`

                , c`> auto some =
                    >     "some really long text here we wanted to split"
                    >     |;`);

    type('\n'   ,    'auto empty = "|";'
                , c`> auto empty = ""
                    >     "|";`);

    type('\n'   ,    '/// \\name Some|'
                , c`> /// \\name Some
                    > |`);

    type('\n'   ,    '// ATTENTION Some|'
                , c`> // ATTENTION Some
                    > |`);
});

testCase('cppstyle:ternary_op', () => {
    sequence(         c`>     foo(
                        >         test
                        >         |
                        >       );`, () => {

        type('?'   ,  c`>     foo(
                        >         test
                        >       ? |
                        >       );`);

        type('ok\n', c`>     foo(
                        >         test
                        >       ? ok
                        >       |
                        >       );`);

        type(':'   ,  c`>     foo(
                        >         test
                        >       ? ok
                        >       : |
                        >       );`);
    });

    type('?'    , 'int a = b|'
                , 'int a = b ? |');
});
