var katescript = {
    "author": "Henri Kaustinen <heka1@protonmail.com>",
    "license": "LGPLv2+",
    "revision": 1,
    "kate-version": "23.04",
    "functions": ["nextParagraph", "previousParagraph"],
    "actions": [
        {   "function": "nextParagraph",
            "name": "Go to Next Paragraph",
            "category": "Navigation"
        },
        {   "function": "previousParagraph",
            "name": "Go to Previous Paragraph",
            "category": "Navigation"
        }
    ]
};

/*
 * Move cursor after current or next paragraph. Paragraph ends with empty line,
 * line with only whitespace or document end.
*/
function nextParagraph() {
    var cursor = view.cursorPosition();
    var line = cursor.line;

    // If cursor is already on an empty line, move to the next non-empty line.
    if (document.line(line).trim() === '') {
        while (line < document.lines() - 1 && document.line(line).trim() === '') {
            line++;
        }
    }

    // Find the next empty line.
    while (line < document.lines() - 1 && document.line(line).trim() !== '') {
        line++;
    }

    cursor.line = line;
    cursor.column = document.lineLength(cursor.line);
    view.setCursorPosition(cursor);
}

/*
 * Move cursor before current or previous paragraph. Paragraph starts after line.
 * line, line with only whitespace or document start.
*/
function previousParagraph() {
    var cursor = view.cursorPosition();
    var line = cursor.line;

    // If cursor is already on an empty line, move to the previous non-empty line.
    if (document.line(line).trim() === '') {
        while (line > 0 && document.line(line).trim() === '') {
            line--;
        }
    }

    // Find the first empty line before the paragraph.
    while (line > 0 && document.line(line).trim() !== '') {
        line--;
    }

    cursor.line = line;
    cursor.column = cursor.line == 0 ? 0 : document.lineLength(cursor.line);
    view.setCursorPosition(cursor);
}

function help(cmd)
{
    if (cmd == 'nextParagraph') {
        return i18n('Move the cursor after the current or next paragraph.');
    } else if (cmd == 'previousParagraph') {
        return i18n('Move the cursor before the current or previous paragraph.');
    }
}

// kate: space-indent on; indent-width 4; replace-tabs on;
