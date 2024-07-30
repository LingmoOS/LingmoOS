var katescript = {
    "name": "Replicode",
    "author": "Martin Sandsmark <martin.sandsmark@kde.org>",
    "license": "BSD",
    "revision": 2,
    "kate-version": "5.1",
    "indent-languages": ["Replicode"]
}; // kate-script-header, must be at the start of the file without comments, pure json

function indent(line, indentWidth, ch)
{
    var lastLevel = document.firstColumn(line-1);

    if (lastLevel != -1 && document.endsWith(line-1, "[]", true)) {
        return lastLevel + 3;
    } else {
        return -1;
    }
}
