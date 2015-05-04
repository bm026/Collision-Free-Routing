
proc main() is
  var var1; var var2; var var3; var var4;
  network 
  { { var1 := 56; 1 ! var1; 2 ! var1; 0 ? var1 } &
    { 0 ? var2; 3 ! var2                       } &
    { 0 ? var3                                 } &
    { 0 ? var4; var4 := addone(var4); 0 ! var4 }
  }

func addone(val x) is return x+1;
