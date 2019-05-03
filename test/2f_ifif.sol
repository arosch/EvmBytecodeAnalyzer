pragma solidity ^0.5;

contract Addition {

    int left =3;

    int right = 5;
    int count = 20;

    function add(int a, int b, int c) public pure returns(int result){
      if(a<0){
        result = a;
      } else if(a==1){
        int i=0;
        if(b>=4){
          i=c;
        }
        result = i+a;
      } else {
        result = a+b;
      }
    }

    function sub() public{
    right =6;
    }
}
