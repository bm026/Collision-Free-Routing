


func addone(val x) is return x+1

func addtwo(val x) is return x+2

proc main() is
  var out;
  var in;
  network
  { { 1 ! out; out := addone(in); 0 ? in; 1 ! out; out := addtwo(in); 0 ? in }&
    { 2 ! out; out := addone(in); 0 ? in }&
    { 3 ! out; out := addone(in); 0 ? in }&
    { 4 ! out; out := addone(in); 0 ? in }&  
    { 5 ! out; out := addone(in); 0 ? in }&
    { 6 ! out; out := addone(in); 0 ? in }&
    { 7 ! out; out := addone(in); 0 ? in }&
    { 8 ! out; out := addone(in); 0 ? in }
  };



