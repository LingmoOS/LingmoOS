#!/usr/bin/perl
# Perl module used by the aggregator programs.

use warnings;
use strict;
use POSIX q{strftime};

sub logpng {
	my $log=shift;
	my $basename=shift;
	my $desc=$log->{description};
	$desc=~s/[^a-zA-Z0-9]//g;
	return "$basename.$desc.png";
}

sub aggregate {
	my $fh=shift;
	my $mailfh=shift;
	my $basename=shift;
	my @buildlist=@_;
	
	my $success=0;
	my $failed=0;
	my $total=0;
	my $old=0;
	
	open (STATS, ">>$basename.stats") || die "$basename.stats: $!";
	print STATS strftime("%m/%d/%Y %H:%M", gmtime);
	
	foreach my $log (@buildlist) {
		my $onesuccess=0;
		my $onefailed=0;
		
		print $fh "<li><a href=\"".$log->{url}."\">".$log->{description}."</a><br>\n";
		if (length $log->{notes}) {
			print $fh "<b>Notes: ".$log->{notes}."</b><br>\n";
			print $mailfh "* ".$log->{description}.":\n".$log->{notes}."\n\n"
				if defined $mailfh;
		}
		my ($basebasename)=($basename)=~m/(?:.*\/)?(.*)/;
		print $fh "<img src=\"".logpng($log, $basebasename)."\" alt=\"graph\">\n";
		my $logurl=$log->{logurl}."overview".(exists $log->{overviewext} ? $log->{overviewext} : $log->{logext})."\n";
		if ($logurl=~m#.*://#) {
			if (! open (LOG, "wget --tries=3 --timeout=5 --quiet -O - $logurl |")) {
				print $fh "<b>wget error</b>\n";
			}
		}
		else {
			open (LOG, $logurl) || print $fh "<b>cannot open $logurl</b>\n";
		}
		my @lines=<LOG>;
		if (! close LOG) {
			print $fh "<p><b>failed</b> to download <a href=\"$logurl\">summary log</a></p>\n";
			print STATS "\t0";
			next;
		}
		if (! @lines) {
			print $fh "<b>empty</b> <a href=\"$logurl\">log</a>";
		}
		else {
			print $fh "<ul>\n";
		}
		foreach my $line (@lines) {
			chomp $line;
			my ($arch, $date, $builder, $ident, $status, $notes) = 
				$line =~ /^(.*?)\s+\((.*?)\)\s+(.*?)\s+(.*?)\s+(.*?)(?:\s+(.*))?$/;
			if (! defined $status) {
				print $fh "<li><b>unparsable</b> entry:</i> $line\n";
			}
			else {
				if (defined $notes && length $notes) {
					$notes="($notes)";
				}
				else {
					$notes="";
				}
				$date=~s/[^A-Za-z0-9: ]//g; # untrusted string
				# date -u seems to generate stuff that date -d doesn't consume, so
				# work around this by switching day and month if appropriate:
				$date=~s/^(\S+) +(\d+) +(\D+) +(.*)/$1 $3 $2 $4/;
				my $shortdate=`TZ=GMT LANG=C date -d '$date' '+%b %d %H:%M'`;
				chomp $shortdate;
				if (! length $shortdate) {
					$shortdate=$date;
				}
				my $unixdate=`TZ=GMT LANG=C date -d '$date' '+%s'`;
				if (length $unixdate && 
				    (time - $unixdate) > ($log->{frequency}+1) * 60*60*24)  {
					print $mailfh "* OLD BUILD:    $arch $shortdate $builder $ident $notes\n"
						if defined $mailfh and not length $log->{notes};
					print $mailfh "                $log->{logurl}$ident$log->{logext}\n\n"
						if defined $mailfh and not length $log->{notes};
					$shortdate="<b>$shortdate</b>";
					$old++;
				}
				if ($status eq 'failed') {
					print $mailfh "* FAILED BUILD: $arch $shortdate $builder $ident $notes\n"
						if defined $mailfh;
					print $mailfh "                $log->{logurl}$ident$log->{logext}\n\n"
						if defined $mailfh;
					$status='<b>failed</b>';
					$failed++;
					$onefailed++;
					$total++;
				}
				elsif ($status eq 'success') {
					$success++;
					$onesuccess++;
					$total++;
				}
				print $fh "<li>$arch $shortdate $builder <a href=\"".$log->{logurl}."$ident".$log->{logext}."\">$status</a> $ident $notes\n";
			}
		}
		print $fh "</ul><br>\n";
		print STATS "\t";
		if ($onesuccess+$onefailed > 0) {
			print STATS ($onesuccess / ($onesuccess+$onefailed) * 100);
		}
		else {
			print STATS 0;
		}
	}
	print STATS "\t";
	if ($success+$failed > 0) {
		print STATS $success / ($success+$failed) * 100;
	}
	else {
		print STATS 0;
	}
	print STATS "\n";
	close STATS;
	
	# plot the stats
	my $c=2;
	foreach my $log (@buildlist) {
		my $png=logpng($log, $basename);
		open (GNUPLOT, "| gnuplot") || die "gnuplot: $!";
		print GNUPLOT qq{
			set timefmt "%m/%d/%Y %H:%M"
			set xdata time
			set format x "%m/%y"
			set yrange [-2 to 102]
			set terminal png size 640,128
			set output '$png'
			set key off
		};
		$c++;
		print GNUPLOT "plot '$basename.stats' using 1:$c title \"".$log->{description}."\" with lines";
		close GNUPLOT;
	}
	
	return ($total, $failed, $success, $old);
}

1
