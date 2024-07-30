loadScript(":/ktexteditor/script/commands/utils.js");

testCase("alignon", () => {
    withInput(    '    - foobar: lala, lili, lolo, lulu\n'
                + '- baz: a, b, c, d\n'
                + '  - quux: this, is, an, automated test\n'
                + 'x - hello: world!', () => {
        cmd(fn(view.alignOn, fn(document.documentRange))
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz: a, b, c, d\n'
                + '    - quux: this, is, an, automated test\n'
                + '    x - hello: world!');
        cmd(fn(view.alignOn, fn(document.documentRange), '-')
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz: a, b, c, d\n'
                + '    - quux: this, is, an, automated test\n'
                + 'x   - hello: world!');
    });

    sequence(REUSE_LAST_EXPECTED_OUTPUT, () => {
        cmd(fn(view.alignOn, fn(document.documentRange), ':\\s(.)')
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz:    a, b, c, d\n'
                + '    - quux:   this, is, an, automated test\n'
                + 'x   - hello:  world!');

        config({blockSelection: true});

        cmd(fn(view.alignOn, new Range(1, 14, 2, 30), ',\\s(.)')
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz:    a,    b, c, d\n'
                + '    - quux:   this, is, an, automated test\n'
                + 'x   - hello:  world!');
        cmd(fn(view.alignOn, new Range(1, 19, 2, 30), ',\\s(.)')
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz:    a,    b,  c, d\n'
                + '    - quux:   this, is, an, automated test\n'
                + 'x   - hello:  world!');
        cmd(fn(view.alignOn, new Range(1, 24, 2, 30), ',\\s(.)')
                , '    - foobar: lala, lili, lolo, lulu\n'
                + '    - baz:    a,    b,  c,  d\n'
                + '    - quux:   this, is, an, automated test\n'
                + 'x   - hello:  world!');
    })
});

testCase("block_selection", () => {
    config({blockSelection: true});
    sequence( 'abc[defghijklm\n'
            + 'nopqrs]tuvwxyz\n', () => {
        cmd(view.removeSelectedText
            , 'abc|[ghijklm\n'
            + 'nop]tuvwxyz\n');
        type("PASS"
            , 'abcPASS|[ghijklm\n'
            + 'nopPASS]tuvwxyz\n');
    })
});

testCase("moveLinesDown", () => {
    config({blockSelection: false});

    sequence(              '1|\n2\n3', () => {
        cmd(moveLinesDown, '2\n1|\n3');
        // bug_309977
        cmd(moveLinesDown, '2\n3\n1|', "Tests if it is possible to move a line to the last line of the document (if the last line is empty).");
        cmd(moveLinesDown, '2\n3\n1|');
    });

    sequence(              '1|\n2\n3\n', () => {
        cmd(moveLinesDown, '2\n1|\n3\n');
        cmd(moveLinesDown, '2\n3\n1|\n');
        cmd(moveLinesDown, '2\n3\n\n1|');
        cmd(moveLinesDown, '2\n3\n\n1|');
    });

    sequence(              '|1\n2\n3\n', () => {
        cmd(moveLinesDown, '2\n|1\n3\n');
        cmd(moveLinesDown, '2\n3\n|1\n');
        // bug_309977
        cmd(moveLinesDown, '2\n3\n\n|1');
        cmd(moveLinesDown, '2\n3\n\n|1');
    });

    sequence(              '[1]\n2\n3', () => {
        cmd(moveLinesDown, '2\n[1]\n3');
        cmd(moveLinesDown, '2\n3\n[1]');
        cmd(moveLinesDown, '2\n3\n[1]');
    });

    // bug_309978
    sequence(              '[1]\n2\n3\n', () => {
        cmd(moveLinesDown, '2\n[1]\n3\n');
        cmd(moveLinesDown, '2\n3\n[1]\n');
        cmd(moveLinesDown, '2\n3\n\n[1]');
        cmd(moveLinesDown, '2\n3\n\n[1]');
    });

    sequence(              '[1\n]2\n3', () => {
        cmd(moveLinesDown, '2\n[1\n]3');
        cmd(moveLinesDown, '2\n3\n[1]');
        cmd(moveLinesDown, '2\n3\n[1]');
    });

    sequence(              '[1\n]2\n3\n', () => {
        cmd(moveLinesDown, '2\n[1\n]3\n');
        cmd(moveLinesDown, '2\n3\n[1\n]');
        cmd(moveLinesDown, '2\n3\n\n[1]');
        cmd(moveLinesDown, '2\n3\n\n[1]');
    });

    sequence(              '1[\n]2\n3', () => {
        cmd(moveLinesDown, '2\n1[\n]3');
        cmd(moveLinesDown, '2\n3\n1[]'); // TODO empty selection !
        cmd(moveLinesDown, '2\n3\n1|'); // empty selection is ignored in input...
        cmd(moveLinesDown, '2\n3\n1|');
    });
    xcmd(moveLinesDown, '2\n1[\n]3', '2\n3\n1|'); // see above

    sequence(              '1[\n]2\n3\n', () => {
        cmd(moveLinesDown, '2\n1[\n]3\n');
        cmd(moveLinesDown, '2\n3\n1[\n]');
        cmd(moveLinesDown, '2\n3\n\n1[]'); // TODO empty selection !
        cmd(moveLinesDown, '2\n3\n\n1|'); // empty selection is ignored in input...
        cmd(moveLinesDown, '2\n3\n\n1|');
    });
    xcmd(moveLinesDown, '2\n3\n1[\n]', '2\n3\n\n1|'); // see above


    sequence(              '[1\n2]\n3\n', () => {
        cmd(moveLinesDown, '3\n[1\n2]\n');
        cmd(moveLinesDown, '3\n\n[1\n2]');
        cmd(moveLinesDown, '3\n\n[1\n2]');
    });

    sequence(              '1[S]\n2\n3\n', () => {
        cmd(moveLinesDown, '2\n1[S]\n3\n');
        cmd(moveLinesDown, '2\n3\n1[S]\n');
        cmd(moveLinesDown, '2\n3\n\n1[S]');
        cmd(moveLinesDown, '2\n3\n\n1[S]');
    });
});

testCase("moveLinesUp", () => {
    config({blockSelection: false});

    sequence(            '2\n3\n1|', () => {
        cmd(moveLinesUp, '2\n1|\n3');
        cmd(moveLinesUp, '1|\n2\n3');
        cmd(moveLinesUp, '1|\n2\n3');
    });

    sequence(            '2\n3\n1|\n', () => {
        cmd(moveLinesUp, '2\n1|\n3\n');
        cmd(moveLinesUp, '1|\n2\n3\n');
        cmd(moveLinesUp, '1|\n2\n3\n');
    });

    sequence(            '2\n3\n|1', () => {
        cmd(moveLinesUp, '2\n|1\n3');
        cmd(moveLinesUp, '|1\n2\n3');
        cmd(moveLinesUp, '|1\n2\n3');
    });

    sequence(            '2\n3\n|1\n', () => {
        cmd(moveLinesUp, '2\n|1\n3\n');
        cmd(moveLinesUp, '|1\n2\n3\n');
        cmd(moveLinesUp, '|1\n2\n3\n');
    });

    sequence(            '2\n3\n[1]', () => {
        cmd(moveLinesUp, '2\n[1]\n3');
        cmd(moveLinesUp, '[1]\n2\n3');
        cmd(moveLinesUp, '[1]\n2\n3');
    });

    sequence(            '2\n3\n[1]\n', () => {
        cmd(moveLinesUp, '2\n[1]\n3\n');
        cmd(moveLinesUp, '[1]\n2\n3\n');
        cmd(moveLinesUp, '[1]\n2\n3\n');
    });

    sequence(            '2\n3[\n1]', () => {
        cmd(moveLinesUp, '3[\n1]\n2');
        cmd(moveLinesUp, '3[\n1]\n2');
    });

    sequence(            '2\n3[\n1]\n', () => {
        cmd(moveLinesUp, '3[\n1]\n2\n');
        cmd(moveLinesUp, '3[\n1]\n2\n');
    });

    sequence(            '2\n3\n[1\n]', () => {
        cmd(moveLinesUp, '2\n[1\n]3\n');
        cmd(moveLinesUp, '[1\n]2\n3\n');
        cmd(moveLinesUp, '[1\n]2\n3\n');
    });

    sequence(            '2\n1[\n]3', () => {
        cmd(moveLinesUp, '1[\n]2\n3');
        cmd(moveLinesUp, '1[\n]2\n3');
    });

    sequence(            '2\n1\n3[\n]', () => {
        cmd(moveLinesUp, '2\n3[\n]1\n');
        cmd(moveLinesUp, '3[\n]2\n1\n');
        cmd(moveLinesUp, '3[\n]2\n1\n');
    });

    sequence(            '3\n\n[1\n2]', () => {
        cmd(moveLinesUp, '3\n[1\n2]\n');
        cmd(moveLinesUp, '[1\n2]\n3\n');
        cmd(moveLinesUp, '[1\n2]\n3\n');
    });

    sequence(            '2\n3\n\n1[S]', () => {
        cmd(moveLinesUp, '2\n3\n1[S]\n');
        cmd(moveLinesUp, '2\n1[S]\n3\n');
        cmd(moveLinesUp, '1[S]\n2\n3\n');
        cmd(moveLinesUp, '1[S]\n2\n3\n');
    });
});

testCase("duplicateLinesDown", () => {
    for (const [input, expectedOutput] of [
        ['|[1]\n2',    '1\n|[1]\n2'],
        ['|[1]\n2\n',  '1\n|[1]\n2\n'],
        ['|[1]\n2\n3', '1\n|[1]\n2\n3'],

        ['[1]|\n2',    '1\n[1]|\n2'],
        ['[1]|\n2\n',  '1\n[1]|\n2\n'],
        ['[1]|\n2\n3', '1\n[1]|\n2\n3'],

        ['1\n|[2]',    '1\n2\n|[2]'],
        ['1\n|[2]\n',  '1\n2\n|[2]\n'],
        ['1\n|[2]\n3', '1\n2\n|[2]\n3'],

        ['1\n[2]|',    '1\n2\n[2]|'],
        ['1\n[2]|\n',  '1\n2\n[2]|\n'],

        ['1\n[2]|S',    '1\n2S\n[2]|S'],
        ['1\n[2]|S\n',  '1\n2S\n[2]|S\n'],
        ['1\n[2]|S\n3', '1\n2S\n[2]|S\n3'],

        ['1\n2|[S]',    '1\n2S\n2|[S]'],
        ['1\n2|[S]\n',  '1\n2S\n2|[S]\n'],
        ['1\n2|[S]\n3', '1\n2S\n2|[S]\n3'],
    ]) {
        cmd(duplicateLinesDown, input, expectedOutput);
        cmd(duplicateLinesDown,
              input.replace('[', '').replace(']', ''),
              expectedOutput.replace('[', '').replace(']', ''));
    }

    cmd(duplicateLinesDown, '[1\n]\n2', '1\n[1\n]\n2');
    cmd(duplicateLinesDown, '[1\n\n]2', '1\n\n[1\n\n]2');
    cmd(duplicateLinesDown, '[1\n\n2]', '1\n\n2\n[1\n\n2]');

    cmd(duplicateLinesDown, '[1\n]\n2\n', '1\n[1\n]\n2\n');
    cmd(duplicateLinesDown, '[1\n\n]2\n', '1\n\n[1\n\n]2\n');
    cmd(duplicateLinesDown, '[1\n\n2]\n', '1\n\n2\n[1\n\n2]\n');
    cmd(duplicateLinesDown, '[1\n\n2\n]', '1\n\n2\n[1\n\n2\n]');

    cmd(duplicateLinesDown, '1\n[2\n]3\n', '1\n2\n[2\n]3\n');

    cmd(duplicateLinesDown, '1[\n]\n2', '1\n1[\n]\n2');
    cmd(duplicateLinesDown, '1[\n\n]2', '1\n\n1[\n\n]2');
    cmd(duplicateLinesDown, '1[\n\n2]', '1\n\n2\n1[\n\n2]');

    cmd(duplicateLinesDown, '1[\n]\n2\n', '1\n1[\n]\n2\n');
    cmd(duplicateLinesDown, '1[\n\n]2\n', '1\n\n1[\n\n]2\n');
    cmd(duplicateLinesDown, '1[\n\n2]\n', '1\n\n2\n1[\n\n2]\n');
    cmd(duplicateLinesDown, '1[\n\n2\n]', '1\n\n2\n1[\n\n2\n]');

    cmd(duplicateLinesDown, '1[\n2]\n3\n', '1\n2\n1[\n2]\n3\n');

    cmd(duplicateLinesDown, '[1\n2\n3]\n4\n', '1\n2\n3\n[1\n2\n3]\n4\n');
    cmd(duplicateLinesDown, '[1\n2\n3\n]4\n', '1\n2\n3\n[1\n2\n3\n]4\n');
});

testCase("duplicateLinesUp", () => {
    for (const [input, expectedOutput] of [
        ['|[1]\n2',    '|[1]\n1\n2'],
        ['|[1]\n2\n',  '|[1]\n1\n2\n'],
        ['|[1]\n2\n3', '|[1]\n1\n2\n3'],

        ['[1]|\n2',    '[1]|\n1\n2'],
        ['[1]|\n2\n',  '[1]|\n1\n2\n'],
        ['[1]|\n2\n3', '[1]|\n1\n2\n3'],

        ['1\n|[2]',    '1\n|[2]\n2'],
        ['1\n|[2]\n',  '1\n|[2]\n2\n'],
        ['1\n|[2]\n3', '1\n|[2]\n2\n3'],

        ['1\n[2]|',    '1\n[2]|\n2'],
        ['1\n[2]|\n',  '1\n[2]|\n2\n'],

        ['1\n[2]|S',    '1\n[2]|S\n2S'],
        ['1\n[2]|S\n',  '1\n[2]|S\n2S\n'],
        ['1\n[2]|S\n3', '1\n[2]|S\n2S\n3'],

        ['1\n2|[S]',    '1\n2|[S]\n2S'],
        ['1\n2|[S]\n',  '1\n2|[S]\n2S\n'],
        ['1\n2|[S]\n3', '1\n2|[S]\n2S\n3'],
    ]) {
        cmd(duplicateLinesUp, input, expectedOutput);
        cmd(duplicateLinesUp,
              input.replace('[', '').replace(']', ''),
              expectedOutput.replace('[', '').replace(']', ''));
    }

    cmd(duplicateLinesUp, '[1\n]\n2', '[1\n]1\n\n2');
    cmd(duplicateLinesUp, '[1\n\n]2', '[1\n\n]1\n\n2');
    cmd(duplicateLinesUp, '[1\n\n2]', '[1\n\n2]\n1\n\n2');

    cmd(duplicateLinesUp, '[1\n]\n2\n', '[1\n]1\n\n2\n');
    cmd(duplicateLinesUp, '[1\n\n]2\n', '[1\n\n]1\n\n2\n');
    cmd(duplicateLinesUp, '[1\n\n2]\n', '[1\n\n2]\n1\n\n2\n');
    cmd(duplicateLinesUp, '[1\n\n2\n]', '[1\n\n2\n]1\n\n2\n');

    cmd(duplicateLinesUp, '1\n[2\n]3\n', '1\n[2\n]2\n3\n');

    cmd(duplicateLinesUp, '1[\n]\n2', '1[\n]1\n\n2');
    cmd(duplicateLinesUp, '1[\n\n]2', '1[\n\n]1\n\n2');
    cmd(duplicateLinesUp, '1[\n\n2]', '1[\n\n2]\n1\n\n2');

    cmd(duplicateLinesUp, '1[\n]\n2\n', '1[\n]1\n\n2\n');
    cmd(duplicateLinesUp, '1[\n\n]2\n', '1[\n\n]1\n\n2\n');
    cmd(duplicateLinesUp, '1[\n\n2]\n', '1[\n\n2]\n1\n\n2\n');
    cmd(duplicateLinesUp, '1[\n\n2\n]', '1[\n\n2\n]1\n\n2\n');

    cmd(duplicateLinesUp, '1[\n2]\n3\n', '1[\n2]\n1\n2\n3\n');

    cmd(duplicateLinesUp, '[1\n2\n3]\n4\n', '[1\n2\n3]\n1\n2\n3\n4\n');
    cmd(duplicateLinesUp, '[1\n2\n3\n]4\n', '[1\n2\n3\n]1\n2\n3\n4\n');
});
