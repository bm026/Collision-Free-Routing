

proc main() is
  network
  { addonea(1) &
    addoneb(2) &
    addonec(3) &
    addoned(4) &
    addonee(5) &
    addonef(6) &
    addoneg(7) &
    addoneh(0)
  }

proc addonea(val out) is 
  var a;
  var in;
  while a<400 do
  { a:=a+1; out ! a; 0 ? in }

proc addoneb(val out1) is 
  var b;
  var in;
  while b<400 do
  { b:=b+1; out1 ! b; 0 ? in }

proc addonec(val out2) is 
  var c;
  var in;
  while c<400 do
  { c:=c+1; out2 ! c; 0 ? in }

proc addoned(val out3) is 
  var d;
  var in;
  while d<400 do
  { d:=d+1; out3 ! d; 0 ? in }

proc addonee(val out4) is 
  var e;
  var in;
  while e<400 do
  { e:=e+1; out4 ! e; 0 ? in }

proc addonef(val out5) is 
  var f;
  var in;
  while f<400 do
  { f:=f+1; out5 ! f; 0 ? in }

proc addoneg(val out6) is 
  var g;
  var in;
  while g<400 do
  { g:=g+1; out6 ! g; 0 ? in }

proc addoneh(val out7) is 
  var h;
  var in;
  while h<400 do
  { h:=h+1; out7 ! h; 0 ? in }