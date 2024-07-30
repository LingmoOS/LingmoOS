config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 2,
    indentationWidth: 2,
    syntax: 'none',
    indentationMode: 'normal',
    selection: '',
});

testCase('normal:cascade', () => {
    type('\n'   , c`> bla bla
                    >     blu blu`
                , c`> bla bla
                    >     blu blu
                    >     `);
});

testCase('normal:emptyline', () => {
    type('\n'   , c`>     totally empty line
                    > `

                , c`>     totally empty line
                    >
                    >     `);

    type('\n'   , c`>     totally empty line
                    >
                    > `

                , c`>     totally empty line
                    >
                    >
                    >     `);

    type('\n'   , c`>     empty line padded with 4 spcs
                    >     `

                , c`>     empty line padded with 4 spcs
                    >     ${''}
                    >     `);
});

testCase('normal:midbreak', () => {
    type('\n'   , c`>     bla bla| blu blu`

                , c`>     bla bla
                    >     |blu blu`);

    type('\n'   , c`>     bla bla|    blu blu`

                , c`>     bla bla
                    >     |blu blu`);
});

testCase('normal:midbreak', () => {
    type('\n'   , 'bla bla'
                , 'bla bla\n');

    type('\n'   , '    bla bla'
                , '    bla bla\n    ');

    type('\n'   , '    bla bla\n    bla bla'
                , '    bla bla\n    bla bla\n    ');
});
