RFC 2445 says that the UNTIL in an RRULE needs to be given in UTC. The test 
files in this directory check for this.

For example, in the following RRULE the recurrence on Jan 31, 2000, 09:00 
local time would be after Jan 31,2000, 09:00 UTC (=UNTIL). So the last 
occurrence is on jan 30, 2000:
  DTSTART;TZID=America/Los_Angeles:19980101T090000
  RRULE:FREQ=YEARLY;UNTIL=20000131T090000Z;INTERVAL=1;BYDAY=SU,MO,TU,WE,TH,FR,SA;BYMONTH=1

Similarly, in this example the UNTIL in UTC is actually before 12:00 local 
time, so the only occurrence is at 09:00:
  DTSTART;TZID=America/Los_Angeles:19970902T090000
  RRULE:FREQ=HOURLY;UNTIL=19970902T170000Z;INTERVAL=3


Several of these test cases are simply copied from other directories of this test suite:
Until_TestCase06.ics: LibICal/LibICal_TestCase14.ics
Until_TestCase05.ics: LibICal/LibICal_TestCase13.ics
Until_TestCase04.ics: LibICal/LibICal_TestCase47.ics
Until_TestCase03.ics: LibICal/LibICal_TestCase22.ics





