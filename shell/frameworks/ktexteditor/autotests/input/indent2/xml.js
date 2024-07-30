config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 2,
    syntax: 'Xml',
    indentationMode: 'xml',
    selection: '',
});

testCase('xml:arguments', () => {
    sequence(            '<opening arg="test', () => {

        type('\n'   , c`> <opening arg="test
                        >          `);

        type('arg2="PASS">'
                    , c`> <opening arg="test
                        >          arg2="PASS">`);

        type('\n'   , c`> <opening arg="test
                        >          arg2="PASS">
                        >   `);
    });

    type('\n'   , c`> <opening arg="test"
                    >          arg2="test" />`

                , c`> <opening arg="test"
                    >          arg2="test" />
                    > `);

    type('\n'   , c`> <opening arg="test"
                    >          arg2="test">text</opening>`

                , c`> <opening arg="test"
                    >          arg2="test">text</opening>
                    > `)
});

testCase('xml:closing', () => {
    for (const prefix of ['', '  ']) {
        sequence(         c`> ${prefix}<tag>
                            > ${prefix}  text`, () => {

            type('\n'   , c`> ${prefix}<tag>
                            > ${prefix}  text
                            > ${prefix}  `);

            type('</tag>'
                        , c`> ${prefix}<tag>
                            > ${prefix}  text
                            > ${prefix}</tag>`);
        });
    };

    sequence(         c`>   <tag>
                        >     text`, () => {

        type('</tag>'
                    , c`>   <tag>
                        >     text</tag>`);

        type('\n'   , c`>   <tag>
                        >     text</tag>
                        >   `);
    });
});

testCase('xml:comment', () => {
    type('\n'   , c`> <!--
                    >       <tag>`

                , c`> <!--
                    >       <tag>
                    >       `);

    type('>'    , c`> <?xml version="1.0" encoding="UTF-8"?>
                    > <!-- indentation in comments should not be touched -->
                    > <!--
                    > <link`

                , c`> <?xml version="1.0" encoding="UTF-8"?>
                    > <!-- indentation in comments should not be touched -->
                    > <!--
                    > <link>`)
});

testCase('xml:empty_lines',  () => {
    cmd([view.align, fn(document.documentRange)]
        , c`> <ul>
            >   <li>a</li>
            >   <li>b</li>
            >   <!--li>c</li-->
            >   </ul>
            >
            >
            > <ul>
            >     <li>a</li>
            >     <li>b</li>
            >
            >
            >     </ul>
            >
            > `

        , c`> <ul>
            >   <li>a</li>
            >   <li>b</li>
            >   <!--li>c</li-->
            > </ul>
            >
            >
            > <ul>
            >   <li>a</li>
            >   <li>b</li>
            >   ${''}
            >   ${''}
            > </ul>
            >
            > `);
});

testCase('xml:keep_indent',  () => {
    type('\n'   , 'text'
                , 'text\n');

    type('\n'   , '  text'
                , '  text\n  ');

    type('\n'   , '    '
                , '    \n    ');
});

testCase('xml:self_closing', () => {
    sequence(         c`>   text`, () => {

        type('<br />'
                    , c`>   text<br />`);

        type('\n'   , c`>   text<br />
                        >   `);

        type('text2', c`>   text<br />
                        >   text2`);
    });

    config({selection: '[]'});

    cmd([view.align, fn(view.selection)]
        , c`> [<tag1>
            >   <tag2/>
            >
            >   <tag2/>
            > </tag1>]`

        , c`> [<tag1>
            >   <tag2/>
            >   ${''}
            >   <tag2/>
            > </tag1>]`);
});

testCase('xml:xhtml', () => {
    sequence(            '<?xml version="1.0" encoding="UTF-8"?>', () => {
        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > `);

        type('<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        > `);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > `);

        type('<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > `);

        type('<head>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   `);

        type('<meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   `);

        type('<title>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>`);

        type('title</title>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   `);

        type('<link rel="stylesheet"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         `);

        type('type="text/css"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         `);

        type('media="screen"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         `);

        type('href="css/style.css" />'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   `);

        type('<!--<link rel="alternate"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   `);

        type('type="application/rss+xml"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   `);

        type('title="Subscribe RSS-feed"\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   `);

        type('href="feeds/feed.rss" />'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />`);

        type('-->'  , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   `);

        type('<style type="text/css">'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >     `);

        type('</style>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   `);

        type('<script type="text/javascript">'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     `);

        type('<![CDATA[\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     `);

        type(']]>'  , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >     `);

        type('</script>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        >   `);

        type('</head>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        > `);

        type('\n' , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > `);

        type('<body>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > `);

        type('<p>'  , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   `);

        type('XHTML is fun.\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        >   `);

        type('</p>' , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > `);

        type('<p>'  , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>`);

        type('And so is Open Source.</p>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > `);

        type('<ul>' , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   `);

        type('<li>' , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>`);

        type('test</li>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   `);

        type('<li>test2</li>\n'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        >   `);

        type('</ul>', c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>
                        > `);

        type('</body>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>
                        > </body>`);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>
                        > </body>
                        > `);

        type('\n'   , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>
                        > </body>
                        >
                        > `);

        type('</html>'
                    , c`> <?xml version="1.0" encoding="UTF-8"?>
                        > <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
                        >
                        > <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
                        > <head>
                        >   <meta content="application/xhtml+xml; charset=utf-8" http-equiv="content-type" />
                        >   <title>title</title>
                        >   <link rel="stylesheet"
                        >         type="text/css"
                        >         media="screen"
                        >         href="css/style.css" />
                        >   <!--<link rel="alternate"
                        >   type="application/rss+xml"
                        >   title="Subscribe RSS-feed"
                        >   href="feeds/feed.rss" />-->
                        >   <style type="text/css">
                        >   </style>
                        >   <script type="text/javascript">
                        >     <![CDATA[
                        >     ]]>
                        >   </script>
                        > </head>
                        >
                        > <body>
                        > <p>
                        >   XHTML is fun.
                        > </p>
                        > <p>And so is Open Source.</p>
                        > <ul>
                        >   <li>test</li>
                        >   <li>test2</li>
                        > </ul>
                        > </body>
                        >
                        > </html>`);
    });
})
