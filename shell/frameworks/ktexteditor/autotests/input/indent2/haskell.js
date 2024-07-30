config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'Haskell',
    indentationMode: 'haskell',
    selection: '',
    cursor: '@',
});

testCase('haskell', () => {
    type('\n'   , 'primitives = [("+", numericBinop (+)),'
                , 'primitives = [("+", numericBinop (+)),\n    ');

    type('\n'   , c`> main = do
                    >     -- This is a comment
                    >     something
                    >     something
                    > `

                , c`> main = do
                    >     -- This is a comment
                    >     something
                    >     something
                    >
                    >     `);

    sequence(            'foo = if true', () => {

        type('\n'   , c`> foo = if true
                        >          `);

        type('then ', c`> foo = if true
                        >          then `);

        type('1\n',   c`> foo = if true
                        >          then 1
                        >          `);

        type('else ',   c`> foo = if true
                        >          then 1
                        >          else `);
    });

    sequence(         c`> parseExpr = parseString
                        >         `, () => {

        type('<|'   , c`> parseExpr = parseString
                        >         <|`);

        type('> '   , c`> parseExpr = parseString
                        >         <|> `);

        type('parseNumber\n'
                    , c`> parseExpr = parseString
                        >         <|> parseNumber
                        >         `);

        type('<|'   , c`> parseExpr = parseString
                        >         <|> parseNumber
                        >         <|`);

        type('> '   , c`> parseExpr = parseString
                        >         <|> parseNumber
                        >         <|> `);
    });
});
