
array pixels[4096];
array local_pixels[9];

proc main() is
    var i;
    while i<4096 do
    { blur_pixel(i); i := i+1 }

proc blur_pixel(val num) is
    var i;
    while i<9 do
    { pixels[num] := pixels[num] + local_pixels[i];
      i := i+1 }