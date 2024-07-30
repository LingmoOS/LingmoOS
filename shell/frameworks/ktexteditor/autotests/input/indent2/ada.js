config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 3,
    syntax: 'Ada',
    indentationMode: 'ada',
});

testCase('ada.openpar2', () => {
    type('\n'   , c`> begin
                    >    f(|)`
                , c`> begin
                    >    f(
                    >        |
                    >     )`);
});

testCase('ada.closepar2', () => {
    sequence(         c`> begin
                        >    f(
                        >        arg1`, () => {

        type('\n'   , c`> begin
                        >    f(
                        >        arg1
                        >        `);

        type(')'    , c`> begin
                        >    f(
                        >        arg1
                        >     )`);
    });
    type('\n'   , c`> begin
                    >    f(
                    >        arg1|)`
                , c`> begin
                    >    f(
                    >        arg1
                    >     |)`);
});

testCase('ada.trig1', () => {
    sequence(         c`> begin
                        > loop|
                        > `, () => {

        type('\n'   , c`> begin
                        >    loop
                        >       |
                        > `);

        type('ok\n' , c`> begin
                        >    loop
                        >       ok
                        >       |
                        > `);

        type('end ' , c`> begin
                        >    loop
                        >       ok
                        >    end |
                        > `);

        type('loop' , c`> begin
                        >    loop
                        >       ok
                        >    end loop|
                        > `);
        type(';'    , c`> begin
                        >    loop
                        >       ok
                        >    end loop;|
                        > `);
    });

    type('\n'   , c`> begin
                    >    loop      ok
                    >    end loop;`

                , c`> begin
                    >    loop      ok
                    >    end loop;
                    >    `);
});

testCase('ada.prep1', () => {
    sequence(         c`> begin|
                        > `, () => {

        type('\n'   , c`> begin
                        >    |
                        > `);

        type('s1;'  , c`> begin
                        >    s1;|
                        > `);

        type('\n'   , c`> begin
                        >    s1;
                        >    |
                        > `);

        type('#'    , c`> begin
                        >    s1;
                        > #|
                        > `);
    });
});

testCase('ada.newline', () => {
    type('\n'   , c`> begin
                    > if e then|
                    > `

                , c`> begin
                    >    if e then
                    >       |
                    > `);
});

indentFiles('ada');
