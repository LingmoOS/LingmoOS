config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 4,
    syntax: 'CMake',
    indentationMode: 'cmake',
});

testCase('cmake.enter', () => {
    type('\n'   , c`> some(|)
                    >
                    >     some()`

                , c`> some(
                    >     |
                    >   )
                    >
                    >     some()`);

    type('\n'   , c`>     some()
                    > some(
                    >
                    >   )
                    >
                    >     some(|)`

                , c`>     some()
                    > some(
                    >
                    >   )
                    >
                    >     some(
                    >         |
                    >       )`);

    type('\n'   ,    'some(|SOME TEXT HERE)'

                , c`> some(
                    >     |SOME TEXT HERE)`);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()|
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > |
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()|
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    >     |
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()|
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >         |
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()|
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >             |
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)|
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >                 |
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)|
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             |
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()|
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         |
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)
                    >     elseif(other)|
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >     elseif(other)
                    >         |
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)
                    >     if(smth)|
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     if(smth)
                    >         |
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);

    type('\n'   , c`> function(some)|
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `

                , c`> function(some)
                    >     |
                    >     if(smth)
                    >     elseif(other)
                    >     else()
                    >         foreach(foo)
                    >             while(bar)
                    >             endwhile()
                    >         endforeach()
                    >     endif()
                    > endfunction()
                    > `);
});

testCase('cmake.paren', () => {
    type('('    , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >             endif
                    >             endif
                    >             endfunction|
                    > // The last line is failed for some reason ;-(`

                , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >             endif
                    >             endif
                    > endfunction(|
                    > // The last line is failed for some reason ;-(`);

    type('('    , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >             endif
                    >             endif|
                    > endfunction(
                    > // The last line is failed for some reason ;-(`

                , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >             endif
                    >     endif()|
                    > endfunction(
                    > // The last line is failed for some reason ;-(`);

    type('('    , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >             endif|
                    >     endif()
                    > endfunction(
                    > // The last line is failed for some reason ;-(`

                , c`> function(some)
                    >     if(smth)
                    >         if(some_other)
                    >             set(foo)
                    >         endif()|
                    >     endif()
                    > endfunction(
                    > // The last line is failed for some reason ;-(`);
});

testCase('cmake.quot', () => {
    type('"'    , 'some(|)\n'
                , 'some("|")\n');

    type('"'    , 'some(|${})\n'
                , 'some("|${})\n');

    type('"'    , 'some(|"")\n'
                , 'some("|")\n');

    type('"'    , c`> some(
                    >     |
                    >     \${some}
                    >   )`

                 , c`> some(
                    >     "|"
                    >     \${some}
                    >   )`);

    type('"'    , c`> some(
                    >     ""
                    >     |\${some}
                    >   )`

                , c`> some(
                    >     ""
                    >     "|\${some}
                    >   )`);
});

testCase('cmake.var', () => {
    type('$'    , 'some(|)'
                , 'some(${|})');

    type('<'    , 'some(${|})'
                , 'some($<|>)');

    type('{'    , 'some($<|>)'
                , 'some(${|})');

    type('$'    , 'some(|{})'
                , 'some(${|})');
});
