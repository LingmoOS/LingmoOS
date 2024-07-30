config({
    blockSelection: false,
    replaceTabs: true,
    tabWidth: 4,
    indentationWidth: 3,
    syntax: 'replicode',
    indentationMode: 'replicode',
    selection: '',
});

testCase('replicode:return', () => {
    type('\n'   , '[]|mod'
                , '[]\n   |mod');
});
