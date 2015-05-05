

proc main() is
  network
  { addone(7, 1) &
    addone(0, 2) &
    addone(1, 3) &
    addone(2, 4) &
    addone(3, 5) &
    addone(4, 6) &
    addone(5, 7) &
    addone(6, 0)
  }

proc addone(val in, val out) is 
  var a;
  var pin;
  while a<300 do
  { a:=pin+1; out ! a; in ? pin }
