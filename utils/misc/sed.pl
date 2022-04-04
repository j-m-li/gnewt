#!/usr/bin/perl

$files = qx{ls -1 *.c};
@files = split('\n', $files);
foreach (@files) {
	qx{mv ${_} ${_}.old};
	open (FIN, "< ${_}.old");
	open (FOUT, "> $_");
	while (<FIN>) {
		$_ =~ s/globalColors/gnewt->globalColors/g;
		print FOUT $_;
	}
	close (FOUT);
	close (FIN)
}
	
