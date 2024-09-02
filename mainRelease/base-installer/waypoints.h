/*
 * debootstrap waypoints for run-debootstrap. See the README for docs.
 */
struct waypoint {
	int startpercent;
	int endpercent;
	char *progress_id;
};
static struct waypoint waypoints[] = {
	{ 0,	0,	"START" },	/* dummy entry, required */
	{ 0,	1,	"DOWNREL" },	/* downloading release files; very quick */
	{ 1,	5,	"DOWNPKGS" },	/* downloading packages files; time varies
		                           by bandwidth (and size); low granularity */
	{ 5,	10,	"SIZEDEBS" },   /* getting packages sizes; high granularity */
	{ 10,	25,	"DOWNDEBS" },   /* downloading packages; run time varies by
					   bandwidth; high granularity */
	{ 25,	45,	"EXTRACTPKGS" },/* extracting the core packages */

	/* old debootstrap with poor granularity */
	{ 45,	100,	"INSTBASE" },	/* installing the base system */

	/* new debootstrap with better granularity */
	{ 45,	50,	"INSTCORE" },	/* installing packages needed for dpkg to
					   work */
	{ 50,	60,	"UNPACKREQ" },	/* unpacking required packages */
	{ 60,	70,	"CONFREQ" },	/* configuring required packages */
	{ 70,	85,	"UNPACKBASE" },	/* unpacking the rest of the base system */
	{ 85,	100,	"CONFBASE" },	/* configuring the rest of the base system */

	{ 100,	0,	NULL },		/* last entry, required */
};
