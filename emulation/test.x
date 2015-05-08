
proc main() is
    var in;
    var jn;
    var kn;
    network
    { { in := 29; 6 ! in } &
      { jn := 192; 7 ! jn } &
      { kn := 23 } & 
      { kn := 23 } &
      { kn := 23 } &
      { kn := 23 } &
      { 0 ? kn } &
      { 1 ? kn }
    }