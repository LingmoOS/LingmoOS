config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 3,
    syntax: 'pascal',
    indentationMode: 'pascal',
    selection: '',
});

document.setVariable('cfgIndentBegin', '0');

document.setVariable('cfgIndentCase', 'on');
testCase('pascal:case:indent=on', () => {
    sequence(         c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of`, () => {

        type('\n'   , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       `);

        type('1: '  , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: `);

        type('s1;\n', c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       `);

        type('2: begin\n'
                    , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             `);

        type('s2;'  , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;`);

        type('\n'   , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             `);

        type('s3\n' , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >             `);

        type('end;' , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >          end;`);

        type('\n'   , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >          end;
                        >       `);

        type('3: s4;'
                    , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >          end;
                        >       3: s4;`);

        type('\n'   , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >          end;
                        >       3: s4;
                        >       `);

        type('end;' , c`> // kate: cfgIndentCase=n
                        > begin
                        >    case ce of
                        >       1: s1;
                        >       2: begin
                        >             s2;
                        >             s3
                        >          end;
                        >       3: s4;
                        >    end;`);
    });

    type('2222: begin\n'
                , c`> // kate: cfgIndentCase=n
                    > begin
                    >    case ce of
                    >       1: s1;
                    >       `

                , c`> // kate: cfgIndentCase=n
                    > begin
                    >    case ce of
                    >       1: s1;
                    >       2222: begin
                    >             |`);
});

document.setVariable('cfgIndentCase', 'off');
testCase('pascal:case:indent=off', () => {
    sequence(         c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of`, () => {

        type('\n'   , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    `);

        type('1: '  , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: `);

        type('s1;\n', c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    `);

        type('2: begin\n'
                    , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          `);

        type('s2;'  , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;`);

        type('\n'   , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          `);

        type('s3\n' , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >          `);

        type('end;' , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >       end;`);

        type('\n'   , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >       end;
                        >    `);

        type('3: s4;'
                    , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >       end;
                        >    3: s4;`);

        type('\n'   , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >       end;
                        >    3: s4;
                        >    `);

        type('end;' , c`> // kate: cfgIndentCase=off
                        > begin
                        >    case ce of
                        >    1: s1;
                        >    2: begin
                        >          s2;
                        >          s3
                        >       end;
                        >    3: s4;
                        >    end;`);
    });

    type('2222: begin\n'
                , c`> // kate: cfgIndentCase=off
                    > begin
                    >    case ce of
                    >    1: s1;
                    >    `
                , c`> // kate: cfgIndentCase=off
                    > begin
                    >    case ce of
                    >    1: s1;
                    >    2222: begin
                    >          |`);
});

testCase('pascal:closepar', () => {
    sequence(         c`> begin
                        >    x := a[
                        >             index1,
                        >             index2`, () => {

        type('\n'   , c`> begin
                        >    x := a[
                        >             index1,
                        >             index2
                        >             `);

        type(']'   , c`> begin
                        >    x := a[
                        >             index1,
                        >             index2
                        >          ]`);
    });

    sequence(         c`> begin
                        >    f(
                        >        arg1`, () => {

        type('\n'   , c`> begin
                        >    f(
                        >        arg1
                        >        `);

        type(')'   , c`> begin
                        >    f(
                        >        arg1
                        >     )`);
    });
});

testCase('pascal:comment', () => {
    document.setVariable('cfgAutoInsertStar', 'true');
    document.setVariable('cfgSnapParen', 'false');

    sequence(         c`> // kate: cfgAutoInsertStar true; cfgSnapParen false;
                        > (*`, () => {

        type('\n'   , c`> // kate: cfgAutoInsertStar true; cfgSnapParen false;
                        > (*
                        >  * `);

        type(')'    , c`> // kate: cfgAutoInsertStar true; cfgSnapParen false;
                        > (*
                        >  * )`);
    });

    document.setVariable('cfgSnapParen', 'true');

    sequence(         c`> // kate: cfgAutoInsertStar true; cfgSnapParen true;
                        > (*`, () => {

        type('\n'   , c`> // kate: cfgAutoInsertStar true; cfgSnapParen true;
                        > (*
                        >  * `);

        type(')'    , c`> // kate: cfgAutoInsertStar true; cfgSnapParen true;
                        > (*
                        >  *)`);
    });

    document.setVariable('cfgAutoInsertStar', 'false');

    sequence(         c`> // kate: cfgAutoInsertStar false
                        > (*`, () => {

        type('\n'   , c`> // kate: cfgAutoInsertStar false
                        > (*
                        > `);

        type(')'    , c`> // kate: cfgAutoInsertStar false
                        > (*
                        > )`);
    });
});

testCase('pascal:if', () => {
    sequence(         c`> begin
                        > if e then|
                        >
                        > `, () => {

        type('\n'   , c`> begin
                        >    if e then
                        >       |
                        >
                        > `);

        type('ok\n' , c`> begin
                        >    if e then
                        >       ok
                        >       |
                        >
                        > `);

        type('else\n'
                    , c`> begin
                        >    if e then
                        >       ok
                        >    else
                        >       |
                        >
                        > `);

        type('ok1;' , c`> begin
                        >    if e then
                        >       ok
                        >    else
                        >       ok1;|
                        >
                        > `);

        type('\n'   , c`> begin
                        >    if e then
                        >       ok
                        >    else
                        >       ok1;
                        >    |
                        >
                        > `);

        type('ok2;' , c`> begin
                        >    if e then
                        >       ok
                        >    else
                        >       ok1;
                        >    ok2;|
                        >
                        > `);

        type('\n'   , c`> begin
                        >    if e then
                        >       ok
                        >    else
                        >       ok1;
                        >    ok2;
                        >    |
                        >
                        > `);
    });
});

testCase('pascal:label', () => {
    sequence(         c`> begin`, () => {

        type('\n'   , c`> begin
                        >    `);

        type('s1;'   , c`> begin
                        >    s1;`);

        type('\n'   , c`> begin
                        >    s1;
                        >    |`);

        type('label1:'
                    , c`> begin
                        >    s1;
                        >    label1:`);

        type(' '    , c`> begin
                        >    s1;
                        > label1: `);

        type('x'   , c`> begin
                        >    s1;
                        > label1: x`);
    });
});

testCase('pascal:openpar', () => {
    type('\n'   , c`> begin
                    >    f(|)`

                , c`> begin
                    >    f(
                    >        |
                    >     )`);

    type('\n'   , c`> begin
                    >    q := a[|]`

                , c`> begin
                    >    q := a[
                    >             |
                    >          ]`);
});

testCase('pascal:prep', () => {
    type('#'    , c`> begin
                    >    s1;
                    >    `

                , c`> begin
                    >    s1;
                    > #`);
});

testCase('pascal:trig', () => {
    sequence(         c`> begin
                        >    `, () => {

        type('repeat\n'
                    , c`> begin
                        >    repeat
                        >       `);

        type('ok\n' , c`> begin
                        >    repeat
                        >       ok
                        >       `);

        type('until '
                    , c`> begin
                        >    repeat
                        >       ok
                        >    until `);

        type('c;'   , c`> begin
                        >    repeat
                        >       ok
                        >    until c;`);
    });
});

indentFiles('pascal');
