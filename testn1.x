

proc main() is
  var out;
  var in;
  network
  { addone(1) &
    addone(2) &
    addone(3) &
    addone(4) &
    addone(5) &
    addone(6) &
    addone(7) &
    addone(8)
  }

proc addone(val out) is 
  var x;
  while true do
  { out ! x + 1; 0 ? x }


