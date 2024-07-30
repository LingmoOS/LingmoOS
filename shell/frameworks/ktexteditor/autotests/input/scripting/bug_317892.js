/*
 * Mark text to indicate which part is a comment or code.
 */
const labelize = function() {
    const a = [];
    const lines = document.lines();

    let isComment = document.isComment(0, 0);
    let isCode = document.isCode(0, 0);

    if (isCode) a.push('[Code]');
    if (isComment) a.push('[Comment]');

    for (let line = 0; line < lines; ++line) {
        const textLine = document.line(line);
        const columns = document.lineLength(line);
        for (let column = 0; column <= columns; ++column) {
            if (isCode !== document.isCode(line, column)) {
                a.push(isCode ? '[/Code]' : '[Code]');
                isCode = !isCode;
            }
            if (isComment !== document.isComment(line, column)) {
                a.push(isComment ? '[/Comment]' : '[Comment]');
                isComment = !isComment;
            }
            a.push(textLine[column]);
        }
        a.push('\n');
    }

    if (isComment) a.push('[/Comment]');
    if (isCode) a.push('[/Code]');

    document.setText(a.join(''));
};

testCase('bug_317892', () => {
    config({
        syntax: 'xml',
        selection: '',
        blockSelection: false,
    });
    cmd(labelize, `\
<?xml version="1.0" encoding="UTF-8"?>
<!-- closed comment -->
<!-- open comment >
`, `\
[Code]<?xml[/Code] version="1.0" encoding="UTF-8"[Code]?>
[/Code][Comment]<!-- closed comment -->
<!-- open comment >

[/Comment]`);
});
