
var in;

proc main() is
  network
  { { 7 ? in; 6 ? in; 5 ? in; 4 ? in; 3 ? in; 2 ? in; 1 ? in; 1 ? in; 1 ! in; 1 ? in } &
    { 0 ! 3; 0 ! 2; 0 ! 1; 0 ? in } &
    { 0 ! 4 } &
    { 0 ! 5 } &
    { 0 ! 6 } &
    { 0 ! 7 } &
    { 0 ! 8 } &
    { 0 ! 9 }
  }