
array north_halo[16];
array south_halo[16];
array east_halo[16];
array west_halo[16];
array pixels[9];

proc main() is
    var in;
    var out;
    network 
    { { send_halos(12,4,1,3); blur_local_block(); 
        receive_halos(12,4,1,3); blur_halo_blocks() } &
      { send_halos(13,5,2,0); blur_local_block(); 
        receive_halos(13,5,2,0); blur_halo_blocks() } &
      { send_halos(14,6,3,1); blur_local_block(); 
        receive_halos(14,6,3,1); blur_halo_blocks() } &
      { send_halos(15,7,0,2); blur_local_block(); 
        receive_halos(15,7,0,2); blur_halo_blocks() } &
      { send_halos(0,8,5,7); blur_local_block(); 
        receive_halos(0,8,5,7); blur_halo_blocks() } &
      { send_halos(1,9,6,4); blur_local_block(); 
        receive_halos(1,9,6,4); blur_halo_blocks() } &
      { send_halos(2,10,7,5); blur_local_block(); 
        receive_halos(2,10,7,5); blur_halo_blocks() } &
      { send_halos(3,11,4,6); blur_local_block(); 
        receive_halos(3,11,4,6); blur_halo_blocks() } &
      { send_halos(4,12,9,11); blur_local_block(); 
        receive_halos(4,12,9,11); blur_halo_blocks() } &
      { send_halos(5,13,10,8); blur_local_block(); 
        receive_halos(5,13,10,8); blur_halo_blocks() } &
      { send_halos(6,14,11,9); blur_local_block(); 
        receive_halos(6,14,11,9); blur_halo_blocks() } &
      { send_halos(7,15,8,10); blur_local_block(); 
        receive_halos(7,15,8,10); blur_halo_blocks() } &
      { send_halos(8,0,13,15); blur_local_block(); 
        receive_halos(8,0,13,15); blur_halo_blocks() } &
      { send_halos(9,1,14,12); blur_local_block(); 
        receive_halos(9,1,14,12); blur_halo_blocks() } &
      { send_halos(10,2,15,13); blur_local_block(); 
        receive_halos(10,2,15,13); blur_halo_blocks() } &
      { send_halos(11,3,12,14); blur_local_block(); 
        receive_halos(11,3,12,14); blur_halo_blocks() } 
    }

proc send_halos(val north, val south, val east, val west) is
    var i;
    while i<16 do 
    { north ! north_halo[i]; east ! east_halo[i];
      south ! south_halo[i]; west ! west_halo[i];
      i := i+1 }

proc blur_local_block() is
    var i;
    while i<196 do
    { blur_pixel(); i := i+1 }

proc blur_pixel() is
    var i;
    var pixel;
    while i<9 do
    { pixel:= pixel + pixels[i]; i := i+1 }

proc receive_halos(val north, val south, val east, val west) is
    var i;
    while i<16 do
    { south ? north_halo[i]; west ? east_halo[i];
      north ? south_halo[i]; east ? west_halo[i];
      i := i+1 }

proc blur_halo_blocks() is
    var i;
    while i<16 do
    { blur_halo_pixels(i); i := i+1 }

proc blur_halo_pixels(val num) is
    var i;
    while i<9 do
    { north_halo[num] := north_halo[num] + pixels[i];
      east_halo[num] := east_halo[num] + pixels[i];
      south_halo[num] := south_halo[num] + pixels[i];
      west_halo[num] := west_halo[num] + pixels[i];
      i := i+1 }

