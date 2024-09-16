# The behavior of dde-application-manager to process Key Exec

本文档描述了dde-application-manager如何处理Desktop文件中的Exec字段

## 原始文档

以下文字引用于[desktop entry specification/The Exec Key](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#exec-variables)

>
>The Exec key must contain a command line. A command line consists of an executable program optionally followed by one or more arguments. The executable program can either be specified with its full path or with the name of the executable only. If no full path is provided the executable is looked up in the \$PATH environment variable used by the desktop environment. The name or path of the executable program may not contain the equal sign ("="). Arguments are separated by a space.
>
>Arguments may be quoted in whole. If an argument contains a reserved character the argument must be quoted. The rules for quoting of arguments is also applicable to the executable name or path of the executable program as provided.
>
>Quoting must be done by enclosing the argument between double quotes and escaping the double quote character, backtick character ("\`"), dollar sign ("\$") and backslash character ("\\") by preceding it with an additional backslash character. Implementations must undo quoting before expanding field codes and before passing the argument to the executable program. Reserved characters are space ("\ "), tab, newline, double quote, single quote ("\'"), backslash character ("\\"), greater-than sign ("\>"), less-than sign ("<"), tilde ("~"), vertical bar ("|"), ampersand ("&"), semicolon ("\;"), dollar sign \("\$"), asterisk ("\*"), question mark ("\?"), hash mark ("#"), parenthesis ("(") and (")") and backtick character ("\`").
>
>Note that the general escape rule for values of type string states that the backslash character can be escaped as ("\\\\") as well and that this escape rule is applied before the quoting rule. As such, to unambiguously represent a literal backslash character in a quoted argument in a desktop entry file requires the use of four successive backslash characters ("\\\\\\\\"). Likewise, a literal dollar sign in a quoted argument in a desktop entry file is unambiguously represented with ("\\\\$").
>
>A number of special field codes have been defined which will be expanded by the file manager or program launcher when encountered in the command line. Field codes consist of the percentage character ("%") followed by an alpha character. Literal percentage characters must be escaped as \%\%. Deprecated field codes should be removed from the command line and ignored. Field codes are expanded only once, the string that is used to replace the field code should not be checked for field codes itself.
>
>Command lines that contain a field code that is not listed in this specification are invalid and must not be processed, in particular implementations may not introduce support for field codes not listed in this specification. Extensions, if any, should be introduced by means of a new key.
>
>Implementations must take care not to expand field codes into multiple arguments unless explicitly instructed by this specification. This means that name fields, filenames and other replacements that can contain spaces must be passed as a single argument to the executable program after expansion.
>
>Although the Exec key is defined to have a value of the type string, which is limited to ASCII characters, field code expansion may introduce non-ASCII characters in arguments. Implementations must take care that all characters in arguments passed to the executable program are properly encoded according to the applicable locale setting.
>
>Recognized field codes are as follows:
>
>| Code  | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>| :---: | :---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
>|  %f   | A single file name (including the path), even if multiple files are selected. The system reading the desktop entry should recognize that the program in question cannot handle multiple file arguments, and it should should probably spawn and execute multiple copies of a program for each selected file if the program is not able to handle additional file arguments. If files are not on the local file system (i.e. are on HTTP or FTP locations), the files will be copied to the local file system and %f will be expanded to point at the temporary file. Used for programs that do not understand the URL syntax. |
>|  %F   | A list of files. Use for apps that can open several local files at once. Each file is passed as a separate argument to the executable program.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |
>|  %u   | A single URL. Local files may either be passed as file: URLs or as file path.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
>|  %U   | A list of URLs. Each URL is passed as a separate argument to the executable program. Local files may either be passed as file: URLs or as file path.                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
>|  %d   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>|  %D   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>|  %n   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>|  %N   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>|  %i   | The Icon key of the desktop entry expanded as two arguments, first --icon and then the value of the Icon key. Should not expand to any arguments if the Icon key is empty or missing.                                                                                                                                                                                                                                                                                                                                                                                                                                         |
>|  %c   | The translated name of the application as listed in the appropriate Name key in the desktop entry.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
>|  %k   | The location of the desktop file as either a URI (if for example gotten from the vfolder system) or a local filename or empty if no location is known.                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
>|  %v   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>|  %m   | Deprecated.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
>
>A command line may contain at most one %f, %u, %F or %U field code. If the application should not open any file the %f, %u, %F and %U field codes must be removed from the command line and ignored.
>
>Field codes must not be used inside a quoted argument, the result of field code expansion inside a quoted argument is undefined. The %F and %U field codes may only be used as an argument on their own.
>

## 中文翻译

Exec关键字中必须包含一个命令行。命令行由一个可执行程序和可选地一个或多个参数组成。可执行程序可以是指定其完整路径，也可以只是可执行文件的名称。如果没有提供完整路径，则在桌面环境中使用的$PATH环境变量中查找可执行文件。可执行程序的名称或路径不能包含等号（"="）。参数用空格分隔。

参数可以被引号全部包裹。如果参数包含保留字符，则必须用引号包裹该参数。包裹参数的规则也适用于提供的可执行程序的可执行文件名或路径。

包裹参数必须通过将参数置于双引号之间并通过在其前面添加额外的反斜杠字符来转义双引号字符、反引号字符（"\`"）、美元符号（"\$"）和反斜杠字符（"\\"）。在展开字段代码和将参数传递给可执行程序之前，实现必须取消引用。保留字符包括空格（"\ "）、制表符、换行符、双引号、单引号（"'"）、反斜杠字符（"\\"）、大于号（">"）、小于号（"<"）、波浪号（"~"）、竖线（"|"）、和符号（"&"）、分号（";"）、美元符号（"$"）、星号（"*"）、问号（"?"）、井号（"#"）、括号（"("）和（"）"）和反引号字符（"`"）。

需要注意的是，对于字符串类型的值的一般转义规则规定，反斜杠字符也可以转义为（"\\")，并且在引用规则之前应用此转义规则。因此，在桌面入口文件中引用的引号参数中清楚地表示文字反斜杠字符需要使用四个连续的反斜杠字符（"\\\\\\\\"）。同样，桌面入口文件中引用的文字美元符号可以使用（"\\\\$"）清晰地表示。

在标准中已定义了许多特殊的字段代码，当在命令行中遇到时，文件管理器或程序启动器将扩展它们。字段代码由百分号字符（"%"）后跟一个字母字符组成。百分号字符必须作为%%转义。并且删除并忽略已弃用的字段代码。字段代码仅扩展一次，用于替换字段代码的字符串不应检查是否包含字段代码本身。

包含在此规范中未列出的字段代码的命令行无效，不得进行处理，特别是实现不得引入未在此规范中列出的字段代码的支持。如果需要的话必须通过新键引入扩展。

在实现的时候不要将字段代码扩展为多个参数，除非由本规范明确指示。这意味着名称字段、文件名和其他可能包含空格的替换必须在展开后作为单个参数传递给可执行程序。

虽然Exec键被定义为具有字符串类型的值，该类型限于ASCII字符，但字段代码扩展可能会在参数中引入非ASCII字符。实现是必须确保传递给可执行程序的参数中的所有字符根据适用的区域设置进行正确编码。

标准中定义的字段代码如下表所示:

| 字段代码 | 描述                                                                                                                                                                                                                                                                                                                                              |
| :------: | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
|    %f    | 一个文件名或本地文件路径（即使传入了多个文件）。读取desktop entry时应该意识到这个程序无法处理多个文件参数，如果该程序无法处理额外的文件参数，它可能会生成并执行多个程序副本以处理每个选定的文件。如果文件不在本地文件系统上（即位于HTTP或FTP位置），则文件将被复制到本地文件系统，并且%f将被扩展为临时文件路径。本字段用于无法处理URL语法的程序。 |
|    %F    | 一个文件列表。用于同时打开多个本地文件的应用程序。每个文件都作为单独的参数传递给可执行程序。                                                                                                                                                                                                                                                      |
|    %u    | 一个URL。本地文件可以以file:URL或文件路径传递。                                                                                                                                                                                                                                                                                                   |
|    %U    | 一个URL列表。每个URL作为单独的参数传递给可执行程序。本地文件可以以file:URL或文件路径传递。                                                                                                                                                                                                                                                        |
|    %d    | 废弃                                                                                                                                                                                                                                                                                                                                              |
|    %D    | 废弃                                                                                                                                                                                                                                                                                                                                              |
|    %n    | 废弃                                                                                                                                                                                                                                                                                                                                              |
|    %N    | 废弃                                                                                                                                                                                                                                                                                                                                              |
|    %i    | 将desktop entry中的Icon键扩展为两个参数，首先是--icon，然后是Icon的值。如果Icon键为空或缺失，则不应扩展为任何参数。                                                                                                                                                                                                                               |
|    %c    | desktop entry中相应名称键列出的应用程序翻译名称。                                                                                                                                                                                                                                                                                                 |
|    %k    | 桌面文件的位置，可以是 URI（例如从 vfolder 系统获取）或本地文件名，如果不知道位置，则为空。                                                                                                                                                                                                                                                       |
|    %v    | 废弃                                                                                                                                                                                                                                                                                                                                              |
|    %m    | 废弃                                                                                                                                                                                                                                                                                                                                              |

一个命令行中最多只能包含一个%f、%u、%F或%U字段代码。如果应用程序没有打开任何文件，则必须从命令行中删除并忽略%f、%u、%F和%U字段代码。

字段代码不能在包裹的参数内部使用，包裹参数内的字段代码扩展的结果是未定义的。%F和%U字段代码只能作为自己的参数使用。

## 实现定义行为的补充

1. 本实现不会检测二进制文件的格式合法性。因为最后是由systemd启动对应的二进制，而且二进制名称中有%等符号是合理的。如果启动失败可以查看systemd中的日志。
2. 本实现中只会将传入的字段扩展到第一个解析到的字段代码中，后续字段代码将会被忽略并移除。目前实现没有做参数的合法性校验，传入者需要自己保证传参合法性，同时对于%f的字段代码，目前不支持非本地文件系统的入参。
3. 对于%u和%U传入本地文件路径时，推荐传入绝对路径，如果希望传入相对路径，需要在启动时指定二进制的工作目录。
4. 本实现在字段代码扩展时不会对扩展后的结果做任何校验，对于desktop文件不规范的应用有可能会造成一些问题

    ```text
        /usr/bin/test /host/%U --> /usr/bin/test /host//home/user/Desktop/example.txt
    ```

在这种情况下应用是否能启动由应用自己保证。
5. 本实现会扩展在包裹参数内的字段代码，是否能成功启动不做任何保证。
