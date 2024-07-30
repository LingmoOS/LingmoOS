config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 2,
    syntax: 'ruby',
    indentationMode: 'ruby',
    selection: '',
});

testCase('ruby:array', () => {
    type('\n'   , '  array = [ :a, :b, :c ]'
                , '  array = [ :a, :b, :c ]\n  ');

    sequence(         c`>   array = [`, () => {

        type('\n'   , c`>   array = [
                        >     `);

        type(':a,\n', c`>   array = [
                        >     :a,
                        >     `);

        type(':b\n' , c`>   array = [
                        >     :a,
                        >     :b
                        >   `);

        type(']'    , c`>   array = [
                        >     :a,
                        >     :b
                        >   ]`);

        type('\n'   , c`>   array = [
                        >     :a,
                        >     :b
                        >   ]
                        >   `);
    });

    for (const sp of ['', ' ']) {
        type('\n'   , c`>   array = [${sp}:a,
                        >            ${sp}:b,
                        >            ${sp}:c]`

                    , c`>   array = [${sp}:a,
                        >            ${sp}:b,
                        >            ${sp}:c]
                        >   `);

        sequence(         c`>   array = [${sp}:a,`, () => {

            type('\n'   , c`>   array = [${sp}:a,
                            >            ${sp}`);

            type(':b,\n', c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}`);

            type(':c\n' , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}:c
                            >           `);

            type(']'    , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}:c
                            >           ]`);
        });

        sequence(         c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}:c`, () => {

            type(']'    , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}:c]`);

            type('\n'   , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}:c]
                            >   `);
        });

        sequence(         c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,`, () => {

            type('\n'   , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}`);

            type(':bar,\n'
                        , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}`);

            type(':baz' , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz`);

            type(sp+']',  c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz${sp}]`);

            type(',\n'  , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz${sp}],
                            >            ${sp}`);

            type(':bar\n'
                        , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz${sp}],
                            >            ${sp}:bar
                            >           `);

            type(']'    , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz${sp}],
                            >            ${sp}:bar
                            >           ]`);

            type('\n'   , c`>   array = [${sp}:a,
                            >            ${sp}:b,
                            >            ${sp}[${sp}:foo,
                            >            ${sp} ${sp}:bar,
                            >            ${sp} ${sp}:baz${sp}],
                            >            ${sp}:bar
                            >           ]
                            >   `);
        });
    }
});

testCase('ruby:array-comment', () => {
    type('\n'   , c`>   array = [ :a, :b, :c ] # comment
                    >                          # comment`

                , c`>   array = [ :a, :b, :c ] # comment
                    >                          # comment
                    >   `);

    type('\n'   , c`>   array = [ # comment`

                , c`>   array = [ # comment
                    >     `);

    type('\n'   , c`>   array = [ # comment
                    >     :a,     # comment`

                , c`>   array = [ # comment
                    >     :a,     # comment
                    >     `);

    type('\n'   , c`>   array = [
                    >     :a,
                    >     :b # comment`

                , c`>   array = [
                    >     :a,
                    >     :b # comment
                    >   `);

    sequence(         c`>   array = [
                        >     :a,
                        >     :b # comment,`, () => {

        type('\n',    c`>   array = [
                        >     :a,
                        >     :b # comment,
                        >   `);

        type(']',     c`>   array = [
                        >     :a,
                        >     :b # comment,
                        >   ]`);
    });
});

sequence('ruby:basic1'
                , c`> # basic1.txt`, () => {

    type('\n'   , c`> # basic1.txt
                    > `);

    type('d'    , c`> # basic1.txt
                    > d`);

    type('e'    , c`> # basic1.txt
                    > de`);

    type('f'    , c`> # basic1.txt
                    > def`);

    type(' f'   , c`> # basic1.txt
                    > def f`);

    type('oo\n' , c`> # basic1.txt
                    > def foo
                    >   `);

    type('i'    , c`> # basic1.txt
                    > def foo
                    >   i`);

    type('f'    , c`> # basic1.txt
                    > def foo
                    >   if`);

    type(' ge'  , c`> # basic1.txt
                    > def foo
                    >   if ge`);

    type('ts'   , c`> # basic1.txt
                    > def foo
                    >   if gets`);

    type('\n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     `);

    type('pu'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     pu`);

    type('ts'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts`);

    type('\n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >     `);

    type('e'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >     e`);

    type('l'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >     el`);

    type('s'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >     els`);

    type('e'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else`);

    type('\n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     `);

    type('e'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     e`);

    type('xi'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exi`);

    type('t\n'  , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >     `);

    type('e'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >     e`);

    type('n'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >     en`);

    type('d'    , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end`);

    type('\n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end
                    >   `);

    type('e'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end
                    >   e`);

    type('n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end
                    >   en`);

    type('d'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end
                    > end`);

    type('\n'   , c`> # basic1.txt
                    > def foo
                    >   if gets
                    >     puts
                    >   else
                    >     exit
                    >   end
                    > end
                    > `);
});

sequence('ruby:basic2'
                , c`> # basic2.txt
                    > `, () => {

    type('\n'   , c`> # basic2.txt
                    >
                    > `);

    type('c'    , c`> # basic2.txt
                    >
                    > c`);

    type('l'    , c`> # basic2.txt
                    >
                    > cl`);

    type('as'   , c`> # basic2.txt
                    >
                    > clas`);

    type('s'    , c`> # basic2.txt
                    >
                    > class`);

    type(' MyCl', c`> # basic2.txt
                    >
                    > class MyCl`);

    type('as'   , c`> # basic2.txt
                    >
                    > class MyClas`);

    type('s'    , c`> # basic2.txt
                    >
                    > class MyClass`);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   `);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   `);

    type('attr' , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr`);

    type('_r'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_r`);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_re`);

    type('ad'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_read`);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reade`);

    type('r'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader`);

    type(' :f'  , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :f`);

    type('oo\n' , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   `);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   `);

    type('pr'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   pr`);

    type('i'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   pri`);

    type('vate' , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private`);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   `);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   `);

    type('d'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   d`);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   de`);

    type('f'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def`);

    type(' h'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def h`);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def he`);

    type('l'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def hel`);

    type('pe'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helpe`);

    type('r'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper`);

    type('(s'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(s`);

    type('tr'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str`);

    type(')'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)`);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     `);

    type('pu'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     pu`);

    type('ts'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts`);

    type(' \"h' , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "h`);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "he`);

    type('l'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "hel`);

    type('pe'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helpe`);

    type('r'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper`);

    type('(#{s' , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{s`);

    type('tr'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str`);

    type('})"\n', c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >     `);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >     e`);

    type('n'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >     en`);

    type('d'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end`);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   `);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   ${''}
                    >   `);

    type('e'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   ${''}
                    >   e`);

    type('n'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   ${''}
                    >   en`);

    type('d'    , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   ${''}
                    > end`);

    type('\n'   , c`> # basic2.txt
                    >
                    > class MyClass
                    >   ${''}
                    >   attr_reader :foo
                    >   ${''}
                    >   private
                    >   ${''}
                    >   def helper(str)
                    >     puts "helper(#{str})"
                    >   end
                    >   ${''}
                    > end
                    > `);
});

sequence('ruby:basic3'
                , c`> def foo
                    >     if check
                    >        bar`, () => {

    type('\n'   , c`> def foo
                    >     if check
                    >        bar
                    >        `);

    type('e'    , c`> def foo
                    >     if check
                    >        bar
                    >        e`);

    type('n'    , c`> def foo
                    >     if check
                    >        bar
                    >        en`);

    type('d'    , c`> def foo
                    >     if check
                    >        bar
                    >     end`);

    type('\n'   , c`> def foo
                    >     if check
                    >        bar
                    >     end
                    >     `);

    type('e'    , c`> def foo
                    >     if check
                    >        bar
                    >     end
                    >     e`);

    type('n'    , c`> def foo
                    >     if check
                    >        bar
                    >     end
                    >     en`);

    type('d'    , c`> def foo
                    >     if check
                    >        bar
                    >     end
                    > end`);

    type('\n'   , c`> def foo
                    >     if check
                    >        bar
                    >     end
                    > end
                    > `);
});

sequence({name :'ruby:basic4', cursor: ''}
                , c`> def foo
                    >     array.each do |v|
                    >        bar`, () => {

    type('\n'   , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >        `);

    type('e'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >        e`);

    type('n'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >        en`);

    type('d'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end`);

    type('\n'   , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end
                    >     `);

    type('e'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end
                    >     e`);

    type('n'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end
                    >     en`);

    type('d'    , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end
                    > end`);

    type('\n'   , c`> def foo
                    >     array.each do |v|
                    >        bar
                    >     end
                    > end
                    > `);
});

sequence('ruby:basic5'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   |
                    > end`, () => {

    type('pu'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   pu|
                    > end`);

    type('bl'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   publ|
                    > end`);

    type('i'    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   publi|
                    > end`);

    type('c'    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public|
                    > end`);

    type(' d'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public d|
                    > end`);

    type('e'    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public de|
                    > end`);

    type('f'    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def|
                    > end`);

    type(' f'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def f|
                    > end`);

    type('oo\n' , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     |
                    > end`);

    type('puts "in foo"\n'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >     |
                    > end`);

    type('end'  , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end|
                    > end`);

    type('\n'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   |
                    > end`);

    type('\n'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   |
                    > end`);

    type('protected def bar(x)\n'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     |
                    > end`);

    type('puts "in bar with #{x}"\n'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >     |
                    > end`);

    type('end'  , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end|
                    > end`);

    type('\n'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   |
                    > end`);

    type('\n'   , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   |
                    > end`);

    type('privat'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   privat|
                    > end`);

    type('e'    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   private|
                    > end`);

    type(' def bat y\n'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   private def bat y
                    >     |
                    > end`);

    type('puts "in bat with #{y}"\n'
                , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   private def bat y
                    >     puts "in bat with #{y}"
                    >     |
                    > end`);

    type('end'  , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   private def bat y
                    >     puts "in bat with #{y}"
                    >   end|
                    > end`);

    type(' '    , c`> # basic5.txt - access modifiers before \`def\`
                    >
                    > class MyClass
                    >   public def foo
                    >     puts "in foo"
                    >   end
                    >   ${''}
                    >   protected def bar(x)
                    >     puts "in bar with #{x}"
                    >   end
                    >   ${''}
                    >   private def bat y
                    >     puts "in bat with #{y}"
                    >   end |
                    > end`);
});

testCase('ruby:block', () => {
    sequence(         c`> 10.times {`, () => {

        type('\n'   , c`> 10.times {
                        >   `);

        xtype('foo\n',c`> 10.times {
                        >   foo
                        >   `, 'Multiline blocks using {} is not supported');

        type('}'    , c`> 10.times {
                        >   foo
                        > }`);
    });

    sequence(         c`> 10.times {
                        >   if foo`, () => {

        xtype('\n'  , c`> 10.times {
                        >   if foo
                        >     `, 'Multiline blocks using {} is not supported');

        xtype('foo\n',c`> 10.times {
                        >   if foo
                        >     foo
                        >     `);

        type('}'    , c`> 10.times {
                        >   if foo
                        >     foo
                        > }`);
    });
});

testCase('ruby:block-comment', () => {
    sequence(         c`> =begin`, () => {

        type('\n'   , c`> =begin
                        > `);

        type('\n'   , c`> =begin
                        > ${''}
                        > `);
    });

    sequence(         c`> =begin
                        > if foo`, () => {

        type('\n'   , c`> =begin
                        > if foo
                        > `);

        type('\n'   , c`> =begin
                        > if foo
                        > ${''}
                        > `);
    });

    sequence(         c`> if foo
                        > =begin`, () => {

        type('\n'   , c`> if foo
                        > =begin
                        > `);

        type('\n'   , c`> if foo
                        > =begin
                        > ${''}
                        > `);
    });

    sequence(         c`> if foo
                        > =begin
                        > =end`, () => {

        type('\n'   , c`> if foo
                        > =begin
                        > =end
                        >   `);

        type('\n'   , c`> if foo
                        > =begin
                        > =end
                        >   ${''}
                        >   `);
    });
});

testCase('ruby:comment', () => {
    sequence(         c`> if foo
                        >   # Comment`, () => {

        type('\n'   , c`> if foo
                        >   # Comment
                        >   `);

        type('end'  , c`> if foo
                        >   # Comment
                        > end`);
    });

    sequence(         c`> if foo
                        >   # Comment
                        >   `, () => {

        type('bar\n', c`> if foo
                        >   # Comment
                        >   bar
                        >   `);

        type('end'  , c`> if foo
                        >   # Comment
                        >   bar
                        > end`);
    });

    sequence(         c`> if foo
                        >   # Comment
                        >   bar
                        >   # Another comment`, () => {

        type('\n'   , c`> if foo
                        >   # Comment
                        >   bar
                        >   # Another comment
                        >   `);

        type('end'  , c`> if foo
                        >   # Comment
                        >   bar
                        >   # Another comment
                        > end`);
    });

    sequence(         c`> if foo
                        >   # Comment
                        >   bar # Another comment`, () => {

        type('\n'   , c`> if foo
                        >   # Comment
                        >   bar # Another comment
                        >   `);

        type('end'  , c`> if foo
                        >   # Comment
                        >   bar # Another comment
                        > end`);
    });

    type('\n'   , c`> if foo
                    >      # Misplaced comment`

                , c`> if foo
                    >      # Misplaced comment
                    >   `);

    type('\n'   , c`> if foo
                    > # Misplaced comment`

                , c`> if foo
                    > # Misplaced comment
                    >   `);
});

testCase({name: 'ruby:do', cursor: ''}, () => {

    sequence(         c`> 5.times do`, () => {

        type('\n'   , c`> 5.times do
                        >   `);

        type('end'  , c`> 5.times do
                        > end`);
    });

    sequence(         c`> File.open("file") do |f|`, () => {

        type('\n'   , c`> File.open("file") do |f|
                        >   `);

        type('f << foo\n'
                    , c`> File.open("file") do |f|
                        >   f << foo
                        >   `);

        type('end'  , c`> File.open("file") do |f|
                        >   f << foo
                        > end`);
    });

    sequence(         c`> [1,2,3].each_with_index do |obj, i|`, () => {

        type('\n'   , c`> [1,2,3].each_with_index do |obj, i|
                        >   `);

        type('puts "#{i}: #{obj.inspect}"\n'
                    , c`> [1,2,3].each_with_index do |obj, i|
                        >   puts "#{i}: #{obj.inspect}"
                        >   `);

        type('end'  , c`> [1,2,3].each_with_index do |obj, i|
                        >   puts "#{i}: #{obj.inspect}"
                        > end`);
    });

    sequence(         c`> def foo(f)
                        >   f.each do # loop`, () => {

        type('\n'   , c`> def foo(f)
                        >   f.each do # loop
                        >     `);

        type('end'  , c`> def foo(f)
                        >   f.each do # loop
                        >   end`);
    });

    sequence(         c`> def foo(f)
                        >   f.each do # loop
                        >     bar`, () => {

        type('\n'   , c`> def foo(f)
                        >   f.each do # loop
                        >     bar
                        >     `);

        type('end'  , c`> def foo(f)
                        >   f.each do # loop
                        >     bar
                        >   end`);
    });

    type('\n'   , c`> def foo(f)
                    >   f.each do # loop with do`

                , c`> def foo(f)
                    >   f.each do # loop with do
                    >     `);
});

sequence('ruby:empty-file', '', () => {

    type('\n'   , c`>
                    > `);

    type('\n'   , c`>
                    >
                    > `);

    type('# Comment\n'
                , c`>
                    >
                    > # Comment
                    > `);

    type('def foo\n'
                , c`>
                    >
                    > # Comment
                    > def foo
                    >   `);

    type('bar\n', c`>
                    >
                    > # Comment
                    > def foo
                    >   bar
                    >   `);

    type('end'  , c`>
                    >
                    > # Comment
                    > def foo
                    >   bar
                    > end`);
});

testCase('ruby:endless', () => {
    type('\n'   , 'def current_time = Time.now'
                , 'def current_time = Time.now\n');

    type('\n'   , c`> class Base
                    >   extend Dry::Initializer`

                , c`> class Base
                    >   extend Dry::Initializer
                    >   `);

    type('\n'   , c`> class Base
                    >   def self.call(...) = new(...).call`

                , c`> class Base
                    >   def self.call(...) = new(...).call
                    >   `);

    type('\n'   , c`> class Base
                    >   def call = raise NotImplementedError`

                , c`> class Base
                    >   def call = raise NotImplementedError
                    >   `);

    type('\n'   , c`> class Base
                    >   def call() = :endless`

                , c`> class Base
                    >   def call() = :endless
                    >   `);

    type('\n'   , c`> class Base
                    >   def compact? =true`

                , c`> class Base
                    >   def compact? =true
                    >   `);

    type('\n'   , c`> class Base
                    >   def expand   (  ts = Time.now )    = ts.year * (rand * 1000)`

                , c`> class Base
                    >   def expand   (  ts = Time.now )    = ts.year * (rand * 1000)
                    >   `);

    sequence(         c`> class Base
                        >   def foo(bar = "baz")`, () => {

        type('\n'   , c`> class Base
                        >   def foo(bar = "baz")
                        >     `);

        type('bar.upcase\n'
                    , c`> class Base
                        >   def foo(bar = "baz")
                        >     bar.upcase
                        >     `);

        type('end'  , c`> class Base
                        >   def foo(bar = "baz")
                        >     bar.upcase
                        >   end`);
    });

    sequence(         c`> class Base
                        >   def foo_blank bar = "baz"`, () => {

        type('\n'   , c`> class Base
                        >   def foo_blank bar = "baz"
                        >     `);

        type('bar.upcase\n'
                    , c`> class Base
                        >   def foo_blank bar = "baz"
                        >     bar.upcase
                        >     `);

        type('end'  , c`> class Base
                        >   def foo_blank bar = "baz"
                        >     bar.upcase
                        >   end`);
    });

    type('\n'   , c`> class Base
                    >   def foo_endless(bar = "baz") = bar.upcase`

                , c`> class Base
                    >   def foo_endless(bar = "baz") = bar.upcase
                    >   `);

    sequence(         c`> class Base
                        >   def width=(other)`, () => {

        type('\n'   , c`> class Base
                        >   def width=(other)
                        >     `);

        type('@width = other\n'
                    , c`> class Base
                        >   def width=(other)
                        >     @width = other
                        >     `);

        type('end'  , c`> class Base
                        >   def width=(other)
                        >     @width = other
                        >   end`);
    });
});

testCase('ruby:hash', () => {
    type('\n'   , c`>   hash = { :a => 1, :b => 2, :c => 3 }`

                , c`>   hash = { :a => 1, :b => 2, :c => 3 }
                    >   `);

    sequence(         c`>   hash = {`, () => {

        type('\n'   , c`>   hash = {
                        >     `);

        type(':a => 1,\n'
                    , c`>   hash = {
                        >     :a => 1,
                        >     `);

        type(':b => 2\n'
                    , c`>   hash = {
                        >     :a => 1,
                        >     :b => 2
                        >   `);

        type('}'    , c`>   hash = {
                        >     :a => 1,
                        >     :b => 2
                        >   }`);
    });

    for (const sp of ['', ' ']) {
        sequence(         c`>   hash = {${sp}:a => 1,`, () => {

            type('\n'   , c`>   hash = {${sp}:a => 1,
                            >           ${sp}`);

            type(':b => 2,\n'
                        , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}`);

            type(`:c => 3${sp}}`
                        , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => 3${sp}}`);

            type('\n'   , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => 3${sp}}
                            >   `);
        });

        sequence(         c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,`, () => {

            type('\n'   , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}`);

            type(':bar => /^b/,\n'
                        , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}:bar => /^b/,
                            >           ${sp}       ${sp}`);

            type(':baz => /^b/\n'
                        , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}:bar => /^b/,
                            >           ${sp}       ${sp}:baz => /^b/
                            >           ${sp}      `);

            type('}'    , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}:bar => /^b/,
                            >           ${sp}       ${sp}:baz => /^b/
                            >           ${sp}      }`);

            type('\n'   , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}:bar => /^b/,
                            >           ${sp}       ${sp}:baz => /^b/
                            >           ${sp}      }
                            >          `);

            type('}'    , c`>   hash = {${sp}:a => 1,
                            >           ${sp}:b => 2,
                            >           ${sp}:c => {${sp}:foo => /^f/,
                            >           ${sp}       ${sp}:bar => /^b/,
                            >           ${sp}       ${sp}:baz => /^b/
                            >           ${sp}      }
                            >          }`);
        });

        type('\n'   , c`>   hash = {${sp}:a => 1,
                        >           ${sp}:b => 2,
                        >           ${sp}:c => {${sp}:foo => /^f/,
                        >           ${sp}       ${sp}:bar => /^b/${sp}},`

                    , c`>   hash = {${sp}:a => 1,
                        >           ${sp}:b => 2,
                        >           ${sp}:c => {${sp}:foo => /^f/,
                        >           ${sp}       ${sp}:bar => /^b/${sp}},
                        >           ${sp}`);
    }
});

testCase('ruby:heredoc', () => {
    sequence(         c`> doc = <<EOF`, () => {

        type('\n'   , c`> doc = <<EOF
                        > `);

        type('\n'   , c`> doc = <<EOF
                        >
                        > `);
    });

    sequence(         c`> doc = <<EOF
                        > if foo`, () => {

        type('\n'   , c`> doc = <<EOF
                        > if foo
                        > `);

        type('\n'   , c`> doc = <<EOF
                        > if foo
                        >
                        > `);
    });

    sequence(         c`> if foo
                        > doc = <<EOF`, () => {

        type('\n'   , c`> if foo
                        > doc = <<EOF
                        > `);

        type('\n'   , c`> if foo
                        > doc = <<EOF
                        >
                        > `);
    });
});

testCase('ruby:if', () => {
    sequence(         c`>   if foo`, () => {

        type('\n'   , c`>   if foo
                        >     `);

        type('blah\n'
                    , c`>   if foo
                        >     blah
                        >     `);

        type('end'  , c`>   if foo
                        >     blah
                        >   end`);
    });

    sequence(         c`>   var = if foo`, () => {

        xtype('\n'  , c`>   var = if foo
                        >     `
                    , 'multi line if assignment is not supported');

        type('blah\n'
                    , c`>   var = if foo
                        >     blah
                        >     `);

        xtype('end' , c`>   var = if foo
                        >     blah
                        >   end`);
    });

    type('\n'   , c`>   var = bar if foo`

                , c`>   var = bar if foo
                    >   `);

    xtype('\n'  , c`>   if foo; 42 else 37 end`

                , c`>   if foo; 42 else 37 end
                    >   `
                , 'single line if is not supported');

    xtype('\n'  , c`>   if foo then 42 else 37 end`

                , c`>   if foo then 42 else 37 end
                    >   `
                , 'single line if is not supported');
});

testCase('ruby:indentpaste', () => {
    cmd([paste
        , c` >     if true
             >         puts "World!"
             >     end`]

        , c`> def hello
            >   puts "Hello!"
            >   |
            > end`

        , c`> def hello
            >   puts "Hello!"
            >   if true
            >     puts "World!"
            >   end|
            > end`);

    cmd([paste
        , c` > a = ["a",
             > "b",
             > "c"]`]

        , c`> def hello
            >   puts "Hello!"
            >   |
            > end`

        , c`> def hello
            >   puts "Hello!"
            >   a = ["a",
            >        "b",
            >        "c"]|
            > end`);
})

testCase('ruby:multiline', () => {
    sequence(         c`> if (foo == "bar" and
                        >     bar == "foo")`, () => {

        type('\n'   , c`> if (foo == "bar" and
                        >     bar == "foo")
                        >   `);

        type('end'  , c`> if (foo == "bar" and
                        >     bar == "foo")
                        > end`);
    });

    sequence(         c`> if (foo == "bar" and
                        >     bar == "foo")
                        >   puts`, () => {

        type('\n'   , c`> if (foo == "bar" and
                        >     bar == "foo")
                        >   puts
                        >   `);

        type('end'  , c`> if (foo == "bar" and
                        >     bar == "foo")
                        >   puts
                        > end`);
    });

    for (comment of [
        '',
        '\n>     # Comment',
        '\n> # Misplaced comment'
    ]) {
        sequence(         c`> s = "hello" +${comment}`, () => {

            type('\n'   , c`> s = "hello" +${comment}
                            >     `);

            type('"world"\n'
                        , c`> s = "hello" +${comment}
                            >     "world"
                            > `);
        });
    }

    sequence(       c`> foo "hello" \\`, () => {

        type('\n'   , c`> foo "hello" \\
                        >     `);

        type('\n'   , c`> foo "hello" \\
                        >     ${''}
                        > `);
    });

    type('\n'   , c`> foo "hello",`

                , c`> foo "hello",
                    >     `);

    type('\n'   , c`> def foo(array)
                    >   array.each_with_index \\
                    >       do`

                , c`> def foo(array)
                    >   array.each_with_index \\
                    >       do
                    >     `);

    type('\n'   , c`> if test \\
                    > # if ends here
                    >   foo do`

                , c`> if test \\
                    > # if ends here
                    >   foo do
                    >     `);

    type('\n'   , c`> if test1 &&
                    > # still part of condition
                    >    test2`

                , c`> if test1 &&
                    > # still part of condition
                    >    test2
                    >   `);
});

testCase('ruby:no-do', () => {
    for (const nothing of [
        '# nothing to do',
        'puts "nothing" # nothing to do'
    ]) {
        sequence(         c`> if foo
                            >   ${nothing}`, () => {

            type('\n'   , c`> if foo
                            >   ${nothing}
                            >   `);

            type('end'  , c`> if foo
                            >   ${nothing}
                            > end`);
        });

        sequence(         c`> if foo
                            >   ${nothing}
                            >   `, () => {

            type('f\n'  , c`> if foo
                            >   ${nothing}
                            >   f
                            >   `);

            type('end'  , c`> if foo
                            >   ${nothing}
                            >   f
                            > end`);
        });
    }
});

testCase('ruby:ops', () => {
    for (const op of "+-*/") {
        for (const comm1 of [
            '',
            ' # Comment'
        ]) {
            for (const comm2 of [
                '',
                ' # Comment'
            ]) {
                sequence(c`> t = foo() +${comm1}`, () => {

                    type('\n'   , c`> t = foo() +${comm1}
                                    >     `);

                    type(`bar()${comm2}\n`
                                , c`> t = foo() +${comm1}
                                    >     bar()${comm2}
                                    > `);
                });
            }
        }
    }
});

testCase('ruby:plist', () => {
    for (sp of ['', ' ']) {
        sequence(         c`>   foobar(${sp}foo,`, () => {

            type('\n'   , c`>   foobar(${sp}foo,
                            >          ${sp}`);

            type(`bar${sp})\n`
                        , c`>   foobar(${sp}foo,
                            >          ${sp}bar${sp})
                            >   `);
        });

        sequence(         c`>   foobar(${sp}foo,
                            >          ${sp}`, () => {

            type('bar,\n'
                        , c`>   foobar(${sp}foo,
                            >          ${sp}bar,
                            >          ${sp}`);

            type(`baz${sp})\n`
                        , c`>   foobar(${sp}foo,
                            >          ${sp}bar,
                            >          ${sp}baz${sp})
                            >   `);
        });

        type('\n'   , c`>   foobar(${sp}foo, foo,`

                    , c`>   foobar(${sp}foo, foo,
                        >          ${sp}`);

        type('\n'   , c`>   foobar(${sp}foo,
                        >          ${sp}bar, bar,`

                    , c`>   foobar(${sp}foo,
                        >          ${sp}bar, bar,
                        >          ${sp}`);

        sequence(         c`>   foobar(${sp}foo(${sp}bar,`, () => {

            type('\n'   , c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}`);

            type(`baz${sp}),\n`
                        , c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}baz${sp}),
                            >          ${sp}`);

            type(`blah${sp})\n`
                        , c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}baz${sp}),
                            >          ${sp}blah${sp})
                            >   `);
        });

        sequence(         c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}baz${sp}),
                            >          ${sp}`, () => {

            type('foobaz(),\n'
                        , c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}baz${sp}),
                            >          ${sp}foobaz(),
                            >          ${sp}`);

            type(`blah${sp})\n`
                        , c`>   foobar(${sp}foo(${sp}bar,
                            >          ${sp}    ${sp}baz${sp}),
                            >          ${sp}foobaz(),
                            >          ${sp}blah${sp})
                            >   `);
        });
    }
});

testCase('ruby:plist-comment', () => {
    type('\n'   , c`>   foobar(foo, # comment`

                , c`>   foobar(foo, # comment
                    >          `);

    type('\n'   , c`>   foobar(foo, foo, # comment`

                , c`>   foobar(foo, foo, # comment
                    >          `);

    type('\n'   , c`>   foobar(foo,
                    >          # comment
                    >          bar) # comment`

                , c`>   foobar(foo,
                    >          # comment
                    >          bar) # comment
                    >   `);

    type('\n'   , c`>   foobar(foo,
                    >          bar, # comment
                    >          # comment`

                , c`>   foobar(foo,
                    >          bar, # comment
                    >          # comment
                    >          `);

    type('\n'   , c`>   foobar(foo,
                    >          bar, bar, # comment
                    >                    # comment`

                , c`>   foobar(foo,
                    >          bar, bar, # comment
                    >                    # comment
                    >          `);

    type('\n'   , c`>   foobar(foo,
                    >          bar,
                    >          baz) # comment,`

                , c`>   foobar(foo,
                    >          bar,
                    >          baz) # comment,
                    >   `);

    type('\n'   , c`>   foobar(foo(bar, # comment
                    >                   # comment`

                , c`>   foobar(foo(bar, # comment
                    >                   # comment
                    >              `);

    type('\n'   , c`>   foobar(foo(bar,
                    >   # comment
                    >              baz), # comment`

                , c`>   foobar(foo(bar,
                    >   # comment
                    >              baz), # comment
                    >          `);

    type('\n'   , c`>   foobar(foo(bar,
                    >              baz), # comment
                    >          foobaz(), # comment
                    >                    # comment`

                , c`>   foobar(foo(bar,
                    >              baz), # comment
                    >          foobaz(), # comment
                    >                    # comment
                    >          `);

    type('\n'   , c`>   foobar(foo(bar,
                    >              baz),
                    >        # comment
                    >          blah)
                    >        # comment`

                , c`>   foobar(foo(bar,
                    >              baz),
                    >        # comment
                    >          blah)
                    >        # comment
                    >   `);
});

testCase('ruby:regex', () => {
    xtype('\n'  , c`>   rx =~ /^hello/`

                , c`>   rx =~ /^hello/
                    >   `
                , 'regression, inside already in commit afc551d14225023ce38900ddc49b43ba2a0762a8');
});

testCase('ruby:singleton', () => {
    xtype('\n'  , c`>   def foo() 42 end`

                , c`>   def foo() 42 end
                    >   `
                , 'Single line defs are not supported');

    xtype('\n'  , c`>   def foo; 42 end`

                , c`>   def foo; 42 end
                    >   `
                , 'Single line defs are not supported');

    for (const sep of ['()', ';']) {
        sequence(         c`>   def foo${sep} bar`, () => {

            type('\n'   , c`>   def foo${sep} bar
                            >     `);

            type('blah\n'
                        , c`>   def foo${sep} bar
                            >     blah
                            >     `);

            type('end'  , c`>   def foo${sep} bar
                            >     blah
                            >   end`);
        });
    }
});

testCase({name: 'ruby:wordlist', cursor: ''}, () => {
    for (const s of ['[]', '<>', '||']) {
        sequence(         c`>   for elem in %w[ foo, bar,`.replace('[', s[0]), () => {

            xtype('\n'  , c`>   for elem in %w[ foo, bar,
                            >                   `.replace('[', s[0])
                        , 'multiline word list is not supported');

            xtype(`foobar ${s[1]}\n`
                        , c`>   for elem in %w[ foo, bar,
                            >                   foobar ]
                            >     `.replace('[', s[0]).replace(']', s[1])
                        , 'multiline word list is not supported');
        })
    }
});
