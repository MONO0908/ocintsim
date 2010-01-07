#!/usr/bin/perl -w

if (exists $ARGV[0]) {
    $layout = $ARGV[0];
} else {
    die "You must specify net layout in (maxX)x(maxY), ie net_gen.pl 4x7 for a 4 by 7 netowrk\n";
    
}

if ($layout =~ /^(\d+)x(\d+)$/) {
    $max_x = $1;
    $max_y = $2;
} else {
    die "You must specify net layout in (maxX)x(maxY), ie net_gen.pl 4x7 for a 4 by 7 netowrk\n";
}

# Populate the data structure
for($x = 0; $x < $max_x; $x++) {
    for($y = 0; $y < $max_y; $y++) {

        $name = "node.".$x.".".$y;

#        print "$name\n";

        #place the coords in it
        $data{$name}{x} = $x;
        $data{$name}{y} = $y;

        # populate the ports part of the data structure

        #east port
        if ($x <($max_x-1)) {
            $e_name = "node.".($x+1).".".$y;
            push @{ $data{$name}{ports} }, $e_name;
        }

        #west port
        if ($x > 0) {
            $w_name = "node.".($x-1).".".$y;
            push @{ $data{$name}{ports} }, $w_name;
        }

        #south port
        if ($y <($max_y-1)) {
            $s_name = "node.".$x.".".($y+1);
            push @{ $data{$name}{ports} }, $s_name;
        }

        #north port
        if ($y > 0) {
            $n_name = "node.".$x.".".($y-1);
            push @{ $data{$name}{ports} }, $n_name;
        }
    }
}

print <<HDREND;
# Default Router type information:
is_source rand
#is_source bitcomp
is_destination noblock
router_type basic
xbar_type fullcon
vc_alloc 2level
xb_alloc 2level
rt_algo_type xydor
#rt_algo_type adaptive_xy
rt_sel_type first_match
#rt_sel_type local
rt_cost_fn buff_nohist
vc_count 4
vc_classes 1
que_depth 4
HDREND


foreach $name (sort(keys %data)) {

    print "\n\n#----------------------------------------\n";

    print "node: $name\n";

    print "\n# X Y coordinates:\n";
    
    print "coord $data{$name}{x}\n";    

    print "coord $data{$name}{y}\n";

    print "\n# Port information:\n";

    foreach $port (@{ $data{$name}{ports} }) {
        print "port $port\n";
    }
}
