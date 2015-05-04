
proc main() is
    var in;
    var out;
    var num;
    network
    { { out := 23; 1 ! out } &
      { 0 ? in             } &
      { num := addone(45)  } &
      { addtwo(67)         }
    }

func addone(val x) is return x+1

proc addtwo(val x) is
    var y;
    y := x+2;
