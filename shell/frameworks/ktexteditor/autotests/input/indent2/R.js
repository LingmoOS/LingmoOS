config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'R Script',
    indentationMode: 'r',
    selection: '',
});

testCase('R:bracketsBreak', () => {
    type('\n'   , 'test(|)'
                , 'test(\n    |\n)');
});

sequence('R:closeBracket'
                , c`> test(
                    >     text`, () => {

    type('\n'   , c`> test(
                    >     text
                    >     `);

    type(')'    , c`> test(
                    >     text
                    > )`);
});

sequence('R:indentAssign',
                     'tmp <-', () => {

    type('\n'   , c`> tmp <-
                    >     `);

    type('Thisisaverylongvariablename'
                , c`> tmp <-
                    >     Thisisaverylongvariablename`);

    type('\n'   , c`> tmp <-
                    >     Thisisaverylongvariablename
                    > `);

    type('test <- NA'
                , c`> tmp <-
                    >     Thisisaverylongvariablename
                    > test <- NA`);

    type('\n'   , c`> tmp <-
                    >     Thisisaverylongvariablename
                    > test <- NA
                    > `);

    type("test <- 'String'"
                , c`> tmp <-
                    >     Thisisaverylongvariablename
                    > test <- NA
                    > test <- 'String'`);

    type('\n'   , c`> tmp <-
                    >     Thisisaverylongvariablename
                    > test <- NA
                    > test <- 'String'
                    > `);
});

testCase('R:indentBracket', () => {
    type('\n'   , c`plot(y ~ x,|)`
                , c`> plot(y ~ x,
                    >      |)`);

    type('\n'   , c`> plot(y ~ x,
                    >      data = data)`
                , c`> plot(y ~ x,
                    >      data = data)
                    > `);
});

sequence('R:indentFormula'
                ,    'lm(Response ~ Var1 +', () => {

    type('\n'   , c`> lm(Response ~ Var1 +
                    >               `);

    type('Var2 +'
                , c`> lm(Response ~ Var1 +
                    >               Var2 +`);

    type('\n'   , c`> lm(Response ~ Var1 +
                    >               Var2 +
                    >               `);

    type('Var3,', c`> lm(Response ~ Var1 +
                    >               Var2 +
                    >               Var3,`);

    type('\n'   , c`> lm(Response ~ Var1 +
                    >               Var2 +
                    >               Var3,
                    >    `);
});

sequence('R:indentOperator'
                ,    'ggplot(data = data) +', () => {

    type('\n'   , c`> ggplot(data = data) +
                    >     `);

    type('geom_line(aes(x = something, y = otherstuff)) +'
                , c`> ggplot(data = data) +
                    >     geom_line(aes(x = something, y = otherstuff)) +`);

    type('\n'   , c`> ggplot(data = data) +
                    >     geom_line(aes(x = something, y = otherstuff)) +
                    >     `);

    type('coord_flip()'
                , c`> ggplot(data = data) +
                    >     geom_line(aes(x = something, y = otherstuff)) +
                    >     coord_flip()`);

    type('\n'   , c`> ggplot(data = data) +
                    >     geom_line(aes(x = something, y = otherstuff)) +
                    >     coord_flip()
                    > `);
});

withInput('R:indentPaste'
        , c`> if(1) {
            >     |
            > }`, () => {

    cmd([paste
        , c`> ggplot(data = data) +
            >         geom_line(aes(x = something, y = otherstuff)) +
            >         coord_flip()`]

        , c`> if(1) {
            >     ggplot(data = data) +
            >         geom_line(aes(x = something, y = otherstuff)) +
            >         coord_flip()|
            > }`);

    cmd([paste, 'mult <- function(a,b) {\n    a*b\n\n}']
        , c`> if(1) {
            >     mult <- function(a,b) {
            >         a*b
            >         ${''}
            >     }|
            > }`);
});

sequence('R:skipComment'
                , c`> data %>%
                    >     select(-Column) %>%
                    >     #filter(ID == "id2") %>%
                    >     gather() %>%`, () => {

    type('\n'   , c`> data %>%
                    >     select(-Column) %>%
                    >     #filter(ID == "id2") %>%
                    >     gather() %>%
                    >     `);

    type('drop_na()'
                , c`> data %>%
                    >     select(-Column) %>%
                    >     #filter(ID == "id2") %>%
                    >     gather() %>%
                    >     drop_na()`);

    type('\n'   , c`> data %>%
                    >     select(-Column) %>%
                    >     #filter(ID == "id2") %>%
                    >     gather() %>%
                    >     drop_na()
                    > `);
});
