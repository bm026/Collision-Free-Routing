
proc main() is
    var in;
    var out;
    network
    { { out := 23; 1 ! out } &
      { 0 ? in             }
    }

proc addtwo(val x) is
    var y;
    y := x+2;
