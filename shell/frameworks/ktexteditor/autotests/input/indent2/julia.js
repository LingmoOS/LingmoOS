config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'Julia',
    indentationMode: 'julia',
    selection: '',
});

testCase('julia:beginblock', () => {
    type('\n'   , 'z = begin x = 1; y = 2 end'
                , 'z = begin x = 1; y = 2 end\n');

    sequence(            'z = begin # testcomment abcde', () => {

        type('\n'   , c`> z = begin # testcomment abcde
                        >     `);

        type('x '   , c`> z = begin # testcomment abcde
                        >     x `);

        type('= '   , c`> z = begin # testcomment abcde
                        >     x = `);

        type('1\n'  , c`> z = begin # testcomment abcde
                        >     x = 1
                        >     `);

        type('y = 2\n'
                    , c`> z = begin # testcomment abcde
                        >     x = 1
                        >     y = 2
                        >     `);

        type('e'    , c`> z = begin # testcomment abcde
                        >     x = 1
                        >     y = 2
                        >     e`);

        type('nd'   , c`> z = begin # testcomment abcde
                        >     x = 1
                        >     y = 2
                        > end`);
    });

    // TODO
    // sequence(            'z = begin\n    ', () => {
    //
    //     type('end'  , c`> z = begin
    //                     >     end`
    //                 , 'not determined between keyword or start of identifier');
    //
    //     type('i = 0\n'
    //                 , c`> z = begin
    //                     >     endi = 0
    //                     >     `);
    //
    //     type('end'  , c`> z = begin
    //                     >     endi = 0
    //                     >     end`
    //                 , 'not determined between keyword or start of identifier');
    //
    //     type(' '    , c`> z = begin
    //                     >     endi = 0
    //                     > end `);
    // });
    //
    // type('\n'   , c`> z = begin
    //                 >     endi = 0
    //                 >     end`
    //             , c`> z = begin
    //                 >     endi = 0
    //                 > end
    //                 > `
    //             , 'not determined between keyword or start of identifier');
});

sequence('julia:doblock'
                   , 'open("test.txt", "r") do f # testcomment abcde', () => {

    type('\n'   , c`> open("test.txt", "r") do f # testcomment abcde
                    >     `);

    type('read(f, String)\n'
                , c`> open("test.txt", "r") do f # testcomment abcde
                    >     read(f, String)
                    >     `);

    type('end'  , c`> open("test.txt", "r") do f # testcomment abcde
                    >     read(f, String)
                    > end`);
});

sequence('julia:if', 'if x && y && z # abctestcomment', () => {

    type('\n'   , c`> if x && y && z # abctestcomment
                    >     `);

    type('z = 1\n'
                , c`> if x && y && z # abctestcomment
                    >     z = 1
                    >     `);

    type('end'  , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > end`);
});

sequence('julia:else'
                , c`> if x && y && z # abctestcomment
                    >     z = 1
                    >     `, () => {

    type('else' , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > else`);

    type('\n'   , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > else
                    >     `);

    type('z = 2\n'
                , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > else
                    >     z = 2
                    >     `);

    type('end'  , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > else
                    >     z = 2
                    > end`);
});

sequence('julia:elseif'
                , c`> if x && y && z # abctestcomment
                    >     z = 1
                    >     `, () => {

    type('else' , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > else`);

    type('if '  , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > elseif `);

    type('a\n'  , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > elseif a
                    >     `);

    type('z = 2\n'
                , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > elseif a
                    >     z = 2
                    >     `);

    type('end'  , c`> if x && y && z # abctestcomment
                    >     z = 1
                    > elseif a
                    >     z = 2
                    > end`);
});

sequence('julia:function_args'
                   , 'function test2(arg1,  # opening function w/ arguments indents to opening', () => {

    type('\n'   , c`> function test2(arg1,  # opening function w/ arguments indents to opening
                    >                `);

    type('arg2)\n'
                , c`> function test2(arg1,  # opening function w/ arguments indents to opening
                    >                arg2)
                    >     `);

    type('return true\n'
                , c`> function test2(arg1,  # opening function w/ arguments indents to opening
                    >                arg2)
                    >     return true
                    >     `);

    type('end'  , c`> function test2(arg1,  # opening function w/ arguments indents to opening
                    >                arg2)
                    >     return true
                    > end`);
});

testCase('julia:indentComment', () => {
    type('\n'   , 'function some_function()  # comment'
                , 'function some_function()  # comment\n    ');
});

testCase('julia:indentpaste', () => {
    cmd([paste
        , c`> if i % 2 == 0
            >     println("even")
            > else
            >     println("odd")
            > end`
        ]
        , c`> function test()
            >     for i in 1:50
            >         |
            >     end
            > end`

        , c`> function test()
            >     for i in 1:50
            >         if i % 2 == 0
            >             println("even")
            >         else
            >             println("odd")
            >         end|
            >     end
            > end`);

    cmd([paste
        , c`>  function hypot(x,y)
            >            x = abs(x)
            >            y = abs(y)
            >            if x > y
            >                r = y/x
            >                return x*sqrt(1+r*r)
            >            end
            >            if y == 0
            >                return zero(x)
            >            end
            >            r = x/y
            >            return y*sqrt(1+r*r)
            >            end`
        ]
        , c`> function test()
            >     for i in 1:50
            >         |
            >     end
            > end`

        , c`> function test()
            >     for i in 1:50
            >         function hypot(x,y)
            >             x = abs(x)
            >             y = abs(y)
            >             if x > y
            >                 r = y/x
            >                 return x*sqrt(1+r*r)
            >             end
            >             if y == 0
            >                 return zero(x)
            >             end
            >             r = x/y
            >             return y*sqrt(1+r*r)
            >         end|
            >     end
            > end`);
});

testCase('julia:keepindent', () => {
    sequence(         c`> function some_function(param, param2)
                        >     a = 5
                        >     b = 7|`, () => {

        type('\n'   , c`> function some_function(param, param2)
                        >     a = 5
                        >     b = 7
                        >     `)

        type('c = true'
                    , c`> function some_function(param, param2)
                        >     a = 5
                        >     b = 7
                        >     c = true`)
    });

    sequence(         c`> struct TestStruct
                        >     function TestSTruct()
                        >         print("Foo")
                        >         print(3)`, () => {

        type('\n'   , c`> struct TestStruct
                        >     function TestSTruct()
                        >         print("Foo")
                        >         print(3)
                        >         `)

        type('c = true'
                    , c`> struct TestStruct
                        >     function TestSTruct()
                        >         print("Foo")
                        >         print(3)
                        >         c = true`)
    });

    sequence(         c`> while true:
                        >     returnFunc()
                        >     myVar = 3`, () => {

        type('\n'   , c`> while true:
                        >     returnFunc()
                        >     myVar = 3
                        >     `)

        type('c = true'
                    , c`> while true:
                        >     returnFunc()
                        >     myVar = 3
                        >     c = true`)
    });

    sequence(         c`> function test(x)
                        >     x *= 5
                        >     for i in 1:10 x += i end`, () => {

        type('\n'   , c`> function test(x)
                        >     x *= 5
                        >     for i in 1:10 x += i end
                        >     `)

        type('y = x / 5'
                    , c`> function test(x)
                        >     x *= 5
                        >     for i in 1:10 x += i end
                        >     y = x / 5`)
    });

    sequence(         c`> using ArgParse
                        > s = ArgParseSettings()
                        > @add_arg_table s begin
                        >     "arg1"
                        >         help = "helpstring"`, () => {

        type('\n'   , c`> using ArgParse
                        > s = ArgParseSettings()
                        > @add_arg_table s begin
                        >     "arg1"
                        >         help = "helpstring"
                        >         `)

        type('required = true'
                    , c`> using ArgParse
                        > s = ArgParseSettings()
                        > @add_arg_table s begin
                        >     "arg1"
                        >         help = "helpstring"
                        >         required = true`)
    });
});

testCase('julia:list', () => {
    sequence(            'mylist = [1,  # opening list w/ out elements should indent once', () => {

        type('\n'   , c`> mylist = [1,  # opening list w/ out elements should indent once
                        >           `)

        type('2]\n' , c`> mylist = [1,  # opening list w/ out elements should indent once
                        >           2]
                        > `)
    });

    sequence(            'mylist = [1,  # opening list w/ elements indents to opening', () => {

        type('\n'   , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           `)

        type('2,\n' , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,
                        >           `)

        type('3]\n' , c`> mylist = [1,  # opening list w/ elements indents to opening
                        >           2,
                        >           3]
                        > `)
    });
});

testCase('julia:nested', () => {
    sequence(         c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")`, () => {

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >             `)

        type('else' , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else`)

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             `)

        type('nothing\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >             `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end`)

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         `)

        type('print(i)\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         print(i)
                        >         `)

        type('print(i^2)\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         print(i)
                        >         print(i^2)
                        >         `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         print(i)
                        >         print(i^2)
                        >     end`)

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         print(i)
                        >         print(i^2)
                        >     end
                        >     `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         else
                        >             nothing
                        >         end
                        >         print(i)
                        >         print(i^2)
                        >     end
                        > end`)
    });

    sequence(         c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f`, () => {

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             `)

        type('file = read(f, String)\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             `)

        type('print(file)\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >             `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end`)

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         `)

        type('print("TEST2")\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         print("TEST2")
                        >         `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         print("TEST2")
                        >     end`)

        type('\n'   , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         print("TEST2")
                        >     end
                        >     `)

        type('print(a)\n'
                    , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         print("TEST2")
                        >     end
                        >     print(a)
                        >     `)

        type('end'  , c`> if x && y && z # abctestcomment
                        >     z = 1
                        >     a = Vector{Int}()
                        >     for i in 1:5
                        >         push!(a, i)
                        >         if i == 3
                        >             print("TEST")
                        >         end
                        >         while !testvar
                        >             sleep(1)
                        >         end
                        >         open("testfile_$i.txt", "r") do f
                        >             file = read(f, String)
                        >             print(file)
                        >         end
                        >         print("TEST2")
                        >     end
                        >     print(a)
                        > end`)
    });
});

sequence('julia:parametrics'
                , c`> test = SomeParametricType{Int,` , () => {

    type('\n'   , c`> test = SomeParametricType{Int,
                    >                           |`);

    type('Float,\n'
                , c`> test = SomeParametricType{Int,
                    >                           Float,
                    >                           |`);
});

sequence('julia:subsettings'
                , c`> if true` , () => {

    type('\n'   , c`> if true
                    >     |`);

    type('x = testarray[5:end'
                , c`> if true
                    >     x = testarray[5:end|`);

    type(']\n'  , c`> if true
                    >     x = testarray[5:end]
                    >     |`);

    type('z = 5\n'
                , c`> if true
                    >     x = testarray[5:end]
                    >     z = 5
                    >     |`);

    type('end'  , c`> if true
                    >     x = testarray[5:end]
                    >     z = 5
                    > end`);
});

testCase('julia:tuple', () => {
    sequence(         c`> a_tuple = (  # opening tuple w/out elements should indent once` , () => {

        type('\n'   , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     `);

        type('1,\n' , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,
                        >     |`);
        type('2)\n' , c`> a_tuple = (  # opening tuple w/out elements should indent once
                        >     1,
                        >     2)
                        > |`);
    });

    sequence(         c`> a_tuple = (1,  # opening tuple w/ elements indents to opening` , () => {

        type('\n'   , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            |`);

        type('2,\n' , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,
                        >            |`);

        type('3)\n' , c`> a_tuple = (1,  # opening tuple w/ elements indents to opening
                        >            2,
                        >            3)
                        > |`);
    });
});

testCase('julia:using', () => {
    for (const mod of ['', 'TestModule: ']) {
        sequence(         c`> using ${mod}TestModule1,` , () => {

            type('\n'   , c`> using ${mod}TestModule1,
                            >     |`);

            type('TestModule2,\n'
                        , c`> using ${mod}TestModule1,
                            >     TestModule2,
                            >     |`);

            type('TestModule3\n'
                        , c`> using ${mod}TestModule1,
                            >     TestModule2,
                            >     TestModule3
                            > |`);
        });

        sequence(         c`> using ${mod}TestModule1,
                            >     |` , () => {

            type('\n'   , c`> using ${mod}TestModule1,
                            >     ${''}
                            >     |`);

            type('TestModule2,\n'
                        , c`> using ${mod}TestModule1,
                            >     ${''}
                            >     TestModule2,
                            >     |`);

            type('TestModule3\n'
                        , c`> using ${mod}TestModule1,
                            >     ${''}
                            >     TestModule2,
                            >     TestModule3
                            > |`);
        });
    }

    sequence(         c`> using TestModule:` , () => {

        type('\n'   , c`> using TestModule:
                        >     |`);

        type('TestType1,\n'
                    , c`> using TestModule:
                        >     TestType1,
                        >     |`);

        type('TestType2,\n'
                    , c`> using TestModule:
                        >     TestType1,
                        >     TestType2,
                        >     |`);

        type('TestType3\n'
                    , c`> using TestModule:
                        >     TestType1,
                        >     TestType2,
                        >     TestType3
                        > |`);
    });
});

sequence('julia:wrapped_line'
                , c`> long_statement = "a" *  # doesn't fit on a line`, () => {

    type('\n'   , c`> long_statement = "a" *  # doesn't fit on a line
                    >     |`);

    type('"b"\n', c`> long_statement = "a" *  # doesn't fit on a line
                    >     "b"
                    > |`);
});
