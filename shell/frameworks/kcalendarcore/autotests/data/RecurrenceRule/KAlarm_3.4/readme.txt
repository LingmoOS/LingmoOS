These files are intended to check which things break on an upgrade and 
if our compat classes are working. 
The files cover all different cases of recurrence rules that could be 
produced by KAlarm from KDE 3.4.


The reference data are the occurrences that were generated with 
KOrganizer from KDE 3.4. They do not alway follow the RFC, but we need 
to ensure that the rrule is adjusted by a Compat class 
so that the new version produces the same set of occurrences. 
