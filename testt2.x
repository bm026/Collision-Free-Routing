
proc main() is
    var in;
    var out;
    network 
    { { out := 50; 1 ! out; 7 ? in } &
      { 0 ? in; out := in+1; 2 ! out } &
      { 1 ? in; out := in+1; 3 ! out } &
      { 2 ? in; out := in+1; 4 ! out } &
      { 3 ? in; out := in+1; 5 ! out } &
      { 4 ? in; out := in+1; 6 ! out } &
      { 5 ? in; out := in+1; 7 ! out } &
      { 6 ? in; out := in+1; 0 ! out } 
    }
