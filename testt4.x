
var in;
var loop;

proc main() is
  network
  { { 1 ? in; 1 ? in; 1 ? in } &
    { 0 ! loop; 0 ! loop; loop := 0; while loop<20 do loop := loop + 1; 0 ! loop }
  }