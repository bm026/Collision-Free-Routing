
array in[16];
array out[16];

proc main() is
    network
    { { out[0] := 50; 8 ! out[0]; 0 ? in[0] } &
      { out[1] := 51; 9 ! out[1]; 0 ? in[1] } &
      { out[2] := 52; 10 ! out[2]; 0 ? in[2] } &
      { out[3] := 53; 11 ! out[3]; 0 ? in[3] } &
      { out[4] := 54; 12 ! out[4]; 0 ? in[4] } &
      { out[5] := 55; 13 ! out[5]; 0 ? in[5] } &
      { out[6] := 56; 14 ! out[6]; 0 ? in[6] } &
      { out[7] := 57; 15 ! out[7]; 0 ? in[7] } &
      { out[8] := 58; 0 ! out[8]; 0 ? in[8] } &
      { out[9] := 59; 1 ! out[9]; 0 ? in[9] } &
      { out[10] := 60; 2 ! out[10]; 0 ? in[10] } &
      { out[11] := 61; 3 ! out[11]; 0 ? in[11] } &
      { out[12] := 62; 4 ! out[12]; 0 ? in[12] } &
      { out[13] := 63; 5 ! out[13]; 0 ? in[13] } &
      { out[14] := 64; 6 ! out[14]; 0 ? in[14] } &
      { out[15] := 65; 7 ! out[15]; 0 ? in[15] } &
    }