
proc main() is
  var in;
  network
  { { in := 23; 2 ! in; 0 ? in }&
    { 4 ! 44; 0 ? in }&
    { 6 ! 26; 0 ? in }&
    { 1 ! 31; 0 ? in }&  
    { 7 ! 47; 0 ? in }&
    { 0 ! 50; 0 ? in }&
    { 5 ! 65; 0 ? in }&
    { 3 ! 73; 0 ? in }
  };
