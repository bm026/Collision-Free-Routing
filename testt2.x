
proc main() is
    var in;
    var out;
    network 
    { { out := 50; 1 ! out; 0 ? in } &
      { 0 ? in; out := in+1; 2 ! out } &
      { 0 ? in; out := in+1; 3 ! out } &
      { 0 ? in; out := in+1; 4 ! out } &
      { 0 ? in; out := in+1; 5 ! out } &
      { 0 ? in; out := in+1; 6 ! out } &
      { 0 ? in; out := in+1; 7 ! out } &
      { 0 ? in; out := in+1; 0 ! out } 
    }
