

proc main() is
  network
  { addone(1) &
    addone(2) &
    addone(3) &
    addone(4) &
    addone(5) &
    addone(6) &
    addone(7) &
    addone(0)
  }

proc addone(val out) is 
  var a;
  var in;
  while a<400 do
  { a:=in+1; out ! a; 0 ? in }
