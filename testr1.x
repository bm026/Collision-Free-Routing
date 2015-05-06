proc main() is
    network
    { { addn(25, 25) }  &  { addn(7, 7)   } &
      { addn(19, 19) }  &  { addn(16, 16) } &
      { addn(8, 8)   }  &  { addn(28, 28) } &
      { addn(21, 21) }  &  { addn(1, 1)   } &
      { addn(4, 4)   }  &  { addn(15, 15) } &
      { addn(29, 29) }  &  { addn(20, 20) } &
      { addn(14, 14) }  &  { addn(30, 30) } &
      { addn(12, 12) }  &  { addn(9, 9)   } &
      { addn(3, 3)   }  &  { addn(27, 27) } &
      { addn(26, 26) }  &  { addn(2, 2)   } &
      { addn(11, 11) }  &  { addn(6, 6)   } &
      { addn(31, 31) }  &  { addn(24, 24) } &
      { addn(23, 23) }  &  { addn(0, 0)   } &
      { addn(18, 18) }  &  { addn(17, 17) } &
      { addn(5, 5)   }  &  { addn(10, 10) } &
      { addn(13, 13) }  &  { addn(22, 22) }
    }

 proc addn(val send, val recv) is 
    var count;
    var in;
    while count<1000 do
    { count:=in+1; send ! count; recv ? in }