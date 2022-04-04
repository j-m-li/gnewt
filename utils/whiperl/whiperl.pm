package whiperl;

use strict;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK);

require Exporter;
require DynaLoader;
require AutoLoader;

@ISA = qw(Exporter DynaLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
$VERSION = '0.06';

bootstrap whiperl $VERSION;

# Preloaded methods go here.

$whiperl::INFOBOX = 1;
$whiperl::MSGBOX = 2;
$whiperl::YESNO = 3;
$whiperl::CHECKLIST = 4;
$whiperl::INPUTBOX = 5;
$whiperl::RADIOLIST = 6;
$whiperl::MENU = 7;
$whiperl::GAUGE = 8;

$whiperl::NOCANCEL = 2;
$whiperl::NOITEM = 1;
$whiperl::SCROLL_TEXT = 4;
$whiperl::DEFAULT_NO = 8;

# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

whiperl - Perl extension for whiptail

=head1 SYNOPSIS

  	use whiperl;
	whiperl::Init();
	FullButtons (full);
	whiperl::Cmd (mode, flags, backtitle, title, text, width, height, ... );
	whiperl::Finish();

=head1 DESCRIPTION

Perl module with whiptail features

=head1 AUTHOR

 O'ksi'D , nickasil@linuxave.net

=head1 SEE ALSO

perl(1).

=cut
