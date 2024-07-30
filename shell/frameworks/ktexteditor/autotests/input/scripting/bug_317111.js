testCase('bug_317111', () => {
    function defStyleNumWithInvalideCoordinate() {
        document.defStyleNum(-1,0);
        document.defStyleNum(0,-1);
        document.defStyleNum(1000,0);
        document.defStyleNum(0,1000);
        document.defStyleNum(0,0);
    }
    config({blockSelection: false});
    cmd(defStyleNumWithInvalideCoordinate, 'blah blah\n', 'blah blah\n');
});
