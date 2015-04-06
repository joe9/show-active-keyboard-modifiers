#!/bin/sh

# got the below from
#https://unix.stackexchange.com/questions/129159/record-every-keystroke-and-store-in-a-file/129171


xinput test-xi2 --root | perl -lne '
  BEGIN{$"=",";
    open X, "-|", "xmodmap -pke";
    for (<X>) {$k{$1}=$2 if /^keycode\s+(\d+) = (\w+)/}
    print "keycode array: %k";
    open X, "-|", "xmodmap -pm"; <X>;<X>;
    for (<X>) {
      if (/^(\w+)\s+(\w*)/){
        ($k=$2)=~s/_[LR]$//;
        $m[$i++]=$k||$1;
      }
    }
  }
  if (/^EVENT type.*\((.*)\)/) {$e = $1}
  elsif (/detail: (\d+)/) {$d=$1}
  elsif (/modifiers:.*effective: (.*)/) {
    $m=$1;
    if ($e =~ /^Key/){
      my @mods;
      print "event : $e, detail: $d, effective: $m, @m";
      for (0..$#m) {
        push @mods, $m[$_] if (hex($m) & (1<<$_))
      }
      print "$e $d [$k{$d}] $m [@mods]"
    }
  }'
