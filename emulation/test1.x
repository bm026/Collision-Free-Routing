
array in[8];
array out[8];

proc main() is
    network
    { { out[0] := 1; 1 ! out[0]; 7 ? in[0] } &
      { 0 ? in[1]; out[1] := in[1] + 1; 2 ! out[1] } &
      { 1 ? in[2]; out[2] := in[2] + 1; 3 ! out[2] } &
      { 2 ? in[3]; out[3] := in[3] + 1; 4 ! out[3] } &
      { 3 ? in[4]; out[4] := in[4] + 1; 5 ! out[4] } &
      { 4 ? in[5]; out[5] := in[5] + 1; 6 ! out[5] } &
      { 5 ? in[6]; out[6] := in[6] + 1; 7 ! out[6] } &
      { 6 ? in[7]; out[7] := in[7] + 1; 0 ! out[7] }
    }