// ------------------------------
// Plural handling for real numbers.
// First argument should be an actual number, not its formatted string
// (i.e. do not call as $[iolra %1 ...] but as $[iolra ^1 ...]).
function real_plural (rnum, form0, form1, form2, form3)
{
    var n = (rnum > 0 ? Math.floor(rnum) : Math.ceil(rnum));
    var form = (  (n == 1 || n == 11) ? form0
                : (n == 2 || n == 12) ? form1
                : (n > 2 && n < 20) ? form2
                : form3);
    return form;
}
Ts.setcall("iolra", real_plural)

