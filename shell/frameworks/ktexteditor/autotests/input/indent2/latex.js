config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 2,
    indentationWidth: 2,
    syntax: 'Latex',
    indentationMode: 'latex',
    selection: '',
});

testCase('latex:env', () => {
    sequence(            '\\begin{foo}', () => {

        type('\n'   , c`> \\begin{foo}
                        >   `);

        type('\\end', c`> \\begin{foo}
                        >   \\end`);

        type('{'    , c`> \\begin{foo}
                        > \\end{`);

        type('foo}' , c`> \\begin{foo}
                        > \\end{foo}`);
    });

    sequence(         c`> \\begin{foo}
                        >   `, () => {

        type('\\begin{bar}'
                    , c`> \\begin{foo}
                        >   \\begin{bar}`);

        type('\n'   , c`> \\begin{foo}
                        >   \\begin{bar}
                        >     `);

        type('\\end{bar}'
                    , c`> \\begin{foo}
                        >   \\begin{bar}
                        >   \\end{bar}`);

        type('\n'   , c`> \\begin{foo}
                        >   \\begin{bar}
                        >   \\end{bar}
                        >   `);

        type('\\end{foo}'
                    , c`> \\begin{foo}
                        >   \\begin{bar}
                        >   \\end{bar}
                        > \\end{foo}`);
    });
});
